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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <unistd.h>
#include "gstproctime.h"
#include "gstctf.h"

#ifdef HAVE_SYS_RESOURCE_H
#ifndef __USE_GNU
# define __USE_GNU              /* PROCTIME_THREAD */
#endif
#include <sys/resource.h>
#endif

GST_DEBUG_CATEGORY_STATIC (gst_proctime_debug);
#define GST_CAT_DEFAULT gst_proctime_debug

G_LOCK_DEFINE (_proc);

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_proctime_debug, "proctime", 0, "proctime tracer");
#define gst_proctime_tracer_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstProcTimeTracer, gst_proctime_tracer,
    GST_TYPE_TRACER, _do_init);

static const char proctime_metadata_event[] = "event {\n\
	name = proctime;\n\
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
  GstProcTimeTracer *procTimeTracer;
  GstProcTime *procTime;

  gchar *parentName;
  gchar *peerParentName;
  GstElement *element;
  GstPad *padPeer;
  gchar *name;
  GstClockTime time;
  GString *timeString;

  procTimeTracer = GST_PROCTIME_TRACER_CAST (self);
  procTime = &procTimeTracer->procTime;
  timeString = procTimeTracer->timeString;

  element = gst_pad_get_parent_element (pad);
  parentName = gst_element_get_name (element);

  padPeer = gst_pad_get_peer (pad);
  element = gst_pad_get_parent_element (padPeer);
  peerParentName = gst_element_get_name (element);

  g_free (parentName);
  g_free (peerParentName);

  gst_proctime_proc_time (procTime, &time, &name, padPeer, pad);
  if (NULL != name) {
    g_string_printf (timeString, "%" GST_TIME_FORMAT, GST_TIME_ARGS (time));

    gst_tracer_log_trace (gst_structure_new (name,
            "time", G_TYPE_STRING, timeString->str, NULL));

    do_print_proctime_event (PROCTIME_EVENT_ID, name, time);
  }
}


static void
do_element_new (GObject * self, GstClockTime ts, GstElement * element)
{
  GstProcTimeTracer *procTimeTracer;
  GstProcTime *procTime;

  procTimeTracer = GST_PROCTIME_TRACER (self);
  procTime = &procTimeTracer->procTime;

  gst_proctime_add_new_element (procTime, element);
}


/* tracer class */

static void
gst_proctime_tracer_finalize (GObject * obj)
{
  GstProcTimeTracer *procTimeTracer;
  GstProcTime *procTime;

  procTimeTracer = GST_PROCTIME_TRACER (obj);
  procTime = &procTimeTracer->procTime;

  g_string_free (procTimeTracer->timeString, TRUE);

  gst_proctime_finalize (procTime);

  G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
gst_proctime_tracer_class_init (GstProcTimeTracerClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gst_proctime_tracer_finalize;
}


static void
gst_proctime_tracer_init (GstProcTimeTracer * self)
{
  GstTracer *tracer = GST_TRACER (self);
  gchar *metadata_event;

  self->timeString = g_string_new ("0:00:00.000000000 ");

  gst_tracing_register_hook (tracer, "pad-push-pre",
      G_CALLBACK (do_push_buffer_pre));

  gst_tracing_register_hook (tracer, "element-new",
      G_CALLBACK (do_element_new));

  gst_tracer_log_trace (gst_structure_new ("proctime.class", "time",
          GST_TYPE_STRUCTURE, gst_structure_new ("value", "type", G_TYPE_GTYPE,
              G_TYPE_INT64, "description", G_TYPE_STRING,
              "Processing time (Microseconds)", NULL), NULL));

  metadata_event =
      g_strdup_printf (proctime_metadata_event, PROCTIME_EVENT_ID, 0);
  add_metadata_event_struct (metadata_event);
  g_free (metadata_event);
}
