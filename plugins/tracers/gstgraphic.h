/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016 RidgeRun Engineering <michael.gruner@ridgerun.com>
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

#ifndef __GST_GRAPHIC_TRACER_H__
#define __GST_GRAPHIC_TRACER_H__

#include "gstsharktracer.h"

G_BEGIN_DECLS
#define GST_TYPE_GRAPHIC_TRACER \
  (gst_graphic_tracer_get_type())
#define GST_GRAPHIC_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_GRAPHIC_TRACER,GstGraphicTracer))
#define GST_GRAPHIC_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_GRAPHIC_TRACER,GstGraphicTracerClass))
#define GST_IS_GRAPHIC_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_GRAPHIC_TRACER))
#define GST_IS_GRAPHIC_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_GRAPHIC_TRACER))
#define GST_GRAPHIC_TRACER_CAST(obj) ((GstGraphicTracer *)(obj))
typedef struct _GstGraphicTracer GstGraphicTracer;
typedef struct _GstGraphicTracerClass GstGraphicTracerClass;

struct _GstGraphicTracer
{
  GstSharkTracer parent;

  /*< private > */
};

struct _GstGraphicTracerClass
{
  GstSharkTracerClass parent_class;

  /* signals */
};

G_GNUC_INTERNAL GType gst_graphic_tracer_get_type (void);

G_END_DECLS
#endif /* __GST_GRAPHIC_TRACER_H__ */
