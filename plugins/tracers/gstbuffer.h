/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016-2017 RidgeRun Engineering <carlos.rodriguez@ridgerun.com>
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

#ifndef __GST_BUFFER_TRACER_H__
#define __GST_BUFFER_TRACER_H__

#include "gstsharktracer.h"

G_BEGIN_DECLS
#define GST_TYPE_BUFFER_TRACER \
  (gst_buffer_tracer_get_type())
#define GST_BUFFER_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_BUFFER_TRACER,GstBufferTracer))
#define GST_BUFFER_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_BUFFER_TRACER,GstBufferTracerClass))
#define GST_IS_BUFFER_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_BUFFER_TRACER))
#define GST_IS_BUFFER_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_BUFFER_TRACER))
#define GST_BUFFER_TRACER_CAST(obj) ((GstBufferTracer *)(obj))
typedef struct _GstSchedulePad GstSchedulePad;
typedef struct _GstBufferTracer GstBufferTracer;
typedef struct _GstBufferTracerClass GstBufferTracerClass;

/**
 * GstBufferTracer:
 *
 * Opaque #GstBufferTracer data structure
 */
struct _GstBufferTracer
{
  GstSharkTracer parent;
};

struct _GstBufferTracerClass
{
  GstSharkTracerClass parent_class;
};

G_GNUC_INTERNAL GType gst_buffer_tracer_get_type (void);

G_END_DECLS
#endif /* __GST_BUFFER_TRACER_H__ */
