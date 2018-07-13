/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016-2017 RidgeRun Engineering <carlos.rodriguez@ridgerun.com>
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
 * SECTION:gstqueuelevel
 * @short_description: log current queue level
 *
 * A tracing module that takes queue's current level
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "gstqueuelevel.h"
#include "gstctf.h"

GST_DEBUG_CATEGORY_STATIC (gst_queue_level_debug);
#define GST_CAT_DEFAULT gst_queue_level_debug

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_queue_level_debug, "queuelevel", 0, "queuelevel tracer");
#define gst_queue_level_tracer_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstQueueLevelTracer, gst_queue_level_tracer,
    GST_SHARK_TYPE_TRACER, _do_init);

static void do_queue_level (GstTracer * self, guint64 ts, GstPad * pad);
static gboolean is_queue (GstElement * element);



#ifdef GST_STABLE_RELEASE
static GstTracerRecord *tr_qlevel;
#endif

static const gchar queue_level_metadata_event[] = "event {\n\
    name = queuelevel;\n\
    id = %d;\n\
    stream_id = %d;\n\
    fields := struct {\n\
        string queue;\n\
        integer { size = 32; align = 8; signed = 0; encoding = none; base = 10; } size_bytes;\n\
        integer { size = 32; align = 8; signed = 0; encoding = none; base = 10; } size_buffers;\n\
        integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } size_time;\n\
    };\n\
};\n\
\n";

static void
do_queue_level (GstTracer * self, guint64 ts, GstPad * pad)
{
  GstElement *element;
  guint32 size_bytes;
  guint32 size_buffers;
  guint64 size_time;
  gchar *size_time_string;
  const gchar *element_name;

  element = gst_pad_get_parent_element (pad);

  if (!is_queue (element)) {
    goto out;
  }

  element_name = GST_OBJECT_NAME (element);

  g_object_get (element, "current-level-bytes", &size_bytes,
      "current-level-buffers", &size_buffers,
      "current-level-time", &size_time, NULL);

  size_time_string =
      g_strdup_printf ("%" GST_TIME_FORMAT, GST_TIME_ARGS (size_time));

#ifdef GST_STABLE_RELEASE
  gst_tracer_record_log (tr_qlevel, element_name, size_bytes, size_buffers,
      size_time_string);
#else
  gst_tracer_log_trace (gst_structure_new ("queuelevel",
          "queue", G_TYPE_STRING, element_name,
          "size_bytes", G_TYPE_UINT, size_bytes,
          "size_buffers", G_TYPE_UINT, size_buffers,
          "size_time", G_TYPE_STRING, size_time_string, NULL));
#endif
  g_free (size_time_string);

  do_print_queue_level_event (QUEUE_LEVEL_EVENT_ID, element_name, size_bytes,
      size_buffers, size_time);

out:
  {
    gst_object_unref (element);
  }
}

static gboolean
is_queue (GstElement * element)
{
  static GstElementFactory *qfactory = NULL;
  GstElementFactory *efactory;

  g_return_val_if_fail (element, FALSE);

  /* Find the queue factory that is going to be compared against
     the element under inspection to see if it is a queue */
  if (NULL == qfactory) {
    qfactory = gst_element_factory_find ("queue");
  }

  efactory = gst_element_get_factory (element);

  return efactory == qfactory;
}

/* tracer class */
static void
gst_queue_level_tracer_class_init (GstQueueLevelTracerClass * klass)
{
}


static void
gst_queue_level_tracer_init (GstQueueLevelTracer * self)
{
  GstSharkTracer *tracer = GST_SHARK_TRACER (self);
  gchar *metadata_event;

  gst_shark_tracer_register_hook (tracer, "pad-push-pre",
      G_CALLBACK (do_queue_level));

  gst_shark_tracer_register_hook (tracer, "pad-push-list-pre",
      G_CALLBACK (do_queue_level));

  gst_shark_tracer_register_hook (tracer, "pad-pull-range-pre",
      G_CALLBACK (do_queue_level));

#ifdef GST_STABLE_RELEASE
  tr_qlevel = gst_tracer_record_new ("queuelevel.class",
      "queue", GST_TYPE_STRUCTURE, gst_structure_new ("scope",
          "type", G_TYPE_GTYPE, G_TYPE_STRING,
          "related-to", GST_TYPE_TRACER_VALUE_SCOPE,
          GST_TRACER_VALUE_SCOPE_ELEMENT, NULL), "size_bytes",
      GST_TYPE_STRUCTURE, gst_structure_new ("scope", "type", G_TYPE_GTYPE,
          G_TYPE_UINT, "related-to", GST_TYPE_TRACER_VALUE_SCOPE,
          GST_TRACER_VALUE_SCOPE_ELEMENT, NULL), "size_buffers",
      GST_TYPE_STRUCTURE, gst_structure_new ("scope", "type", G_TYPE_GTYPE,
          G_TYPE_UINT, "related-to", GST_TYPE_TRACER_VALUE_SCOPE,
          GST_TRACER_VALUE_SCOPE_ELEMENT, NULL), "size_time",
      GST_TYPE_STRUCTURE, gst_structure_new ("scope", "type", G_TYPE_GTYPE,
          G_TYPE_STRING, "related-to", GST_TYPE_TRACER_VALUE_SCOPE,
          GST_TRACER_VALUE_SCOPE_ELEMENT, NULL), NULL);
#else
  gst_tracer_log_trace (gst_structure_new ("queuelevel.class",
          "queue", GST_TYPE_STRUCTURE, gst_structure_new ("scope", "related-to",
              G_TYPE_STRING, "element", NULL), "size_bytes", GST_TYPE_STRUCTURE,
          gst_structure_new ("scope", "related-to", G_TYPE_STRING, "element",
              NULL), "size_buffers", GST_TYPE_STRUCTURE,
          gst_structure_new ("scope", "related-to", G_TYPE_STRING, "element",
              NULL), "size_time", GST_TYPE_STRUCTURE,
          gst_structure_new ("scope", "related-to", G_TYPE_STRING, "element",
              NULL), NULL));
#endif

  metadata_event =
      g_strdup_printf (queue_level_metadata_event, QUEUE_LEVEL_EVENT_ID, 0);
  add_metadata_event_struct (metadata_event);
  g_free (metadata_event);
}
