/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016 RidgeRun Engineering <manuel.leiva@ridgerun.com>
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
 * SECTION:gstproctime
 * @short_description: log cpu usage stats
 *
 * A tracing module that take proctime() snapshots and logs them.
 */

#include "gstproctime.h"
#include "gstctf.h"

GST_DEBUG_CATEGORY_STATIC (gst_proc_time_debug);
#define GST_CAT_DEFAULT gst_proc_time_debug

/**
 * GstProcTimeTracer:
 *
 * Opaque #GstProcTimeTracer data structure
 */
struct _GstProcTimeTracer
{
  GstTracer parent;

  GstProcTime proc_time;
};

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_proc_time_debug, "proctime", 0, "proctime tracer");

G_DEFINE_TYPE_WITH_CODE (GstProcTimeTracer, gst_proc_time_tracer,
    GST_TYPE_TRACER, _do_init);

static GstTracerRecord *tr_proc_time;

static const gchar proc_time_metadata_event[] = "event {\n\
    name = proc_time;\n\
    id = %d;\n\
    stream_id = %d;\n\
    fields := struct {\n\
        string element; \n\
        integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _time;\n\
    };\n\
};\n\
\n";

static void
do_push_buffer_pre (GstTracer * self, guint64 ts, GstPad * pad)
{
  GstProcTimeTracer *proc_time_tracer;
  GstProcTime *proc_time;

  GstPad *pad_peer;
  gchar *name;
  GstClockTime time;
  GString *time_string = NULL;

  proc_time_tracer = GST_PROC_TIME_TRACER (self);
  proc_time = &proc_time_tracer->proc_time;
  time_string = g_string_new ("");

  pad_peer = gst_pad_get_peer (pad);

  gst_proctime_proc_time (proc_time, &time, &name, pad_peer, pad);

  if (NULL != name) {
    g_string_printf (time_string, "%" GST_TIME_FORMAT, GST_TIME_ARGS (time));

    gst_tracer_record_log (tr_proc_time, name, time_string->str);

    do_print_proctime_event (PROCTIME_EVENT_ID, name, time);
  }

  g_string_free (time_string, TRUE);
  gst_object_unref (pad_peer);
}

static void
do_element_new (GObject * self, GstClockTime ts, GstElement * element)
{
  GstProcTimeTracer *proc_time_tracer;
  GstProcTime *proc_time;

  proc_time_tracer = GST_PROC_TIME_TRACER (self);
  proc_time = &proc_time_tracer->proc_time;

  gst_proctime_add_new_element (proc_time, element);
}

/* tracer class */

static void
gst_proc_time_tracer_finalize (GObject * obj)
{
  GstProcTimeTracer *proc_time_tracer;
  GstProcTime *proc_time;

  proc_time_tracer = GST_PROC_TIME_TRACER (obj);
  proc_time = &proc_time_tracer->proc_time;

  gst_proctime_finalize (proc_time);

  G_OBJECT_CLASS (gst_proc_time_tracer_parent_class)->finalize (obj);
}

static void
gst_proc_time_tracer_class_init (GstProcTimeTracerClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gst_proc_time_tracer_finalize;

  tr_proc_time = gst_tracer_record_new ("proc_time.class",
      "element", GST_TYPE_STRUCTURE, gst_structure_new ("scope",
          "type", G_TYPE_GTYPE, G_TYPE_STRING,
          "related-to", GST_TYPE_TRACER_VALUE_SCOPE,
          GST_TRACER_VALUE_SCOPE_ELEMENT, NULL), "time", GST_TYPE_STRUCTURE,
      gst_structure_new ("scope", "type", G_TYPE_GTYPE, G_TYPE_STRING,
          "related-to", GST_TYPE_TRACER_VALUE_SCOPE,
          GST_TRACER_VALUE_SCOPE_PROCESS, NULL), NULL);
}


static void
gst_proc_time_tracer_init (GstProcTimeTracer * self)
{
  GstTracer *tracer = GST_TRACER (self);
  gchar *metadata_event;

  gst_tracing_register_hook (tracer, "pad-push-pre",
      G_CALLBACK (do_push_buffer_pre));

  gst_tracing_register_hook (tracer, "element-new",
      G_CALLBACK (do_element_new));

  metadata_event =
      g_strdup_printf (proc_time_metadata_event, PROCTIME_EVENT_ID, 0);
  add_metadata_event_struct (metadata_event);
  g_free (metadata_event);
}
