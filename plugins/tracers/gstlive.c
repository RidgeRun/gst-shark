/* GstShark - A Front End for GstTracer
 * Copyright (C) 2018 RidgeRun Engineering <michael.gruner@ridgerun.com>
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

/**
 * SECTION:gstlive
 * @short_description: display live profiler
 *
 * A tracing module that uses the DOT libraries in order to show the pipeline executed liveally
 */

#include "gstlive.h"
#include "gstdot.h"
#include "gstcpuusagecompute.h"
#include "gstperiodictracer.h"
#include "gstliveprofiler.h"

#include <stdio.h>

GST_DEBUG_CATEGORY_STATIC (gst_live_debug);
#define GST_CAT_DEFAULT gst_live_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_STATES);

struct _GstLiveTracer
{
  GstTracer parent;
  GstCPUUsage cpu_usage;
  gboolean event_running;
};

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_live_debug, "live", 0, "live tracer"); \
    GST_DEBUG_CATEGORY_GET (GST_CAT_STATES, "GST_STATES");

G_DEFINE_TYPE_WITH_CODE (GstLiveTracer, gst_live_tracer,
    GST_SHARK_TYPE_TRACER, _do_init);

#define PERIODIC_INTERVAL 1

static gboolean
do_periodic (GObject * obj)
{
  GstLiveTracer *self = GST_LIVE_TRACER (obj);
  GstCPUUsage *cpu_usage;
  gfloat *cpu_load;
  gint cpu_load_len;
  static guint counter = 0;

  cpu_usage = &self->cpu_usage;

  cpu_load = CPU_USAGE_ARRAY (cpu_usage);
  cpu_load_len = CPU_USAGE_ARRAY_LENGTH (cpu_usage);

  //cpu_usage->cpu_array_sel = FALSE;
  gst_cpu_usage_compute (cpu_usage);

  update_cpuusage_event (cpu_load_len, cpu_load);

  return TRUE;
}

static void
do_element_change_state_post (GObject * self, guint64 ts,
    GstElement * element, GstStateChange transition,
    GstStateChangeReturn result)
{
  GstLiveTracer *tracer = GST_LIVE_TRACER (self);

  if (GST_IS_PIPELINE (element)
      && (transition == GST_STATE_CHANGE_PAUSED_TO_PLAYING)) {
    update_pipeline_init ((GstPipeline *) element);
    if (!tracer->event_running) {
      gst_cpu_usage_init (&(tracer->cpu_usage));
      g_timeout_add_seconds (PERIODIC_INTERVAL,
          (GSourceFunc) do_periodic, (gpointer) tracer);
      tracer->event_running = TRUE;
    }
  }
}

static void
do_pad_push_pre (GstTracer * self, guint64 ts, GstPad * pad, GstBuffer * buffer)
{
  gchar *element_name = GST_OBJECT_NAME (GST_OBJECT_PARENT (pad));
  gchar *pad_name = GST_OBJECT_NAME (pad);
  element_push_buffer_pre (element_name, pad_name, ts,
      gst_buffer_get_size (buffer));
}

static void
do_pad_push_post (GstTracer * self, guint64 ts, GstPad * pad)
{
  gchar *element_name = GST_OBJECT_NAME (GST_OBJECT_PARENT (pad));
  gchar *pad_name = GST_OBJECT_NAME (pad);
  element_push_buffer_post (element_name, pad_name, ts);
}

static void
do_pad_push_list_pre (GstTracer * self, guint64 ts, GstPad * pad)
{
  gchar *element_name = GST_OBJECT_NAME (GST_OBJECT_PARENT (pad));
  gchar *pad_name = GST_OBJECT_NAME (pad);
  element_push_buffer_list_pre (element_name, pad_name, ts);
}

static void
do_pad_push_list_post (GstTracer * self, guint64 ts, GstPad * pad)
{
  gchar *element_name = GST_OBJECT_NAME (GST_OBJECT_PARENT (pad));
  gchar *pad_name = GST_OBJECT_NAME (pad);
  element_push_buffer_list_post (element_name, pad_name, ts);
}

static void
do_pad_pull_range_pre (GstTracer * self, guint64 ts, GstPad * pad)
{
  gchar *element_name = GST_OBJECT_NAME (GST_OBJECT_PARENT (pad));
  gchar *pad_name = GST_OBJECT_NAME (pad);
  element_pull_range_pre (element_name, pad_name, ts);
}

static void
do_pad_pull_range_post (GstTracer * self, guint64 ts, GstPad * pad)
{
  gchar *element_name = GST_OBJECT_NAME (GST_OBJECT_PARENT (pad));
  gchar *pad_name = GST_OBJECT_NAME (pad);
  element_pull_range_post (element_name, pad_name, ts);
}

static void gst_live_tracer_finalize (GObject * obj);

/* tracer class */

static void
gst_live_tracer_finalize (GObject * obj)
{
  g_unsetenv ("LIVEPROFILER_ENABLED");
  gst_liveprofiler_finalize ();
  G_OBJECT_CLASS (gst_live_tracer_parent_class)->finalize (obj);
}

static void
gst_live_tracer_class_init (GstLiveTracerClass * klass)
{
  GObjectClass *g_obj_class = G_OBJECT_CLASS (klass);

  gst_liveprofiler_init ();
  g_setenv ("LIVEPROFILER_ENABLED", "TRUE", TRUE);
  g_obj_class->finalize = gst_live_tracer_finalize;
}

static void
gst_live_tracer_init (GstLiveTracer * self)
{
  GstTracer *tracer = GST_TRACER (self);
  GstCPUUsage *cpu_usage;

  cpu_usage = &self->cpu_usage;
  gst_cpu_usage_init (cpu_usage);
  cpu_usage->cpu_array_sel = FALSE;
  self->event_running = FALSE;

  gst_tracing_register_hook (tracer, "element-change-state-post",
      G_CALLBACK (do_element_change_state_post));
  gst_tracing_register_hook (tracer, "pad-push-pre",
      G_CALLBACK (do_pad_push_pre));
  gst_tracing_register_hook (tracer, "pad-push-post",
      G_CALLBACK (do_pad_push_post));
  gst_tracing_register_hook (tracer, "pad-push-list-pre",
      G_CALLBACK (do_pad_push_list_pre));
  gst_tracing_register_hook (tracer, "pad-push-list-post",
      G_CALLBACK (do_pad_push_list_post));
  gst_tracing_register_hook (tracer, "pad-pull-range-pre",
      G_CALLBACK (do_pad_pull_range_pre));
  gst_tracing_register_hook (tracer, "pad-pull-range-post",
      G_CALLBACK (do_pad_pull_range_post));
}
