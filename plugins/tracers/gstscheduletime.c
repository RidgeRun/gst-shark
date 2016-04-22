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

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_scheduletime_debug, "scheduletime", 0, "scheduletime tracer");
#define gst_scheduletime_tracer_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstScheduletimeTracer, gst_scheduletime_tracer,
    GST_TYPE_TRACER, _do_init);

#define PAD_NAME_SIZE  (64)

#ifdef EVAL
#define EVAL_TIME (10)
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
key_destroy (gpointer pad)
{
  gst_object_unref (pad);
}

static void
schedule_pad_destroy (gpointer data)
{
  g_free (data);
}


static void
sched_time_compute (GstTracer * self, guint64 ts, GstPad * pad)
{
  GObject *obj;
  GstScheduletimeTracer *schedule_time_tracer;
  GHashTable *schedule_pads;
  GstSchedulePad *schedule_pad;
  GstSchedulePad *schedule_pad_new;
  GString *time_string;
  gchar pad_name[PAD_NAME_SIZE];
  guint64 time_diff;

#ifdef EVAL
  if (ts > EVAL_TIME * GST_SECOND)
    return;
#endif

  obj = (GObject *) self;
  schedule_time_tracer = GST_SCHEDULETIME_TRACER_CAST (self);
  schedule_pads = schedule_time_tracer->schedule_pads;

  g_snprintf (pad_name, PAD_NAME_SIZE, "%s_%s", GST_DEBUG_PAD_NAME (pad));
  schedule_pad = (GstSchedulePad *) g_hash_table_lookup (schedule_pads, pad);

  if (NULL == schedule_pad) {
    /* Add new pad sink */
    schedule_pad_new = g_new0 (GstSchedulePad, 1);
    schedule_pad_new->pad = pad;
    schedule_pad_new->previous_time = 0;
    gst_object_ref (pad);
    if (!g_hash_table_insert (schedule_pads, pad, (gpointer) schedule_pad_new)) {
      GST_ERROR_OBJECT (obj, "Failed to create schedule pad");
    }
    return;
  }

  if (schedule_pad->previous_time != 0) {
    time_string = schedule_time_tracer->time_string;
    time_diff = GST_CLOCK_DIFF (schedule_pad->previous_time, ts);
    g_string_printf (time_string, "%" GST_TIME_FORMAT,
        GST_TIME_ARGS (time_diff));

    gst_tracer_log_trace (gst_structure_new (pad_name,
            "scheduling-time", G_TYPE_STRING, time_string->str, NULL));
    do_print_scheduling_event (SCHED_TIME_EVENT_ID, pad_name, time_diff);
  }
  schedule_pad->previous_time = ts;
}

static void
do_push_buffer_pre (GstTracer * self, guint64 ts, GstPad * pad)
{
  GstPad *pad_peer;

  pad_peer = gst_pad_get_peer (pad);

  sched_time_compute (self, ts, pad_peer);

  gst_object_unref (pad_peer);
}

/* tracer class */

static void
gst_scheduletime_tracer_finalize (GObject * obj)
{
  GstScheduletimeTracer *schedule_time_tracer;
  schedule_time_tracer = GST_SCHEDULETIME_TRACER (obj);

  g_string_free (schedule_time_tracer->time_string, TRUE);

  g_hash_table_destroy (schedule_time_tracer->schedule_pads);

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

  self->time_string = g_string_new ("0:00:00.000000000 ");

  self->schedule_pads =
      g_hash_table_new_full (g_direct_hash, g_direct_equal, key_destroy,
      schedule_pad_destroy);

  gst_tracing_register_hook (tracer, "pad-push-pre",
      G_CALLBACK (do_push_buffer_pre));

  gst_tracing_register_hook (tracer, "pad-push-list-pre",
      G_CALLBACK (do_push_buffer_pre));

  gst_tracing_register_hook (tracer, "pad-pull-range-pre",
      G_CALLBACK (sched_time_compute));

  gst_tracer_log_trace (gst_structure_new ("scheduletime.class", "time",
          GST_TYPE_STRUCTURE, gst_structure_new ("value", "type", G_TYPE_GTYPE,
              G_TYPE_INT64, "description", G_TYPE_STRING,
              "Scheduling time (Nanoseconds)", NULL), NULL));

  metadata_event =
      g_strdup_printf (scheduling_metadata_event, SCHED_TIME_EVENT_ID, 0);
  add_metadata_event_struct (metadata_event);
  g_free (metadata_event);
}
