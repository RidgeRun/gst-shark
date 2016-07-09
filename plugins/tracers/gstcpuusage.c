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
#include <string.h>
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

#ifdef GST_STABLE_RELEASE
static GstTracerRecord *tr_cpuusage;
#endif

static const gchar cpuusage_metadata_event_header[] = "\
event {\n\
    name = cpuusage;\n\
    id = %d;\n\
    stream_id = %d;\n\
    fields := struct {\n";

static const gchar cpuusage_metadata_event_footer[] = "\
    };\n\
};\n\
\n";

static const gchar floating_point_event_field[] =
    "        floating_point { exp_dig = %lu; mant_dig = %d; byte_order = le; align = 8; } _cpu%d;\n";

static gboolean cpu_usage_thread_func (gpointer data);
static void cpuusage_dummy_bin_add_post (GObject * obj, GstClockTime ts,
    GstBin * bin, GstElement * element, gboolean result);

/* tracer class */

static void
gst_cpuusage_tracer_finalize (GObject * obj)
{
  GstCPUUsageTracer *self;

  self = GST_CPUUSAGE_TRACER (obj);

  g_source_remove (self->source_id);
  self->source_id = 0;

  G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
gst_cpuusage_tracer_class_init (GstCPUUsageTracerClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gst_cpuusage_tracer_finalize;
}

static void
cpuusage_dummy_bin_add_post (GObject * obj, GstClockTime ts,
    GstBin * bin, GstElement * element, gboolean result)
{
  return;
}

static gboolean
cpu_usage_thread_func (gpointer data)
{
  GstCPUUsageTracer *self;
  GstCPUUsage *cpu_usage;
  gfloat *cpu_load;
  gint cpu_id;
  gint cpu_load_len;

#ifdef EVAL
  static gint sec_counter = 0;
#endif

  self = GST_CPUUSAGE_TRACER (data);

  cpu_usage = &self->cpu_usage;

  cpu_load = CPU_USAGE_ARRAY (cpu_usage);
  cpu_load_len = CPU_USAGE_ARRAY_LENGTH (cpu_usage);

  gst_cpu_usage_compute (cpu_usage);

  for (cpu_id = 0; cpu_id < cpu_load_len; ++cpu_id) {
#ifdef GST_STABLE_RELEASE
    gst_tracer_record_log (tr_cpuusage, cpu_id, cpu_load[cpu_id]);
#else
    gst_tracer_log_trace (gst_structure_new ("cpuusage",
            "number", G_TYPE_UINT, cpu_id,
            "load", G_TYPE_DOUBLE, cpu_load[cpu_id], NULL));
#endif
  }
  do_print_cpuusage_event (CPUUSAGE_EVENT_ID, cpu_load_len, cpu_load);

#ifdef EVAL
  sec_counter++;
  if (sec_counter > EVAL_TIME) {
    return FALSE;
  }
#endif

  return TRUE;
}

static void
create_metadata_event (gint cpu_num)
{
  gchar *mem;
  gchar *mem_start;
  gchar *event_header;
  gsize str_size;
  gsize mem_size;
  gint msg_id;
  gint number_of_bytes;

  event_header =
      g_strdup_printf (cpuusage_metadata_event_header, CPUUSAGE_EVENT_ID, 0);

  str_size = strlen (event_header);

  /* Compute event description size
   * size = header + fields + footer
   */
  mem_size =
      str_size + cpu_num * sizeof (floating_point_event_field) +
      sizeof (cpuusage_metadata_event_footer);
  mem_start = g_malloc (mem_size);
  mem = mem_start;

  /* Add event header */
  mem += g_strlcpy (mem, event_header, mem_size);
  mem_size -= str_size;

  /* Add event fields */
  for (msg_id = 0; msg_id < cpu_num; ++msg_id) {
    /* floating point field definition:
     * http://diamon.org/ctf/#spec4.1.7
     */
    number_of_bytes = g_snprintf (mem,
        mem_size,
        floating_point_event_field,
        (unsigned long) (sizeof (gfloat) * CHAR_BIT - FLT_MANT_DIG),
        FLT_MANT_DIG, msg_id);

    mem += number_of_bytes;
    mem_size -= number_of_bytes;
  }

  /* Add event footer */
  g_strlcpy (mem, cpuusage_metadata_event_footer, mem_size);

  /* Add event in metadata file */
  add_metadata_event_struct (mem_start);
  /* Free allocated memory */
  g_free (mem_start);
  g_free (event_header);
}

static void
gst_cpuusage_tracer_init (GstCPUUsageTracer * self)
{
  GstCPUUsage *cpu_usage;

  cpu_usage = &self->cpu_usage;
  gst_cpu_usage_init (cpu_usage);
  cpu_usage->cpu_array_sel = FALSE;

#ifdef GST_STABLE_RELEASE
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
#else
  gst_tracer_log_trace (gst_structure_new ("cpuusage.class",
          "number", GST_TYPE_STRUCTURE, gst_structure_new ("value",
              "type", G_TYPE_GTYPE, G_TYPE_UINT,
              "description", G_TYPE_STRING, "Core number",
              "flags", G_TYPE_STRING, "aggregated",
              "min", G_TYPE_UINT, G_GUINT64_CONSTANT (0),
              "max", G_TYPE_UINT, CPU_NUM_MAX, NULL),
          "load", GST_TYPE_STRUCTURE, gst_structure_new ("value",
              "type", G_TYPE_GTYPE, G_TYPE_DOUBLE,
              "description", G_TYPE_STRING, "Core load percentage [%]",
              "flags", G_TYPE_STRING, "aggregated",
              "min", G_TYPE_DOUBLE, G_GUINT64_CONSTANT (0),
              "max", G_TYPE_DOUBLE, G_GUINT64_CONSTANT (100), NULL), NULL));
#endif

  create_metadata_event (CPU_USAGE_ARRAY_LENGTH (cpu_usage));

  /* Register a dummy hook so that the tracer remains alive */
  gst_tracing_register_hook (GST_TRACER (self), "bin-add-post",
      G_CALLBACK (cpuusage_dummy_bin_add_post));

  /* Create new thread to compute the cpu usage periodically */
  self->source_id =
      g_timeout_add_seconds (1, cpu_usage_thread_func, (gpointer) self);
}
