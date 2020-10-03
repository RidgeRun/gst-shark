/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016-2020 RidgeRun Engineering <support@ridgerun.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstctfcomponent.h"

#include <babeltrace2/babeltrace.h>

#include "gstctfrecordpriv.h"

GST_DEBUG_CATEGORY_EXTERN (gst_ctf_debug);
#define GST_CAT_DEFAULT gst_ctf_debug

typedef enum _GstCtfComponentState GstCtfComponentState;
enum _GstCtfComponentState
{
  GST_CTF_COMPONENT_STATE_INIT,
  GST_CTF_COMPONENT_STATE_RUNNING,
  GST_CTF_COMPONENT_STATE_ENDED,
};

struct _GstCtfComponent
{
  GstObject base;

  bt_stream *stream;
  bt_stream_class *stream_class;
  bt_self_message_iterator *iterator;
  bt_component *component;

  GAsyncQueue *queue;
  GstCtfComponentState state;
};

G_DEFINE_TYPE (GstCtfComponent, gst_ctf_component, GST_TYPE_OBJECT);

/* GObject */
static void gst_ctf_component_finalize (GObject * self);

/* Babeltrece */
static bt_message_iterator_class_next_method_status
ctf_component_iterator_next (bt_self_message_iterator * self_message_iterator,
    bt_message_array_const messages, guint64 capacity, guint64 * count);
static bt_component_class_initialize_method_status
ctf_component_initialize (bt_self_component_source * self_component_source,
    bt_self_component_source_configuration * configuration,
    const bt_value * params, void *initialize_method_data);
static void ctf_component_finalize (bt_self_component_source *
    self_component_source);
static bt_message_iterator_class_initialize_method_status
ctf_component_iterator_initialize (bt_self_message_iterator *
    self_message_iterator,
    bt_self_message_iterator_configuration * configuration,
    bt_self_component_port_output * self_port);
static void ctf_component_iterator_finalize (bt_self_message_iterator *
    self_message_iterator);

/* Self */
static bt_component_class_initialize_method_status
gst_ctf_component_create_metadata_and_stream (GstCtfComponent * self,
    bt_self_component * component);

static void
gst_ctf_component_init (GstCtfComponent * self)
{
  self->stream = NULL;
  self->iterator = NULL;
  self->component = NULL;
  self->queue = g_async_queue_new_full ((GDestroyNotify) bt_message_put_ref);
  self->state = GST_CTF_COMPONENT_STATE_INIT;
}

static void
gst_ctf_component_class_init (GstCtfComponentClass * klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->finalize = gst_ctf_component_finalize;
}

static void
gst_ctf_component_finalize (GObject * object)
{
  GstCtfComponent *self = GST_CTF_COMPONENT (object);

  GST_INFO_OBJECT (self, "Freeing component");

  bt_stream_put_ref (self->stream);
  self->stream = NULL;

  /* Iterator's ref is handled in the component */
  bt_component_put_ref (self->component);
  self->component = NULL;
  self->iterator = NULL;

  if (self->queue) {
    g_async_queue_unref (self->queue);
  }
  self->queue = NULL;

  return G_OBJECT_CLASS (gst_ctf_component_parent_class)->finalize (object);
}

GstCtfRecord *
gst_ctf_component_register_event_valist (GstCtfComponent * self,
    const gchar * name, const gchar * firstfield, va_list var_args)
{
  bt_stream *stream = NULL;
  bt_self_message_iterator *iterator = NULL;
  GAsyncQueue *queue = NULL;

  g_return_val_if_fail (self, NULL);
  g_return_val_if_fail (name, NULL);
  g_return_val_if_fail (firstfield, NULL);

  GST_OBJECT_LOCK (self);
  stream = self->stream;
  iterator = self->iterator;
  queue = self->queue;
  GST_OBJECT_UNLOCK (self);

  return gst_ctf_record_new_valist (stream, iterator, queue, name, firstfield,
      var_args);
}

void
gst_ctf_component_set_queue (GstCtfComponent * self, GAsyncQueue * queue)
{
  g_return_if_fail (self);
  g_return_if_fail (queue);

  GST_OBJECT_LOCK (self);
  if (NULL != self->queue) {
    g_async_queue_unref (self->queue);
  }

  self->queue = g_async_queue_ref (queue);
  GST_OBJECT_UNLOCK (self);
}

static bt_component_class_initialize_method_status
gst_ctf_component_create_metadata_and_stream (GstCtfComponent * self,
    bt_self_component * component)
{
  bt_trace_class *trace_class = NULL;
  bt_stream_class *stream_class = NULL;
  bt_clock_class *clock_class = NULL;
  bt_trace *trace = NULL;
  bt_stream *stream = NULL;
  const gchar *trace_name = "gst";
  const gchar *stream_name = "tracers";
  gint ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_OK;

  g_return_val_if_fail (self,
      BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_ERROR);
  g_return_val_if_fail (component,
      BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_ERROR);

  trace_class = bt_trace_class_create (component);
  if (NULL == trace_class) {
    GST_ERROR_OBJECT (self, "Unable to create trace class");
    ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_MEMORY_ERROR;
    goto out;
  }

  stream_class = bt_stream_class_create (trace_class);
  if (NULL == stream_class) {
    GST_ERROR_OBJECT (self, "Unable to create stream class");
    ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_MEMORY_ERROR;
    goto free_trace_class;
  }

  clock_class = bt_clock_class_create (component);
  if (NULL == stream_class) {
    GST_ERROR_OBJECT (self, "Unable to create clock class");
    ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_MEMORY_ERROR;
    goto free_stream_class;
  }

  /*
   * Set `clock_class` as the default clock class of `stream_class`.
   *
   * This means all the streams created from `stream_class` have a
   * conceptual default clock which is an instance of `clock_class`.
   * Any event message created for such a stream has a snapshot of the
   * stream's default clock.
   */
  ret = bt_stream_class_set_default_clock_class (stream_class, clock_class);
  if (BT_STREAM_CLASS_SET_DEFAULT_CLOCK_CLASS_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self, "Unable to assign clock class to stream");
    ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_ERROR;
    goto free_clock_class;
  }

  /* Create a default trace from (instance of `trace_class`) */
  trace = bt_trace_create (trace_class);
  if (NULL == trace) {
    GST_ERROR_OBJECT (self, "Unable to create trace");
    ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_ERROR;
    goto free_clock_class;
  }

  ret = bt_trace_set_name (trace, trace_name);
  if (BT_TRACE_SET_NAME_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self, "Unable to set trace name");
    ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_ERROR;
    goto free_clock_class;
  }

  stream = bt_stream_create (stream_class, trace);
  if (NULL == trace) {
    GST_ERROR_OBJECT (self, "Unable to create stream");
    ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_ERROR;
    goto free_trace;
  }

  ret = bt_stream_set_name (stream, stream_name);
  if (BT_TRACE_SET_NAME_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self, "Unable to set stream name");
    ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_ERROR;
    goto free_trace;
  }

  GST_OBJECT_LOCK (self);
  self->stream = stream;
  GST_OBJECT_UNLOCK (self);

  ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_OK;

free_trace:
  bt_trace_put_ref (trace);

free_clock_class:
  bt_clock_class_put_ref (clock_class);

free_stream_class:
  bt_stream_class_put_ref (stream_class);

free_trace_class:
  bt_trace_class_put_ref (trace_class);

out:
  return ret;
}

static bt_component_class_initialize_method_status
ctf_component_initialize (bt_self_component_source * component_source,
    bt_self_component_source_configuration * configuration,
    const bt_value * params, void *user_data)
{
  GstCtfComponent *self = NULL;
  bt_self_component *component = NULL;
  const gchar *name = "out";
  GstCtfComponent **placeholder = (GstCtfComponent **) user_data;
  gint ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_OK;

  component = bt_self_component_source_as_self_component (component_source);
  g_return_val_if_fail (component,
      BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_ERROR);

  self = g_object_new (GST_TYPE_CTF_COMPONENT, NULL);

  /* Create the source component's metadata and stream objects */
  ret = gst_ctf_component_create_metadata_and_stream (self, component);
  if (BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self, "Failed initializing component");
    goto out;
  }

  /* Add an output port so that it can be connected */
  ret =
      bt_self_component_source_add_output_port (component_source, name, NULL,
      NULL);
  if (BT_SELF_COMPONENT_ADD_PORT_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self, "Unable to add %s port to the component", name);
    goto out;
  }

  /* Set the component's user data to our private data structure */
  bt_self_component_set_data (component, gst_object_ref (self));

  if (placeholder) {
    *placeholder = gst_object_ref (self);
  }

  GST_INFO_OBJECT (self, "Successfully initialized CTF component");

out:
  gst_object_unref (self);

  return ret;
}

/*
 * Finalizes the source component.
 */
static void
ctf_component_finalize (bt_self_component_source * self_component_source)
{
  GstCtfComponent *self = NULL;
  bt_self_component *component = NULL;

  component =
      bt_self_component_source_as_self_component (self_component_source);
  g_return_if_fail (component);

  self = GST_CTF_COMPONENT (bt_self_component_get_data (component));

  GST_INFO_OBJECT (self, "Freeing BT component");

  gst_object_unref (self);
}

static bt_message_iterator_class_next_method_status
ctf_component_iterator_next (bt_self_message_iterator * self_message_iterator,
    bt_message_array_const messages, guint64 capacity, guint64 * count)
{
  GstCtfComponent *self = NULL;
  GstCtfComponentState state = GST_CTF_COMPONENT_STATE_INIT;
  GAsyncQueue *queue = NULL;
  bt_self_component *self_component = NULL;
  bt_stream *stream = NULL;
  gint status = BT_MESSAGE_ITERATOR_CLASS_NEXT_METHOD_STATUS_OK;
  gboolean processing = TRUE;
  static const guint64 timeout = 100 * GST_MSECOND / GST_USECOND;

  self_component =
      bt_self_message_iterator_borrow_component (self_message_iterator);
  g_return_val_if_fail (self_component,
      BT_MESSAGE_ITERATOR_CLASS_NEXT_METHOD_STATUS_ERROR);

  self = GST_CTF_COMPONENT (bt_self_component_get_data (self_component));

  GST_LOG_OBJECT (self, "Requested %" G_GUINT64_FORMAT " new messages",
      capacity);

  *count = 0;

  GST_OBJECT_LOCK (self);
  state = self->state;
  queue = self->queue;
  stream = self->stream;
  GST_OBJECT_UNLOCK (self);

  while (processing) {
    switch (state) {
      case GST_CTF_COMPONENT_STATE_INIT:
        GST_DEBUG_OBJECT (self, "Created stream beggining message");
        messages[(*count)++] =
            bt_message_stream_beginning_create (self_message_iterator, stream);
        state = GST_CTF_COMPONENT_STATE_RUNNING;
        break;
      case GST_CTF_COMPONENT_STATE_RUNNING:{
        bt_message *msg = NULL;

        /* Output array is full, return */
        if (*count >= capacity) {
          processing = FALSE;
          break;
        }

        /* No more messages available, don't block anymore */
        msg = g_async_queue_timeout_pop (queue, timeout);
        if (NULL != msg) {
          GST_LOG_OBJECT (self, "Processing msg num %llu %p", *count, msg);
          messages[(*count)++] = msg;

          /* FIXME: Nothing gets written unless the end message is sent */
          /* messages[(*count)++] =
             bt_message_stream_end_create (self_message_iterator, stream);
             state = GST_CTF_COMPONENT_STATE_ENDED; */
        } else {
          GST_LOG_OBJECT (self, "No new mesages");
          /* FIXME: If we have no messages in the array, we should
             return BT_MESSAGE_ITERATOR_CLASS_NEXT_METHOD_STATUS_AGAIN
             to ask BT to call the callback again. However, for some
             reason we never get called again */
          processing = FALSE;
        }
        break;
      }
      case GST_CTF_COMPONENT_STATE_ENDED:
        GST_DEBUG_OBJECT (self, "Finished CTF");
        processing = FALSE;
        status = BT_MESSAGE_ITERATOR_CLASS_NEXT_METHOD_STATUS_END;
        break;
    }
  }

  GST_LOG_OBJECT (self, "Sent %" G_GUINT64_FORMAT " messages", *count);

  GST_OBJECT_LOCK (self);
  self->state = state;
  GST_OBJECT_UNLOCK (self);

  return status;
}

static bt_message_iterator_class_initialize_method_status
ctf_component_iterator_initialize (bt_self_message_iterator *
    self_message_iterator,
    bt_self_message_iterator_configuration * configuration,
    bt_self_component_port_output * self_port)
{
  GstCtfComponent *self = NULL;
  bt_self_component *self_component = NULL;
  gint ret = BT_MESSAGE_ITERATOR_CLASS_INITIALIZE_METHOD_STATUS_OK;

  self_component =
      bt_self_message_iterator_borrow_component (self_message_iterator);
  g_return_val_if_fail (self_component,
      BT_MESSAGE_ITERATOR_CLASS_INITIALIZE_METHOD_STATUS_ERROR);

  self = GST_CTF_COMPONENT (bt_self_component_get_data (self_component));

  /* Do not take a reference to the iterator to avoid a dependency
     cycle. We already have the component which is the same reference
     as the iterator */
  GST_OBJECT_LOCK (self);
  self->iterator = self_message_iterator;
  GST_OBJECT_UNLOCK (self);

  /* Set the message iterator's user data to our private data structure */
  bt_self_message_iterator_set_data (self_message_iterator,
      gst_object_ref (self));

  GST_DEBUG_OBJECT (self, "Initialized CTF message iterator");

  return ret;
}

static void
ctf_component_iterator_finalize (bt_self_message_iterator *
    self_message_iterator)
{
  GstCtfComponent *self =
      GST_CTF_COMPONENT (bt_self_message_iterator_get_data
      (self_message_iterator));

  GST_INFO_OBJECT (self, "Freeing BT iterator");

  gst_object_unref (self);
}

/* Define plugin and source component */
BT_PLUGIN_MODULE ();
BT_PLUGIN (gst);
BT_PLUGIN_SOURCE_COMPONENT_CLASS (tracers, ctf_component_iterator_next);
BT_PLUGIN_SOURCE_COMPONENT_CLASS_INITIALIZE_METHOD (tracers,
    ctf_component_initialize);
BT_PLUGIN_SOURCE_COMPONENT_CLASS_FINALIZE_METHOD (tracers,
    ctf_component_finalize);
BT_PLUGIN_SOURCE_COMPONENT_CLASS_MESSAGE_ITERATOR_CLASS_INITIALIZE_METHOD
    (tracers, ctf_component_iterator_initialize);
BT_PLUGIN_SOURCE_COMPONENT_CLASS_MESSAGE_ITERATOR_CLASS_FINALIZE_METHOD
    (tracers, ctf_component_iterator_finalize);
/* Plugin metadata */
BT_PLUGIN_AUTHOR ("RidgeRun <support@ridgerun.com>");
BT_PLUGIN_DESCRIPTION ("GStreamer tracing subsystem");
BT_PLUGIN_LICENSE (GST_SHARK_LICENSE);
BT_PLUGIN_VERSION (GST_VERSION_MAJOR, GST_VERSION_MINOR,
    GST_VERSION_MICRO, GST_VERSION_NANO);

/* Component metadata */
BT_PLUGIN_SOURCE_COMPONENT_CLASS_DESCRIPTION (tracers,
    "Connect to GStreamer tracers");
BT_PLUGIN_SOURCE_COMPONENT_CLASS_HELP (tracers,
    "See https://gstreamer.freedesktop.org/documentation/additional/"
    "design/tracing.html for more details");
