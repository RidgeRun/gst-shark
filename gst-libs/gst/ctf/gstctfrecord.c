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

#include "gstctfrecordpriv.h"

#include <babeltrace2/babeltrace.h>

GST_DEBUG_CATEGORY_EXTERN (gst_ctf_debug);
#define GST_CAT_DEFAULT gst_ctf_debug

struct _GstCtfRecord
{
  GstObject base;

  bt_self_message_iterator *iterator;
  bt_stream *stream;
  bt_event_class *event_class;
  gchar *name;
};

G_DEFINE_TYPE (GstCtfRecord, gst_ctf_record, GST_TYPE_OBJECT);

typedef bt_field_class *(*BtFieldClassCreate) (bt_trace_class *);

/* GObject */
static void gst_ctf_record_finalize (GObject * self);
static bt_event_class *gst_ctf_record_create_event_class (GstCtfRecord * self,
    bt_stream * stream, const gchar * name, const gchar * fieldname,
    va_list var_args);
static gboolean gst_ctf_record_append_field (GstCtfRecord * self,
    bt_trace_class * trace_class,
    bt_field_class * payload_field_class, const gchar * name,
    BtFieldClassCreate factory, const gchar * typename);
static gboolean gst_ctf_record_iterate_fields (GstCtfRecord * self,
    bt_trace_class * trace_class,
    bt_field_class * payload_field_class, const gchar * name, va_list var_args);
static gboolean gst_ctf_record_append_structure (GstCtfRecord * self,
    bt_trace_class * trace_class,
    bt_field_class * payload_field_class, const gchar * name,
    GstStructure * st);

static const bt_component *iterator_borrow_component (bt_self_message_iterator *
    iterator);

static void
gst_ctf_record_init (GstCtfRecord * self)
{
  self->iterator = NULL;
  self->stream = NULL;
  self->event_class = NULL;
  self->name = NULL;
}

static void
gst_ctf_record_class_init (GstCtfRecordClass * klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->finalize = gst_ctf_record_finalize;
}

static const bt_component *
iterator_borrow_component (bt_self_message_iterator * iterator)
{
  bt_self_component *self_component = NULL;

  g_return_val_if_fail (iterator, NULL);

  self_component = bt_self_message_iterator_borrow_component (iterator);
  g_return_val_if_fail (self_component, NULL);

  return bt_self_component_as_component (self_component);
}

static void
gst_ctf_record_finalize (GObject * object)
{
  GstCtfRecord *self = GST_CTF_RECORD (object);

  /* Iterator's ref is handled in the component */
  bt_component_put_ref (iterator_borrow_component (self->iterator));
  self->iterator = NULL;

  bt_stream_put_ref (self->stream);
  self->stream = NULL;

  bt_event_class_put_ref (self->event_class);
  self->event_class = NULL;

  g_free (self->name);
  self->name = NULL;

  return G_OBJECT_CLASS (gst_ctf_record_parent_class)->finalize (object);
}

static gboolean
gst_ctf_record_append_field (GstCtfRecord * self,
    bt_trace_class * trace_class,
    bt_field_class * payload_field_class, const gchar * name,
    BtFieldClassCreate factory, const gchar * typename)
{
  gboolean ret = FALSE;
  bt_field_class *msg_field_class = NULL;
  gint status = BT_FIELD_CLASS_STRUCTURE_APPEND_MEMBER_STATUS_OK;

  g_return_val_if_fail (self, FALSE);
  g_return_val_if_fail (trace_class, FALSE);
  g_return_val_if_fail (payload_field_class, FALSE);
  g_return_val_if_fail (name, FALSE);

  /*
   * Create a string field class to be used as the payload field
   * class's `msg` member.
   */
  msg_field_class = factory (trace_class);
  if (NULL == msg_field_class) {
    GST_ERROR_OBJECT (self,
        "Unable to create the %s msg field class for \"%s\"", typename, name);
    goto out;
  }

  /*
   * Append the string field class to the structure field class as the
   * `msg` member.
   */
  status = bt_field_class_structure_append_member (payload_field_class, name,
      msg_field_class);
  if (BT_FIELD_CLASS_STRUCTURE_APPEND_MEMBER_STATUS_OK != status) {
    GST_ERROR_OBJECT (self, "Unable to append the message field to the payload "
        "for \"%s\": %d", name, status);
    goto free_msg_field;
  }

  GST_DEBUG_OBJECT (self, "Added \"%s\" field of type %s to record", name,
      typename);
  ret = TRUE;

free_msg_field:
  bt_field_class_put_ref (msg_field_class);

out:
  return ret;
}

static gboolean
gst_ctf_record_append_structure (GstCtfRecord * self,
    bt_trace_class * trace_class,
    bt_field_class * payload_field_class, const gchar * name, GstStructure * st)
{
  gboolean ret = TRUE;
  const gchar *type_field = "type";
  const GValue *value = NULL;
  GType type = 0;
  BtFieldClassCreate factory = NULL;

  g_return_val_if_fail (self, FALSE);
  g_return_val_if_fail (trace_class, FALSE);
  g_return_val_if_fail (payload_field_class, FALSE);
  g_return_val_if_fail (name, FALSE);
  g_return_val_if_fail (st, FALSE);

  /* A tracer record is a structure with the following shape:
   * - "type": (GType) The type of the field
   * - "description": (String) A description of the field
   */
  value = gst_structure_get_value (st, type_field);
  g_return_val_if_fail (value, FALSE);

  type = g_value_get_gtype (value);

  switch (type) {
    case G_TYPE_STRING:
      factory = bt_field_class_string_create;
      break;
    case G_TYPE_BOOLEAN:
      factory = bt_field_class_bool_create;
      break;
    case G_TYPE_UINT:
      factory = bt_field_class_integer_unsigned_create;
      break;
    case G_TYPE_INT:
      factory = bt_field_class_integer_signed_create;
      break;
    case G_TYPE_FLOAT:
      factory = bt_field_class_real_single_precision_create;
      break;
    case G_TYPE_DOUBLE:
      factory = bt_field_class_real_double_precision_create;
      break;
    default:
      GST_FIXME_OBJECT (self, "Type \"%s\" is not supported yet",
          g_type_name (type));
      ret = FALSE;
      goto out;
  }

  ret = gst_ctf_record_append_field (self, trace_class,
      payload_field_class, name, factory, g_type_name (type));

out:
  return ret;
}

static gboolean
gst_ctf_record_iterate_fields (GstCtfRecord * self,
    bt_trace_class * trace_class,
    bt_field_class * payload_field_class, const gchar * name, va_list var_args)
{
  gboolean ret = FALSE;
  GstStructure *st = NULL;
  GType type = 0;
  const gchar *fieldname = NULL;

  g_return_val_if_fail (self, FALSE);
  g_return_val_if_fail (trace_class, FALSE);
  g_return_val_if_fail (payload_field_class, FALSE);
  g_return_val_if_fail (name, FALSE);

  do {
    fieldname = name ? name : va_arg (var_args, const gchar *);
    if (NULL == fieldname) {
      GST_DEBUG_OBJECT (self, "No more entries for this event");
      break;
    }

    type = va_arg (var_args, GType);
    if (GST_TYPE_STRUCTURE != type) {
      GST_ERROR_OBJECT (self,
          "Malformed event, expected GST_TYPE_STRUCTURE after "
          "the field name");
      break;
    }

    st = va_arg (var_args, GstStructure *);
    if (NULL == st) {
      GST_ERROR_OBJECT (self, "Malformed event, expected a GstStructure after "
          "GST_TYPE_STRUCTURE");
      break;
    }

    ret = gst_ctf_record_append_structure (self, trace_class,
        payload_field_class, fieldname, st);

    /* Grab the rest of the fieldnames from the var args */
    name = NULL;
  } while (TRUE == ret);

  return ret;
}

static bt_event_class *
gst_ctf_record_create_event_class (GstCtfRecord * self,
    bt_stream * stream, const char *name, const gchar * fieldname,
    va_list var_args)
{
  bt_event_class *event_class = NULL;
  bt_event_class *event_class_ret = NULL;
  bt_trace_class *trace_class = NULL;
  bt_stream_class *stream_class = NULL;
  bt_field_class *payload_field_class = NULL;
  gint ret = BT_EVENT_CLASS_SET_NAME_STATUS_OK;

  g_return_val_if_fail (self, NULL);
  g_return_val_if_fail (stream, NULL);
  g_return_val_if_fail (name, NULL);
  g_return_val_if_fail (fieldname, NULL);

  stream_class = bt_stream_borrow_class (stream);
  g_return_val_if_fail (stream_class, NULL);

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

  if (FALSE == gst_ctf_record_iterate_fields (self, trace_class,
          payload_field_class, fieldname, var_args)) {
    GST_ERROR_OBJECT (self, "Unable to append payload to \"%s\"", name);
    goto free_payload_class;
  }

  /* Set the event class's payload field class */
  ret =
      bt_event_class_set_payload_field_class (event_class, payload_field_class);
  if (BT_EVENT_CLASS_SET_FIELD_CLASS_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self,
        "Unable to set the payload field in the event class " "for \"%s\": %d",
        name, ret);
    goto free_payload_class;
  }

  /* Success, save a ref and exit */
  GST_INFO_OBJECT (self, "Successfully created event class \"%s\"", name);
  bt_event_class_get_ref (event_class);
  event_class_ret = event_class;

free_payload_class:
  bt_field_class_put_ref (payload_field_class);

free_event_class:
  bt_event_class_put_ref (event_class);

out:
  return event_class_ret;
}

GstCtfRecord *
gst_ctf_record_new_valist (bt_stream * stream,
    bt_self_message_iterator * iterator, const gchar * name,
    const gchar * firstfield, va_list var_args)
{
  GstCtfRecord *self = NULL;

  g_return_val_if_fail (stream, NULL);
  g_return_val_if_fail (iterator, NULL);
  g_return_val_if_fail (name, NULL);
  g_return_val_if_fail (firstfield, NULL);

  self = g_object_new (GST_TYPE_CTF_RECORD, NULL);

  bt_stream_get_ref (stream);
  self->stream = stream;

  bt_component_get_ref (iterator_borrow_component (iterator));
  self->iterator = iterator;

  self->name = g_strdup (name);

  self->event_class =
      gst_ctf_record_create_event_class (self, stream, name, firstfield,
      var_args);
  if (NULL == self->event_class) {
    GST_ERROR_OBJECT (self, "Unable to create CTF record");
    gst_clear_object (&self);
    goto out;
  }

  GST_DEBUG_OBJECT (self, "Registered new even \"%s\"", name);

out:
  return self;
}
