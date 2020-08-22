/* GstShark - A Front End for GstTracer
 * Copyright (C) 2018 RidgeRun Engineering <manuel.leiva@ridgerun.com>
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

#include <glib/gstdio.h>
#include <unistd.h>
#include <string.h>

#include "gstcpuusage.h"
#include "gstcpuusagecompute.h"

GST_DEBUG_CATEGORY_STATIC (gst_cpu_usage_debug);
#define GST_CAT_DEFAULT gst_cpu_usage_debug

struct _GstCPUUsageTracer
{
  GstPeriodicTracer parent;
  GstCPUUsage cpu_usage;
};

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_cpu_usage_debug, "cpuusage", 0, "cpuusage tracer");

G_DEFINE_TYPE_WITH_CODE (GstCPUUsageTracer, gst_cpu_usage_tracer,
    GST_TYPE_PERIODIC_TRACER, _do_init);

static GstTracerRecord *tr_cpuusage;

static void cpuusage_dummy_bin_add_post (GObject * obj, GstClockTime ts,
    GstBin * bin, GstElement * element, gboolean result);
static gboolean cpu_usage_thread_func (GstPeriodicTracer * tracer);
static void create_metadata_event (GstPeriodicTracer * tracer);
static void reset_counters (GstPeriodicTracer * tracer);

static void
cpuusage_dummy_bin_add_post (GObject * obj, GstClockTime ts,
    GstBin * bin, GstElement * element, gboolean result)
{
  return;
}

static void
reset_counters (GstPeriodicTracer * tracer)
{
  GstCPUUsageTracer *self;

  self = GST_CPU_USAGE_TRACER (tracer);

  gst_cpu_usage_init (&(self->cpu_usage));
}

static gboolean
cpu_usage_thread_func (GstPeriodicTracer * tracer)
{
  GstCPUUsageTracer *self;
  GstCPUUsage *cpu_usage;
  gfloat *cpu_load;
  gint cpu_id;
  gint cpu_load_len;

  self = GST_CPU_USAGE_TRACER (tracer);

  cpu_usage = &self->cpu_usage;

  cpu_load = CPU_USAGE_ARRAY (cpu_usage);
  cpu_load_len = CPU_USAGE_ARRAY_LENGTH (cpu_usage);

  gst_cpu_usage_compute (cpu_usage);

  for (cpu_id = 0; cpu_id < cpu_load_len; ++cpu_id) {
    gst_tracer_record_log (tr_cpuusage, cpu_id, cpu_load[cpu_id]);
  }

  return TRUE;
}

static void
create_metadata_event (GstPeriodicTracer * tracer)
{

}

static void
gst_cpu_usage_tracer_class_init (GstCPUUsageTracerClass * klass)
{
  GstPeriodicTracerClass *tracer_class;

  tracer_class = GST_PERIODIC_TRACER_CLASS (klass);

  tracer_class->timer_callback = GST_DEBUG_FUNCPTR (cpu_usage_thread_func);
  tracer_class->reset = GST_DEBUG_FUNCPTR (reset_counters);
  tracer_class->write_header = GST_DEBUG_FUNCPTR (create_metadata_event);

  tr_cpuusage = gst_tracer_record_new ("cpuusage.class",
      "number", GST_TYPE_STRUCTURE, gst_structure_new ("value",
          "type", G_TYPE_GTYPE, G_TYPE_UINT,
          "description", G_TYPE_STRING, "Core number",
          "flags", GST_TYPE_TRACER_VALUE_FLAGS,
          GST_TRACER_VALUE_FLAGS_AGGREGATED, "min", G_TYPE_UINT, 0, "max",
          G_TYPE_UINT, CPU_NUM_MAX, NULL), "load", GST_TYPE_STRUCTURE,
      gst_structure_new ("value", "type", G_TYPE_GTYPE, G_TYPE_DOUBLE,
          "description", G_TYPE_STRING, "Core load percentage [%]", "flags",
          GST_TYPE_TRACER_VALUE_FLAGS, GST_TRACER_VALUE_FLAGS_AGGREGATED, "min",
          G_TYPE_DOUBLE, 0.0f, "max", G_TYPE_DOUBLE, 100.0f, NULL), NULL);
}

static void
gst_cpu_usage_tracer_init (GstCPUUsageTracer * self)
{
  GstCPUUsage *cpu_usage;

  cpu_usage = &self->cpu_usage;
  gst_cpu_usage_init (cpu_usage);
  cpu_usage->cpu_array_sel = FALSE;

  /* Register a dummy hook so that the tracer remains alive */
  gst_tracing_register_hook (GST_TRACER (self), "bin-add-post",
      G_CALLBACK (cpuusage_dummy_bin_add_post));
}
