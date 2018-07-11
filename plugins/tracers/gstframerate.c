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

/**
 * SECTION: gstframerate
 * @short_description: shows the framerate on every pad in the pipeline.
 *
 * A tracing module that displays the amount of frames per second on every
 * SRC or SINK pad of every element of the running pipeline, depending on 
 * the scheduling mode.
 */

#include "gstframerate.h"
#include "gstctf.h"

GST_DEBUG_CATEGORY_STATIC (gst_framerate_debug);
#define GST_CAT_DEFAULT gst_framerate_debug

#define FRAMERATE_INTERVAL 1

struct _GstFramerateTracer
{
  GstTracer parent;

  GHashTable *frame_counters;
  guint callback_id;
  gboolean metadata_written;
};

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_framerate_debug, "framerate", 0, "framerate tracer");

G_DEFINE_TYPE_WITH_CODE (GstFramerateTracer, gst_framerate_tracer,
    GST_TYPE_TRACER, _do_init);

static GstTracerRecord *tr_framerate;

static gchar *make_char_array_valid (gchar * src);
static void create_metadata_event (GHashTable * table);
static gboolean do_print_framerate (gpointer * data);
static void install_callback (GstFramerateTracer * self);
static void remove_callback (GstFramerateTracer * self);
static void reset_counters (GstFramerateTracer * self);

typedef struct _GstFramerateHash GstFramerateHash;

struct _GstFramerateHash
{
  const gchar *fullname;
  guint counter;
};

static const gchar framerate_metadata_event_header[] = "event {\n\
    name = framerate;\n\
    id = %d;\n\
    stream_id = %d;\n\
    fields := struct {\n";

static const gchar framerate_metadata_event_footer[] = "\
    };\n\
};\n\
\n";

static const gchar framerate_metadata_event_field[] =
    "      integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } %s;\n";

static void
install_callback (GstFramerateTracer * self)
{
  g_return_if_fail (self);

  if (0 != self->callback_id) {
    GST_INFO_OBJECT (self,
        "A framerate timer callback is already installed, ignoring new request");
    return;
  }

  GST_OBJECT_LOCK (self);
  self->callback_id =
      g_timeout_add_seconds (FRAMERATE_INTERVAL,
      (GSourceFunc) do_print_framerate, (gpointer) self);
  GST_OBJECT_UNLOCK (self);
}

static void
remove_callback (GstFramerateTracer * self)
{
  g_return_if_fail (self);

  GST_OBJECT_LOCK (self);
  g_source_remove (self->callback_id);
  self->callback_id = 0;
  GST_OBJECT_UNLOCK (self);
}

static void
reset_counters (GstFramerateTracer * self)
{
  GHashTableIter iter;
  gpointer key, value;
  GstFramerateHash *pad_table;

  g_return_if_fail (self);

  GST_OBJECT_LOCK (self);
  g_hash_table_iter_init (&iter, self->frame_counters);

  while (g_hash_table_iter_next (&iter, &key, &value)) {
    pad_table = (GstFramerateHash *) value;
    pad_table->counter = 0;
  }
  GST_OBJECT_UNLOCK (self);
}

static gboolean
do_print_framerate (gpointer * data)
{
  GstFramerateTracer *self;
  GHashTableIter iter;
  gpointer key, value;
  GstFramerateHash *pad_table;
  guint size;
  guint64 *pad_counts;
  guint32 pad_idx;

  self = GST_FRAMERATE_TRACER (data);

  size = g_hash_table_size (self->frame_counters);
  pad_counts = g_malloc (size * sizeof (guint64));
  pad_idx = 0;

  if (!self->metadata_written) {
    create_metadata_event (self->frame_counters);
    self->metadata_written = TRUE;
  }

  /* Lock the tracer to make sure no new pad is added while we are logging */
  GST_OBJECT_LOCK (self);

  /* Using the iterator functions to go through the Hash table and print the framerate 
     of every element stored */
  g_hash_table_iter_init (&iter, self->frame_counters);
  while (g_hash_table_iter_next (&iter, &key, &value)) {
    pad_table = (GstFramerateHash *) value;

    gst_tracer_record_log (tr_framerate, pad_table->fullname,
        pad_table->counter);

    pad_counts[pad_idx] = pad_table->counter;
    pad_idx++;

    pad_table->counter = 0;
  }

  GST_OBJECT_UNLOCK (self);
  do_print_framerate_event (FPS_EVENT_ID, size, pad_counts);
  g_free (pad_counts);

  return TRUE;
}

static void
do_destroy_hashtable_value (gpointer data)
{
  GstFramerateHash *value;

  /* In order to properly free the memory of a element of the Hash table
     it is needed to free the memory of the structure in every value */
  value = (GstFramerateHash *) data;

  g_free ((gchar *) value->fullname);
  g_free (value);
}

static void
do_destroy_hashtable_key (gpointer data)
{
  gst_object_unref (data);
}

static void
do_pad_push_buffer_pre (GstFramerateTracer * self, guint64 ts, GstPad * pad,
    GstBuffer * buffer)
{
  gchar *fullname;
  gint value = 1;
  GstFramerateHash *pad_frames;

  g_return_if_fail (pad);

  /* The full name of every pad has the format elementName.padName and it is going 
     to be used for displaying the framerate in a friendly user way */
  fullname = g_strdup_printf ("%s_%s", GST_DEBUG_PAD_NAME (pad));

  /* Function contains on the Hash table returns TRUE if the key already exists */
  if (g_hash_table_contains (self->frame_counters, pad)) {
    /* If the pad that is pushing a buffer has already a space on the Hash table
       only the value should be updated */
    pad_frames =
        (GstFramerateHash *) g_hash_table_lookup (self->frame_counters, pad);
    pad_frames->counter++;
  } else {
    GST_INFO_OBJECT (self, "The %s key was added to the Hash Table", fullname);

    /* Ref pad to be used in the Hash table */
    gst_object_ref (pad);
    /* Reserving memory space for every structure that is going to be stored as a 
       value in the Hash table */
    pad_frames = g_malloc (sizeof (GstFramerateHash));
    pad_frames->fullname = make_char_array_valid (g_strdup (fullname));
    pad_frames->counter = value;

    GST_OBJECT_LOCK (self);
    g_hash_table_insert (self->frame_counters, pad, (gpointer) pad_frames);
    GST_OBJECT_UNLOCK (self);
  }

  g_free (fullname);
}

static void
do_pad_push_list_pre (GstFramerateTracer * self, GstClockTime ts, GstPad * pad,
    GstBufferList * list)
{
  // We aren't doing anything with the buffer, we can reuse push_buffer using NULL
  do_pad_push_buffer_pre (self, ts, pad, NULL);
}

static void
do_pad_pull_range_pre (GstFramerateTracer * self, GstClockTime ts, GstPad * pad,
    guint64 offset, guint size)
{
  // We aren't doing anything with the buffer, we can reuse push_buffer using NULL
  do_pad_push_buffer_pre (self, ts, pad, NULL);
}

static void
do_element_change_state_post (GstFramerateTracer * self, guint64 ts,
    GstElement * element, GstStateChange transition,
    GstStateChangeReturn result)
{
  /* We are only interested in capturing when a pipeline goes to
     playing, but this hook reports for every element in the
     pipeline
   */
  if (FALSE == GST_IS_PIPELINE (element)) {
    return;
  }

  if (transition == GST_STATE_CHANGE_PAUSED_TO_PLAYING) {
    reset_counters (self);
    install_callback (self);
  } else if (transition == GST_STATE_CHANGE_PLAYING_TO_PAUSED) {
    remove_callback (self);
  }
}

/* tracer class */

static void
gst_framerate_tracer_finalize (GObject * obj)
{
  GstFramerateTracer *self = GST_FRAMERATE_TRACER (obj);

  g_hash_table_destroy (self->frame_counters);

  G_OBJECT_CLASS (gst_framerate_tracer_parent_class)->finalize (obj);
}

static void
gst_framerate_tracer_class_init (GstFramerateTracerClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gst_framerate_tracer_finalize;

  tr_framerate = gst_tracer_record_new ("framerate.class",
      "pad", GST_TYPE_STRUCTURE, gst_structure_new ("scope",
          "type", G_TYPE_GTYPE, G_TYPE_STRING,
          "related-to", GST_TYPE_TRACER_VALUE_SCOPE, GST_TRACER_VALUE_SCOPE_PAD,
          NULL),
      "fps", GST_TYPE_STRUCTURE, gst_structure_new ("value",
          "type", G_TYPE_GTYPE, G_TYPE_UINT,
          "description", G_TYPE_STRING, "Frames per second",
          "flags", GST_TYPE_TRACER_VALUE_FLAGS,
          GST_TRACER_VALUE_FLAGS_AGGREGATED, "min", G_TYPE_UINT, 0, "max",
          G_TYPE_UINT, G_MAXUINT, NULL), NULL);
}

static void
gst_framerate_tracer_init (GstFramerateTracer * self)
{
  GstTracer *tracer = GST_TRACER (self);

  self->frame_counters =
      g_hash_table_new_full (g_direct_hash, g_direct_equal,
      do_destroy_hashtable_key, do_destroy_hashtable_value);
  self->callback_id = 0;
  self->metadata_written = FALSE;

  gst_tracing_register_hook (tracer, "pad-push-pre",
      G_CALLBACK (do_pad_push_buffer_pre));
  gst_tracing_register_hook (tracer, "pad-push-list-pre",
      G_CALLBACK (do_pad_push_list_pre));
  gst_tracing_register_hook (tracer, "pad-pull-range-pre",
      G_CALLBACK (do_pad_pull_range_pre));

  gst_tracing_register_hook (tracer, "element-change-state-post",
      G_CALLBACK (do_element_change_state_post));
}

static void
create_metadata_event (GHashTable * frame_counters)
{
  GString *builder;
  GHashTableIter iter;
  gpointer key, value;
  gchar *cstring;

  builder = g_string_new (NULL);

  g_string_printf (builder, framerate_metadata_event_header, FPS_EVENT_ID, 0);

  g_hash_table_iter_init (&iter, frame_counters);
  while (g_hash_table_iter_next (&iter, &key, &value)) {
    g_string_append_printf (builder, framerate_metadata_event_field,
        ((GstFramerateHash *) value)->fullname);
  }

  /* Add event footer */
  g_string_append (builder, framerate_metadata_event_footer);

  cstring = g_string_free (builder, FALSE);

  /* Add event in metadata file */
  add_metadata_event_struct (cstring);
  g_free (cstring);


}

static gchar *
make_char_array_valid (gchar * src)
{
  gchar *c;

  g_return_val_if_fail (src, NULL);

  for (c = src; '\0' != *c; c++) {
    if ('-' == *c) {
      *c = '_';
    }
  }

  return src;
}
