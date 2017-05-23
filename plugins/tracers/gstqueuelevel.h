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

#ifndef __GST_QUEUE_LEVEL_TRACER_H__
#define __GST_QUEUE_LEVEL_TRACER_H__

#include <gst/gst.h>
#include <gst/gsttracer.h>

G_BEGIN_DECLS
#define GST_TYPE_QUEUE_LEVEL_TRACER \
  (gst_queue_level_tracer_get_type())
#define GST_QUEUE_LEVEL_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_QUEUE_LEVEL_TRACER,GstQueueLevelTracer))
#define GST_QUEUE_LEVEL_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_QUEUE_LEVEL_TRACER,GstQueueLevelTracerClass))
#define GST_IS_QUEUE_LEVEL_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_QUEUE_LEVEL_TRACER))
#define GST_IS_QUEUE_LEVEL_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_QUEUE_LEVEL_TRACER))
#define GST_QUEUE_LEVEL_TRACER_CAST(obj) ((GstQueueLevelTracer *)(obj))
typedef struct _GstSchedulePad GstSchedulePad;
typedef struct _GstQueueLevelTracer GstQueueLevelTracer;
typedef struct _GstQueueLevelTracerClass GstQueueLevelTracerClass;

/**
 * GstQueueLevelTracer:
 *
 * Opaque #GstQueueLevelTracer data structure
 */
struct _GstQueueLevelTracer
{
  GstTracer parent;
};

struct _GstQueueLevelTracerClass
{
  GstTracerClass parent_class;
};

G_GNUC_INTERNAL GType gst_queue_level_tracer_get_type (void);

G_END_DECLS
#endif /* __GST_QUEUE_LEVEL_TRACER_H__ */
