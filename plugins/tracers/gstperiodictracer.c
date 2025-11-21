/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016-2018 RidgeRun Engineering <sebastian.fatjo@ridgerun.com>
 *
 * This file is part of GstShark.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "gstperiodictracer.h"

GST_DEBUG_CATEGORY_STATIC (gst_periodic_debug);
#define GST_CAT_DEFAULT gst_periodic_debug

static void element_change_state_post (GstTracer * self, guint64 ts,
    GstElement * element, GstStateChange transition,
    GstStateChangeReturn result);
static void install_callback (GstPeriodicTracer * self);
static void remove_callback (GstPeriodicTracer * self);
static void reset_internal (GstPeriodicTracer * self);
static gboolean callback_internal (gpointer data);
static void write_header_internal (GstPeriodicTracer * self);
static gint set_period (GstPeriodicTracer * self);
static void start_monitoring_internal (GstPeriodicTracer * self,
    GstPipeline * pipeline);
static void stop_monitoring_internal (GstPeriodicTracer * self,
    GstPipeline * pipeline);
static gpointer periodic_thread (gpointer data);
static void start_timer_thread (GstPeriodicTracer * self);
static void stop_timer_thread (GstPeriodicTracer * self);
static void gst_periodic_tracer_finalize (GObject * object);

#define GST_PERIODIC_TRACER_PRIVATE(o) \
  gst_periodic_tracer_get_instance_private(GST_PERIODIC_TRACER(o))

#define DEFAULT_TIMEOUT_INTERVAL 1

typedef struct _GstPeriodicTracerPrivate GstPeriodicTracerPrivate;
struct _GstPeriodicTracerPrivate
{
  guint pipes_running;
  gboolean header_written;
  gint period;
  GThread *thread;
  GMutex thread_lock;
  GCond thread_cond;
  gboolean stop_thread;
};

G_DEFINE_TYPE_WITH_PRIVATE (GstPeriodicTracer, gst_periodic_tracer,
    GST_SHARK_TYPE_TRACER);

static void
gst_periodic_tracer_class_init (GstPeriodicTracerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  GST_DEBUG_CATEGORY_INIT (gst_periodic_debug, "periodictracer", 0,
      "base periodic tracer");

  klass->timer_callback = NULL;
  klass->reset = NULL;
  klass->write_header = NULL;

  gobject_class->finalize = gst_periodic_tracer_finalize;
}

static void
gst_periodic_tracer_init (GstPeriodicTracer *self)
{
  GstPeriodicTracerPrivate *priv;
  GstTracer *tracer;

  priv = GST_PERIODIC_TRACER_PRIVATE (self);
  tracer = GST_TRACER (self);

  priv->pipes_running = 0;
  priv->header_written = FALSE;
  priv->period = DEFAULT_TIMEOUT_INTERVAL;
  priv->thread = NULL;
  priv->stop_thread = FALSE;
  g_mutex_init (&priv->thread_lock);
  g_cond_init (&priv->thread_cond);

  gst_tracing_register_hook (tracer, "element-change-state-post",
      G_CALLBACK (element_change_state_post));
}

static void
element_change_state_post (GstTracer *tracer, guint64 ts,
    GstElement *element, GstStateChange transition, GstStateChangeReturn result)
{
  GstPeriodicTracer *self = GST_PERIODIC_TRACER (tracer);

  /* We are only interested in capturing when a pipeline goes to
     playing, but this hook reports for every element in the
     pipeline
   */
  if (FALSE == GST_IS_PIPELINE (element)) {
    return;
  }

  if (transition == GST_STATE_CHANGE_PAUSED_TO_PLAYING
      && result == GST_STATE_CHANGE_SUCCESS) {
    GST_DEBUG_OBJECT (self, "Pipeline %s changed to playing",
        GST_OBJECT_NAME (element));
    write_header_internal (self);
    reset_internal (self);
    start_monitoring_internal (self, GST_PIPELINE (element));
    install_callback (self);
  } else if (transition == GST_STATE_CHANGE_PLAYING_TO_PAUSED) {
    GST_DEBUG_OBJECT (self, "Pipeline %s changed to paused",
        GST_OBJECT_NAME (element));
    stop_monitoring_internal (self, GST_PIPELINE (element));
    remove_callback (self);
  }
}

static void
write_header_internal (GstPeriodicTracer *self)
{
  GstPeriodicTracerClass *klass;
  GstPeriodicTracerPrivate *priv;
  gboolean header_written;

  g_return_if_fail (self);

  priv = GST_PERIODIC_TRACER_PRIVATE (self);

  GST_OBJECT_LOCK (self);
  header_written = priv->header_written;
  if (FALSE == priv->header_written) {
    priv->header_written = TRUE;
  }
  GST_OBJECT_UNLOCK (self);

  /* Header must only be written once in all the lifetime of the tracer */
  if (TRUE == header_written) {
    return;
  }

  GST_DEBUG_OBJECT (self, "Writing event header");

  klass = GST_PERIODIC_TRACER_GET_CLASS (self);

  if (klass->write_header) {
    klass->write_header (self);
  }
}

static void
install_callback (GstPeriodicTracer *self)
{
  GstPeriodicTracerPrivate *priv;

  g_return_if_fail (self);

  priv = GST_PERIODIC_TRACER_PRIVATE (self);

  GST_OBJECT_LOCK (self);

  if (0 == priv->pipes_running) {
    GST_INFO_OBJECT (self,
        "First pipeline started running, starting profiling");

    priv->period = set_period (self);
    start_timer_thread (self);
  }

  priv->pipes_running++;
  GST_DEBUG_OBJECT (self, "Pipes running: %d", priv->pipes_running);

  GST_OBJECT_UNLOCK (self);
}

static void
remove_callback (GstPeriodicTracer *self)
{
  GstPeriodicTracerPrivate *priv;
  gboolean stop_thread = FALSE;

  g_return_if_fail (self);

  priv = GST_PERIODIC_TRACER_PRIVATE (self);

  GST_OBJECT_LOCK (self);

  if (1 == priv->pipes_running) {
    GST_INFO_OBJECT (self, "Last pipeline stopped running, stopped profiling");
    stop_thread = TRUE;
  }

  priv->pipes_running--;
  GST_DEBUG_OBJECT (self, "Pipes running: %d", priv->pipes_running);

  GST_OBJECT_UNLOCK (self);

  if (stop_thread) {
    stop_timer_thread (self);
  }
}

static void
reset_internal (GstPeriodicTracer *self)
{
  GstPeriodicTracerClass *klass;

  g_return_if_fail (self);

  klass = GST_PERIODIC_TRACER_GET_CLASS (self);

  /* It is okay if subclass didn't provide a reset implementation */
  if (klass->reset) {
    GST_DEBUG_OBJECT (self, "Resetting ourselves");
    klass->reset (self);
  }
}

static gboolean
callback_internal (gpointer data)
{
  GstPeriodicTracer *self;
  GstPeriodicTracerClass *klass;

  g_return_val_if_fail (data, FALSE);

  self = GST_PERIODIC_TRACER (data);
  klass = GST_PERIODIC_TRACER_GET_CLASS (self);

  /* This is a required method, if no implementation was provided, we
     consider it as a programming error */
  g_return_val_if_fail (klass->timer_callback, FALSE);

  return klass->timer_callback (self);
}

static gpointer
periodic_thread (gpointer data)
{
  GstPeriodicTracer *self = NULL;
  GstPeriodicTracerPrivate *priv = NULL;

  g_return_val_if_fail (data, NULL);

  self = GST_PERIODIC_TRACER (data);
  priv = GST_PERIODIC_TRACER_PRIVATE (self);

  while (TRUE) {
    gboolean timed_out = FALSE;
    gint period;
    gboolean stop;
    gint64 end_time;

    g_mutex_lock (&priv->thread_lock);
    stop = priv->stop_thread;
    period = priv->period;
    end_time = g_get_monotonic_time () + ((gint64) period * G_TIME_SPAN_SECOND);

    while (!priv->stop_thread) {
      gboolean signaled;

      signaled =
          g_cond_wait_until (&priv->thread_cond, &priv->thread_lock, end_time);

      if (!signaled) {
        timed_out = TRUE;
        break;
      }
    }

    stop = priv->stop_thread;

    g_mutex_unlock (&priv->thread_lock);

    if (stop) {
      break;
    }

    if (timed_out) {
      if (!callback_internal (self)) {
        g_mutex_lock (&priv->thread_lock);
        priv->stop_thread = TRUE;
        g_mutex_unlock (&priv->thread_lock);
        break;
      }
    }
  }

  g_object_unref (self);

  return NULL;
}

static void
start_timer_thread (GstPeriodicTracer *self)
{
  GstPeriodicTracerPrivate *priv;

  g_return_if_fail (self);

  priv = GST_PERIODIC_TRACER_PRIVATE (self);

  g_mutex_lock (&priv->thread_lock);

  if (NULL == priv->thread) {
    priv->stop_thread = FALSE;
    priv->thread = g_thread_new ("gst-periodic-tracer",
        (GThreadFunc) periodic_thread, g_object_ref (self));
  }

  g_mutex_unlock (&priv->thread_lock);
}

static void
stop_timer_thread (GstPeriodicTracer *self)
{
  GstPeriodicTracerPrivate *priv;
  GThread *thread = NULL;

  g_return_if_fail (self);

  priv = GST_PERIODIC_TRACER_PRIVATE (self);

  g_mutex_lock (&priv->thread_lock);

  if (priv->thread) {
    priv->stop_thread = TRUE;
    g_cond_broadcast (&priv->thread_cond);
    thread = priv->thread;
  }

  g_mutex_unlock (&priv->thread_lock);

  if (thread) {
    g_thread_join (thread);

    g_mutex_lock (&priv->thread_lock);
    if (priv->thread == thread) {
      priv->thread = NULL;
      priv->stop_thread = FALSE;
    }
    g_mutex_unlock (&priv->thread_lock);
  }
}


static void
start_monitoring_internal (GstPeriodicTracer *self, GstPipeline *pipeline)
{
  GstPeriodicTracerClass *klass;

  g_return_if_fail (self);

  klass = GST_PERIODIC_TRACER_GET_CLASS (self);

  /* It is okay if subclass don't provide start_monitoring implementation */
  if (klass->start_monitoring) {
    GST_DEBUG_OBJECT (self, "Start pipeline monitoring");
    klass->start_monitoring (self, pipeline);
  }
}

static void
stop_monitoring_internal (GstPeriodicTracer *self, GstPipeline *pipeline)
{
  GstPeriodicTracerClass *klass;

  g_return_if_fail (self);

  klass = GST_PERIODIC_TRACER_GET_CLASS (self);

  /* It is okay if subclass don't provide stop_monitoring implementation */
  if (klass->stop_monitoring) {
    GST_DEBUG_OBJECT (self, "Stop pipeline monitoring");
    klass->stop_monitoring (self, pipeline);
  }
}

static gint
set_period (GstPeriodicTracer *self)
{
  GList *list = NULL;
  GstSharkTracer *tracer = NULL;
  static const gchar *name = "period";
  gint period = -1;

  g_return_val_if_fail (self, DEFAULT_TIMEOUT_INTERVAL);

  tracer = GST_SHARK_TRACER (self);

  list = gst_shark_tracer_get_param (tracer, name);

  if (NULL == list) {
    GST_INFO_OBJECT (self, "No period specifying, using default of %d",
        DEFAULT_TIMEOUT_INTERVAL);
    period = DEFAULT_TIMEOUT_INTERVAL;
  } else {
    GST_INFO_OBJECT (self, "Attempting to parse provided period \"%s\"",
        (gchar *) list->data);
    period = g_ascii_strtoull (list->data, NULL, 0);
    /* On error, 0 is set */
    if (0 == period) {
      period = DEFAULT_TIMEOUT_INTERVAL;
    }
  }

  return period;
}

static void
gst_periodic_tracer_finalize (GObject *object)
{
  GstPeriodicTracer *self = GST_PERIODIC_TRACER (object);
  GstPeriodicTracerPrivate *priv;

  priv = GST_PERIODIC_TRACER_PRIVATE (self);

  stop_timer_thread (self);

  g_mutex_clear (&priv->thread_lock);
  g_cond_clear (&priv->thread_cond);

  G_OBJECT_CLASS (gst_periodic_tracer_parent_class)->finalize (object);
}
