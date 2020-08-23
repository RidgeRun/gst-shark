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

GST_DEBUG_CATEGORY_EXTERN (gst_ctf_debug);
#define GST_CAT_DEFAULT gst_ctf_debug

struct _GstCtfComponent
{
  GstObject base;

  bt_stream *stream;
  bt_event_class *event_class;
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

/* Sefl */
static bt_event_class *gst_ctf_component_create_event_class (GstCtfComponent *
    self, bt_stream_class * stream_class, const char *name);
static bt_component_class_initialize_method_status
gst_ctf_component_create_metadata_and_stream (GstCtfComponent * self,
    bt_self_component * component);


static void
gst_ctf_component_init (GstCtfComponent * self)
{
  self->stream = NULL;
  self->event_class = NULL;
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

  bt_stream_put_ref (self->stream);
  self->stream = NULL;

  bt_event_class_put_ref (self->event_class);
  self->event_class = NULL;

  return G_OBJECT_CLASS (gst_ctf_component_parent_class)->finalize (object);
}


static bt_event_class *
gst_ctf_component_create_event_class (GstCtfComponent * self,
    bt_stream_class * stream_class, const char *name)
{
  bt_event_class *event_class = NULL;
  bt_event_class *event_class_ret = NULL;
  bt_trace_class *trace_class = NULL;
  bt_field_class *payload_field_class = NULL;
  bt_field_class *msg_field_class = NULL;
  gint ret = BT_EVENT_CLASS_SET_NAME_STATUS_OK;

  g_return_val_if_fail (self, NULL);
  g_return_val_if_fail (stream_class, NULL);
  g_return_val_if_fail (name, NULL);

  /* Borrow trace class from stream class */
  trace_class = bt_stream_class_borrow_trace_class (stream_class);
  g_return_val_if_fail (trace_class, NULL);

  /* Create a default event class */
  event_class = bt_event_class_create (stream_class);
  if (NULL == event_class) {
    GST_ERROR_OBJECT (self, "Unable to create event class for \"%s\"", name);
    goto out;
  }

  /* Name the event class */
  ret = bt_event_class_set_name (event_class, name);
  if (BT_EVENT_CLASS_SET_NAME_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self, "Unable to create event class for \"%s\": %d", name,
        ret);
    goto free_event_class;
  }

  /*
   * Create an empty structure field class to be used as the
   * event class's payload field class.
   */
  payload_field_class = bt_field_class_structure_create (trace_class);
  if (NULL == payload_field_class) {
    GST_ERROR_OBJECT (self,
        "Unable to create the payload field class for \"%s\"", name);
    goto free_event_class;
  }

  /*
   * Create a string field class to be used as the payload field
   * class's `msg` member.
   */
  msg_field_class = bt_field_class_string_create (trace_class);
  if (NULL == msg_field_class) {
    GST_ERROR_OBJECT (self, "Unable to create the msg field class for \"%s\"",
        name);
    goto free_payload_field;
  }

  /*
   * Append the string field class to the structure field class as the
   * `msg` member.
   */
  ret = bt_field_class_structure_append_member (payload_field_class, "msg",
      msg_field_class);
  if (BT_FIELD_CLASS_STRUCTURE_APPEND_MEMBER_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self, "Unable to append the message field to the payload "
        "for \"%s\": %d", name, ret);
    goto free_msg_field;
  }

  /* Set the event class's payload field class */
  ret =
      bt_event_class_set_payload_field_class (event_class, payload_field_class);
  if (BT_EVENT_CLASS_SET_FIELD_CLASS_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self,
        "Unable to set the payload field in the event class " "for \"%s\": %d",
        name, ret);
    goto free_msg_field;
  }

  /* Success, save a ref and exit */
  GST_INFO_OBJECT (self, "Successfully created event class \"%s\"", name);
  bt_event_class_get_ref (event_class);
  event_class_ret = event_class;

free_msg_field:
  bt_field_class_put_ref (msg_field_class);

free_payload_field:
  bt_field_class_put_ref (payload_field_class);

free_event_class:
  bt_event_class_put_ref (event_class);

out:
  return event_class_ret;
}

static bt_component_class_initialize_method_status
gst_ctf_component_create_metadata_and_stream (GstCtfComponent * self,
    bt_self_component * component)
{
  bt_trace_class *trace_class = NULL;
  bt_stream_class *stream_class = NULL;
  bt_clock_class *clock_class = NULL;
  bt_event_class *event_class = NULL;
  bt_trace *trace = NULL;
  bt_stream *stream = NULL;
  gint ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_OK;
  const gchar *event_class_name = "event";

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

  stream = bt_stream_create (stream_class, trace);
  if (NULL == trace) {
    GST_ERROR_OBJECT (self, "Unable to create stream");
    ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_ERROR;
    goto free_trace;
  }

  /* Create the two event classes we need */
  event_class =
      gst_ctf_component_create_event_class (self, stream_class,
      event_class_name);
  if (NULL == event_class) {
    GST_ERROR_OBJECT (self, "Unable to create event class");
    ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_ERROR;
    goto free_stream;
  }

  self->event_class = event_class;

  bt_stream_get_ref (stream);
  self->stream = stream;

  ret = BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_OK;

free_stream:
  bt_stream_put_ref (stream);

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

  gst_object_unref (self);
}

static bt_message_iterator_class_next_method_status
ctf_component_iterator_next (bt_self_message_iterator * self_message_iterator,
    bt_message_array_const messages, guint64 capacity, guint64 * count)
{
  gint status = BT_MESSAGE_ITERATOR_CLASS_NEXT_METHOD_STATUS_OK;

  return status;
}

/* Define plugin and source component */
BT_PLUGIN_MODULE ();
BT_PLUGIN (gst);
BT_PLUGIN_SOURCE_COMPONENT_CLASS (tracers, ctf_component_iterator_next);
BT_PLUGIN_SOURCE_COMPONENT_CLASS_INITIALIZE_METHOD (tracers,
    ctf_component_initialize);
BT_PLUGIN_SOURCE_COMPONENT_CLASS_FINALIZE_METHOD (tracers,
    ctf_component_finalize);

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
