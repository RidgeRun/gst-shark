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

#ifndef __GST_SCHEDULETIME_TRACER_H__
#define __GST_SCHEDULETIME_TRACER_H__

#include <gst/gst.h>
#include <gst/gsttracer.h>

G_BEGIN_DECLS
#define GST_TYPE_SCHEDULETIME_TRACER \
  (gst_scheduletime_tracer_get_type())
#define GST_SCHEDULETIME_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_SCHEDULETIME_TRACER,GstScheduletimeTracer))
#define GST_SCHEDULETIME_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_SCHEDULETIME_TRACER,GstScheduletimeTracerClass))
#define GST_IS_SCHEDULETIME_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_SCHEDULETIME_TRACER))
#define GST_IS_SCHEDULETIME_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_SCHEDULETIME_TRACER))
#define GST_SCHEDULETIME_TRACER_CAST(obj) ((GstScheduletimeTracer *)(obj))
typedef struct _GstSchedulePad GstSchedulePad;
typedef struct _GstScheduletimeTracer GstScheduletimeTracer;
typedef struct _GstScheduletimeTracerClass GstScheduletimeTracerClass;

/**
 * GstScheduletimeTracer:
 *
 * Opaque #GstScheduletimeTracer data structure
 */

struct _GstSchedulePad
{
  GstPad *pad;
  GstClockTime previous_time;
};

struct _GstScheduletimeTracer
{
  GstTracer parent;
  GHashTable *schedule_pads;
  GString *time_string;
};

struct _GstScheduletimeTracerClass
{
  GstTracerClass parent_class;
};

G_GNUC_INTERNAL GType gst_scheduletime_tracer_get_type (void);

G_END_DECLS
#endif /* __GST_SCHEDULETIME_TRACER_H__ */
