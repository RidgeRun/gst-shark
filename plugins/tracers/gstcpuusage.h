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

#ifndef __GST_CPUUSAGE_TRACER_H__
#define __GST_CPUUSAGE_TRACER_H__

#include <gst/gst.h>
#include <gst/gsttracer.h>


G_BEGIN_DECLS

#define GST_TYPE_CPUUSAGE_TRACER \
  (gst_cpuusage_tracer_get_type())
#define GST_CPUUSAGE_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_CPUUSAGE_TRACER,GstCPUUsageTracer))
#define GST_CPUUSAGE_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_CPUUSAGE_TRACER,GstCPUUsageTracerClass))
#define GST_IS_CPUUSAGE_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_CPUUSAGE_TRACER))
#define GST_IS_CPUUSAGE_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_CPUUSAGE_TRACER))
#define GST_CPUUSAGE_TRACER_CAST(obj) ((GstCPUUsageTracer *)(obj))

#define CPU_NUM_MAX  8

typedef struct _GstCPUUsageTracer GstCPUUsageTracer;
typedef struct _GstCPUUsageTracerClass GstCPUUsageTracerClass;

typedef struct
{
  GstClockTime ts;
  GstClockTime val;
} GstTraceValue;

typedef struct
{
  GstClockTime window;
  GQueue values;                /* GstTraceValue* */
} GstTraceValues;


typedef struct
{
  /* CPU core number */
  gint32  cpu_num;
  gdouble cpu_usage[CPU_NUM_MAX];

  gint user    [CPU_NUM_MAX]; /* Time spent in user mode */
  gint user_aux[CPU_NUM_MAX]; /* Time spent in user mode */
  gint nice    [CPU_NUM_MAX]; /* Time spent in user mode with low priority */
  gint nice_aux[CPU_NUM_MAX]; /* Time spent in user mode with low priority */
  gint system    [CPU_NUM_MAX];/* Time spent in user mode with low priority */
  gint system_aux[CPU_NUM_MAX];/* Time spent in user mode with low priority */
  gint idle    [CPU_NUM_MAX]; /* Time spent in system mode */
  gint idle_aux[CPU_NUM_MAX]; /* Time spent in system mode */
  gboolean cpu_array_sel;
} GstCPUUsage;

/**
 * GstCPUUsageTracer:
 *
 * Opaque #GstCPUUsageTracer data structure
 */
struct _GstCPUUsageTracer {
  GstTracer  parent;

  /*< private >*/
  GHashTable *threads;
  GstTraceValues *tvs_proc;

  /* for ts calibration */
  gpointer main_thread_id;
  guint64 tproc_base;

  GstCPUUsage cpuusage;
};

struct _GstCPUUsageTracerClass {
  GstTracerClass parent_class;

  /* signals */
};

G_GNUC_INTERNAL GType gst_cpuusage_tracer_get_type (void);

G_END_DECLS

#endif /* __GST_CPUUSAGE_TRACER_H__ */
