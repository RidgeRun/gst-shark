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
#include <glib/gstdio.h>

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

gpointer cpuusage_thread_func (gpointer data);


static void
do_stats (GstTracer * obj, guint64 ts)
{

}

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
cpuusage_thread_func (gpointer data)
{
  GstCPUUsageTracer *self;
  gdouble *cpu_usage;
  gint msg_id;

  self = (GstCPUUsageTracer *) data;

  cpu_usage = self->cpuusage.cpu_usage;

  while (1) {
    gst_cpu_usage_compute (&self->cpuusage);

    sleep (1);
    for (msg_id = 0; msg_id < self->cpuusage.cpu_num; ++msg_id) {
      gst_tracer_log_trace (gst_structure_new ("cpu",
              "number", G_TYPE_INT, msg_id,
              "load", G_TYPE_DOUBLE, cpu_usage[msg_id] * 100, NULL));
    }

  }
  g_thread_exit (0);

  return NULL;
}

static void
gst_cpuusage_tracer_init (GstCPUUsageTracer * self)
{
  gint32 cpu_num;

  GstTracer *tracer = GST_TRACER (self);

  if ((cpu_num = sysconf (_SC_NPROCESSORS_CONF)) == -1) {
    GST_WARNING ("failed to get number of cpus");
    cpu_num = 1;
  }

  gst_tracing_register_hook (tracer, "pad-push-pre", G_CALLBACK (do_stats));


  /* Create new thread to compute the cpu usage periodically */
  g_thread_new ("cpuusage_compute", cpuusage_thread_func, self);

  self->cpuusage.cpu_array_sel = 0;
  self->cpuusage.cpu_num = cpu_num;


  gst_tracer_log_trace (gst_structure_new ("cpuusage.class", "number", GST_TYPE_STRUCTURE, gst_structure_new ("value", "type", G_TYPE_GTYPE, G_TYPE_INT, "description", G_TYPE_STRING, "Core number", "flags", G_TYPE_STRING, "aggregated",     /* TODO: use gflags */
              "min", G_TYPE_UINT, G_GINT64_CONSTANT (0), "max", G_TYPE_UINT, CPU_NUM_MAX, NULL), "load", GST_TYPE_STRUCTURE, gst_structure_new ("value", "type", G_TYPE_GTYPE, G_TYPE_DOUBLE, "description", G_TYPE_STRING, "Core load percentage", "flags", G_TYPE_STRING, "aggregated",       /* TODO: use gflags */
              "min", G_TYPE_DOUBLE, G_GUINT64_CONSTANT (0),
              "max", G_TYPE_DOUBLE, G_GUINT64_CONSTANT (100), NULL), NULL));
}
