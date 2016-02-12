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
 * SECTION:gstcpuusage
 * @short_description: log cpu usage stats
 *
 * A tracing module that take cpuusage() snapshots and logs them.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <unistd.h>
#include "gstcpuusage.h"
#include <stdio.h>

#ifdef HAVE_SYS_RESOURCE_H
#ifndef __USE_GNU
# define __USE_GNU              /* CPUUSAGE_THREAD */
#endif
#include <sys/resource.h>
#endif

GST_DEBUG_CATEGORY_STATIC (gst_cpuusage_debug);
#define GST_CAT_DEFAULT gst_cpuusage_debug

G_LOCK_DEFINE (_proc);

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_cpuusage_debug, "cpuusage", 0, "cpuusage tracer");
#define gst_cpuusage_tracer_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstCPUUsageTracer, gst_cpuusage_tracer,
    GST_TYPE_TRACER, _do_init);

/* remember x measurements per self->window */
#define WINDOW_SUBDIV 100

typedef struct
{
  /* time spend in this thread */
  GstClockTime tthread;
  GstTraceValues *tvs_thread;
} GstThreadStats;

/* data helper */

static void
free_trace_value (gpointer data)
{
  g_slice_free (GstTraceValue, data);
}

static GstTraceValues *
make_trace_values (GstClockTime window)
{
  GstTraceValues *self = g_slice_new0 (GstTraceValues);
  self->window = window;
  g_queue_init (&self->values);
  return self;
}

static void
free_trace_values (GstTraceValues * self)
{
  g_queue_free_full (&self->values, free_trace_value);
  g_slice_free (GstTraceValues, self);
}


static void
free_thread_stats (gpointer data)
{
  free_trace_values (((GstThreadStats *) data)->tvs_thread);
  g_slice_free (GstThreadStats, data);
}

static void
do_stats (GstTracer * obj, guint64 ts)
{
  printf ("cpuusage: hook\n");
}

/* tracer class */

static void
gst_cpuusage_tracer_finalize (GObject * obj)
{
  GstCPUUsageTracer *self = GST_CPUUSAGE_TRACER (obj);

  g_hash_table_destroy (self->threads);
  free_trace_values (self->tvs_proc);

  G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
gst_cpuusage_tracer_class_init (GstCPUUsageTracerClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gst_cpuusage_tracer_finalize;
}

static void
gst_cpuusage_tracer_init (GstCPUUsageTracer * self)
{
  GstTracer *tracer = GST_TRACER (self);

  gst_tracing_register_hook (tracer, "pad-push-pre", G_CALLBACK (do_stats));

  self->threads = g_hash_table_new_full (NULL, NULL, NULL, free_thread_stats);
  self->tvs_proc = make_trace_values (GST_SECOND);
  self->main_thread_id = g_thread_self ();

  GST_DEBUG ("cpuusage: main thread=%p", self->main_thread_id);
}
