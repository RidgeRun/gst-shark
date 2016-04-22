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
#include "gstcpuusagecompute.h"
#include "gstctf.h"
#include <glib/gstdio.h>

#ifdef HAVE_SYS_RESOURCE_H
#ifndef __USE_GNU
# define __USE_GNU              /* CPUUSAGE_THREAD */
#endif
#include <sys/resource.h>
#endif

GST_DEBUG_CATEGORY_STATIC (gst_cpuusage_debug);
#define GST_CAT_DEFAULT gst_cpuusage_debug

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_cpuusage_debug, "cpuusage", 0, "cpuusage tracer");
#define gst_cpuusage_tracer_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstCPUUsageTracer, gst_cpuusage_tracer,
    GST_TYPE_TRACER, _do_init);

#ifdef EVAL
#define EVAL_TIME (10)
#endif

static const gchar cpuusage_metadata_event[] = "event {\n\
	name = cpuusage;\n\
	id = %d;\n\
	stream_id = %d;\n\
	fields := struct {\n\
		integer { size = 32; align = 8; signed = 0; encoding = none; base = 10; } _cpunum;\n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _cpuload;\n\
	};\n\
};\n\
\n";

gpointer cpu_usage_thread_func (gpointer data);

/* tracer class */

static void
gst_cpuusage_tracer_finalize (GObject * obj)
{
  G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
gst_cpuusage_tracer_class_init (GstCPUUsageTracerClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gst_cpuusage_tracer_finalize;
}

gpointer
cpu_usage_thread_func (gpointer data)
{
  GstCPUUsageTracer *self;
  GstCPUUsage *cpu_usage;
  gdouble *cpu_load;
  gint msg_id;
  gint cpu_num;

#ifdef EVAL
  gint sec_counter;
#endif

  self = (GstCPUUsageTracer *) data;
  cpu_usage = &self->cpu_usage;

  cpu_load = CPU_USAGE_ARRAY (cpu_usage);
  cpu_num = CPU_USAGE_ARRAY_LENGTH (cpu_usage);

  while (1) {
    gst_cpu_usage_compute (cpu_usage);

    for (msg_id = 0; msg_id < cpu_num; ++msg_id) {
      gst_tracer_log_trace (gst_structure_new ("cpu",
              "number", G_TYPE_INT, msg_id,
              "load", G_TYPE_DOUBLE, cpu_load[msg_id] * 100, NULL));
      do_print_cpuusage_event (CPUUSAGE_EVENT_ID, msg_id,
          (int) (cpu_load[msg_id] * 100));
    }
    sleep (1);

#ifdef EVAL
    sec_counter++;
    if (sec_counter > EVAL_TIME)
      break;
#endif

  }
  g_thread_exit (0);

  return NULL;
}

static void
gst_cpuusage_tracer_init (GstCPUUsageTracer * self)
{
  gchar *metadata_event;

  gst_cpu_usage_init (&(self->cpu_usage));

  /* Create new thread to compute the cpu usage periodically */
  g_thread_new ("cpuusage_compute", cpu_usage_thread_func, self);

  gst_tracer_log_trace (gst_structure_new ("cpuusage.class", "number", GST_TYPE_STRUCTURE, gst_structure_new ("value", "type", G_TYPE_GTYPE, G_TYPE_INT, "description", G_TYPE_STRING, "Core number", "flags", G_TYPE_STRING, "aggregated",     /* TODO: use gflags */
              "min", G_TYPE_UINT, G_GINT64_CONSTANT (0), "max", G_TYPE_UINT, CPU_NUM_MAX, NULL), "load", GST_TYPE_STRUCTURE, gst_structure_new ("value", "type", G_TYPE_GTYPE, G_TYPE_DOUBLE, "description", G_TYPE_STRING, "Core load percentage", "flags", G_TYPE_STRING, "aggregated",       /* TODO: use gflags */
              "min", G_TYPE_DOUBLE, G_GUINT64_CONSTANT (0),
              "max", G_TYPE_DOUBLE, G_GUINT64_CONSTANT (100), NULL), NULL));

  metadata_event =
      g_strdup_printf (cpuusage_metadata_event, CPUUSAGE_EVENT_ID, 0);
  add_metadata_event_struct (metadata_event);
  g_free (metadata_event);
}
