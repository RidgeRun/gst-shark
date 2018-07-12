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

typedef struct _GstSchedulePad GstSchedulePad;

struct _GstSchedulePad
{
  GstPad *pad;
  GstClockTime previous_time;
};

struct _GstScheduletimeTracer
{
  GstTracer parent;
  GHashTable *schedule_pads;
};

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_scheduletime_debug, "scheduletime", 0, "scheduletime tracer");

G_DEFINE_TYPE_WITH_CODE (GstScheduletimeTracer, gst_scheduletime_tracer,
    GST_TYPE_TRACER, _do_init);

#define PAD_NAME_SIZE  (64)

static GstTracerRecord *tr_schedule;

static const gchar scheduling_metadata_event[] = "event {\n\
    name = scheduling;\n\
    id = %d;\n\
    stream_id = %d;\n\
    fields := struct {\n\
        string pad;\n\
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
  GString *time_string = NULL;
  gchar pad_name[PAD_NAME_SIZE];
  guint64 time_diff;

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
    time_string = g_string_new ("");
    time_diff = GST_CLOCK_DIFF (schedule_pad->previous_time, ts);
    g_string_printf (time_string, "%" GST_TIME_FORMAT,
        GST_TIME_ARGS (time_diff));

#ifdef GST_STABLE_RELEASE
    gst_tracer_record_log (tr_schedule, pad_name, time_string->str);
#else
    gst_tracer_log_trace (gst_structure_new ("scheduletime",
            "pad", G_TYPE_STRING, pad_name,
            "time", G_TYPE_STRING, time_string->str, NULL));
#endif
    do_print_scheduling_event (SCHED_TIME_EVENT_ID, pad_name, time_diff);
    g_string_free (time_string, TRUE);
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

  g_hash_table_destroy (schedule_time_tracer->schedule_pads);

  G_OBJECT_CLASS (gst_scheduletime_tracer_parent_class)->finalize (obj);
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

  self->schedule_pads =
      g_hash_table_new_full (g_direct_hash, g_direct_equal, key_destroy,
      schedule_pad_destroy);

  gst_tracing_register_hook (tracer, "pad-push-pre",
      G_CALLBACK (do_push_buffer_pre));

  gst_tracing_register_hook (tracer, "pad-push-list-pre",
      G_CALLBACK (do_push_buffer_pre));

  gst_tracing_register_hook (tracer, "pad-pull-range-pre",
      G_CALLBACK (sched_time_compute));

#ifdef GST_STABLE_RELEASE
  tr_schedule = gst_tracer_record_new ("scheduletime.class",
      "pad", GST_TYPE_STRUCTURE, gst_structure_new ("scope",
          "type", G_TYPE_GTYPE, G_TYPE_STRING,
          "related-to", GST_TYPE_TRACER_VALUE_SCOPE, GST_TRACER_VALUE_SCOPE_PAD,
          NULL),
      "time", GST_TYPE_STRUCTURE, gst_structure_new ("scope",
          "type", G_TYPE_GTYPE, G_TYPE_STRING,
          "related-to", GST_TYPE_TRACER_VALUE_SCOPE,
          GST_TRACER_VALUE_SCOPE_PROCESS, NULL), NULL);
#else
  gst_tracer_log_trace (gst_structure_new ("scheduletime.class", "pad", GST_TYPE_STRUCTURE, gst_structure_new ("scope", "related-to", G_TYPE_STRING, "pad",     /* TODO: use genum */
              NULL), "time", GST_TYPE_STRUCTURE, gst_structure_new ("scope", "related-to", G_TYPE_STRING, "process",    /* TODO: use genum */
              NULL), NULL));
#endif

  metadata_event =
      g_strdup_printf (scheduling_metadata_event, SCHED_TIME_EVENT_ID, 0);
  add_metadata_event_struct (metadata_event);
  g_free (metadata_event);
}
