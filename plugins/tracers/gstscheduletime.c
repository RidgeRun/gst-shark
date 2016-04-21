/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016 RidgeRun Engineering <carlos.rodriguez@ridgerun.com>
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
 * SECTION:gstscheduletime
 * @short_description: log scheduling time, which is the time between one incoming buffer and the next incoming buffer in a sinkpad
 *
 * A tracing module that take scheduletime() snapshots and logs them.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <unistd.h>
#include "gstscheduletime.h"
#include "gstctf.h"

#ifdef HAVE_SYS_RESOURCE_H
#ifndef __USE_GNU
# define __USE_GNU              /* SCHEDULETIME_THREAD */
#endif
#include <sys/resource.h>
#endif

GST_DEBUG_CATEGORY_STATIC (gst_scheduletime_debug);
#define GST_CAT_DEFAULT gst_scheduletime_debug

G_LOCK_DEFINE (_proc);

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_scheduletime_debug, "scheduletime", 0, "scheduletime tracer");
#define gst_scheduletime_tracer_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstScheduletimeTracer, gst_scheduletime_tracer,
    GST_TYPE_TRACER, _do_init);

#ifdef EVAL
#define EVAL_TIME 10
#endif

static const gchar scheduling_metadata_event[] = "event {\n\
	name = scheduling;\n\
	id = %d;\n\
	stream_id = %d;\n\
	fields := struct {\n\
		string elementname;\n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _time;\n\
	};\n\
};\n\
\n";

static void
schedule_pad_destroy (gpointer data)
{
  g_free (data);
}

static void
do_push_buffer_pre (GstTracer * self, guint64 ts, GstPad * pad)
{
  GstScheduletimeTracer *scheduletimeTracer;
  GHashTable *schedulepads;
  GstSchedulePad *schedulepad;
  GstPad *padPeer;
  GstElement *element;

#ifdef EVAL
  if (ts > EVAL_TIME * GST_SECOND)
    return;
#endif

  scheduletimeTracer = GST_SCHEDULETIME_TRACER_CAST (self);
  padPeer = gst_pad_get_peer (pad);
  element = gst_pad_get_parent_element (padPeer);
  schedulepads = scheduletimeTracer->schedulepads;
  schedulepad =
      (GstSchedulePad *) g_hash_table_lookup (schedulepads,
      g_strdup_printf ("%s_%s", GST_DEBUG_PAD_NAME (padPeer)));

  if (schedulepad->previous_time != 0) {
    gst_tracer_log_trace (gst_structure_new (GST_ELEMENT_NAME (element),
            "scheduling-time", G_TYPE_UINT64,
            GST_CLOCK_DIFF (schedulepad->previous_time, ts), NULL));
    do_print_scheduling_event (SCHED_TIME_EVENT_ID, GST_ELEMENT_NAME (element),
        GST_CLOCK_DIFF (schedulepad->previous_time, ts));
  }

  schedulepad->previous_time = ts;
}

static void
do_pull_range_pre (GstTracer * self, guint64 ts, GstPad * pad)
{
  GstScheduletimeTracer *scheduletimeTracer;
  GHashTable *schedulepads;
  GstSchedulePad *schedulepad;
  GstElement *element;

#ifdef EVAL
  if (ts > EVAL_TIME * GST_SECOND)
    return;
#endif

  scheduletimeTracer = GST_SCHEDULETIME_TRACER_CAST (self);
  element = gst_pad_get_parent_element (pad);
  schedulepads = scheduletimeTracer->schedulepads;
  schedulepad =
      (GstSchedulePad *) g_hash_table_lookup (schedulepads,
      g_strdup_printf ("%s_%s", GST_DEBUG_PAD_NAME (pad)));

  if (schedulepad->previous_time != 0) {
    gst_tracer_log_trace (gst_structure_new (GST_ELEMENT_NAME (element),
            "scheduling-time", G_TYPE_UINT64,
            GST_CLOCK_DIFF (schedulepad->previous_time, ts), NULL));
    do_print_scheduling_event (SCHED_TIME_EVENT_ID, GST_ELEMENT_NAME (element),
        GST_CLOCK_DIFF (schedulepad->previous_time, ts));
  }

  schedulepad->previous_time = ts;
}

static void
create_schedule_pad (gpointer data, gpointer user_data)
{
  GstScheduletimeTracer *scheduletimeTracer;
  GHashTable *schedulepads;
  GstSchedulePad *schedulepad;
  gchar *pad_sink;

  scheduletimeTracer = GST_SCHEDULETIME_TRACER (user_data);
  schedulepads = scheduletimeTracer->schedulepads;

  schedulepad = g_new0 (GstSchedulePad, 1);
  schedulepad->pad = GST_PAD (data);
  schedulepad->previous_time = 0;
  pad_sink = g_strdup_printf ("%s_%s", GST_DEBUG_PAD_NAME (schedulepad->pad));

  if (!g_hash_table_insert (schedulepads, pad_sink, (gpointer) schedulepad))
    GST_ERROR ("Failed to create schedule pad");
}

static void
do_element_new (GObject * self, GstClockTime ts, GstElement * element)
{
  GstScheduletimeTracer *scheduletimeTracer;

  scheduletimeTracer = GST_SCHEDULETIME_TRACER (self);

  if (!GST_OBJECT_FLAG_IS_SET (element, GST_ELEMENT_FLAG_SOURCE)) {
    g_list_foreach (element->sinkpads, create_schedule_pad, scheduletimeTracer);
  }
}


/* tracer class */

static void
gst_scheduletime_tracer_finalize (GObject * obj)
{
  GstScheduletimeTracer *scheduletimeTracer;
  scheduletimeTracer = GST_SCHEDULETIME_TRACER (obj);

  g_hash_table_destroy (scheduletimeTracer->schedulepads);

  G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
gst_scheduletime_tracer_class_init (GstScheduletimeTracerClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gst_scheduletime_tracer_finalize;
}


static void
gst_scheduletime_tracer_init (GstScheduletimeTracer * self)
{
  GstTracer *tracer = GST_TRACER (self);
  gchar *metadata_event;

  self->schedulepads =
      g_hash_table_new_full (g_str_hash, g_str_equal, schedule_pad_destroy,
      schedule_pad_destroy);

  gst_tracing_register_hook (tracer, "pad-push-pre",
      G_CALLBACK (do_push_buffer_pre));

  gst_tracing_register_hook (tracer, "pad-push-list-pre",
      G_CALLBACK (do_push_buffer_pre));

  gst_tracing_register_hook (tracer, "pad-pull-range-pre",
      G_CALLBACK (do_pull_range_pre));

  gst_tracing_register_hook (tracer, "element-new",
      G_CALLBACK (do_element_new));

  gst_tracer_log_trace (gst_structure_new ("scheduletime.class", "time",
          GST_TYPE_STRUCTURE, gst_structure_new ("value", "type", G_TYPE_GTYPE,
              G_TYPE_INT64, "description", G_TYPE_STRING,
              "Scheduling time (Nanoseconds)", NULL), NULL));

  metadata_event =
      g_strdup_printf (scheduling_metadata_event, SCHED_TIME_EVENT_ID, 0);
  add_metadata_event_struct (metadata_event);
  g_free (metadata_event);
}
