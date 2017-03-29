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
    GST_TYPE_TRACER, _do_init);

#ifdef EVAL
#define EVAL_TIME (10)
#endif

static void do_queue_level (GstTracer * self, guint64 ts, GstPad * pad);
static gboolean is_queue (GstElement * element);



#ifdef GST_STABLE_RELEASE
static GstTracerRecord *tr_qlevel;
#endif

static const gchar scheduling_metadata_event[] = "event {\n\
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

#ifdef EVAL
  if (ts > EVAL_TIME * GST_SECOND) {
    goto out;
  }
#endif

  element = gst_pad_get_parent_element (pad);

  if (!is_queue (element)) {
    goto noqueue;
  }

noqueue:
  {
    gst_object_unref (element);
  }
out:
  {
    return;
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
  GstTracer *tracer = GST_TRACER (self);
  gchar *metadata_event;

  gst_tracing_register_hook (tracer, "pad-push-post",
      G_CALLBACK (do_queue_level));

  gst_tracing_register_hook (tracer, "pad-push-list-post",
      G_CALLBACK (do_queue_level));

  gst_tracing_register_hook (tracer, "pad-pull-range-post",
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
      g_strdup_printf (scheduling_metadata_event, SCHED_TIME_EVENT_ID, 0);
  add_metadata_event_struct (metadata_event);
  g_free (metadata_event);
}
