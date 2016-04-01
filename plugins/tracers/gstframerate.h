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

#ifndef __GST_FRAMERATE_TRACER_H__
#define __GST_FRAMERATE_TRACER_H__

#include <gst/gst.h>
#include <gst/gsttracer.h>

G_BEGIN_DECLS
#define GST_TYPE_FRAMERATE_TRACER \
  (gst_framerate_tracer_get_type())
#define GST_FRAMERATE_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_FRAMERATE_TRACER,GstFramerateTracer))
#define GST_FRAMERATE_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_FRAMERATE_TRACER,GstFramerateTracerClass))
#define GST_IS_FRAMERATE_TRACER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_FRAMERATE_TRACER))
#define GST_IS_FRAMERATE_TRACER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_FRAMERATE_TRACER))
#define GST_FRAMERATE_TRACER_CAST(obj) ((GstFramerateTracer *)(obj))
typedef struct _GstFramerateTracer GstFramerateTracer;
typedef struct _GstFramerateTracerClass GstFramerateTracerClass;

gboolean do_print_framerate (gpointer * data);

struct _GstFramerateTracer
{
  GstTracer parent;

  /*< private > */

  GHashTable *frame_counters;
  gboolean start_timer;
};

struct _GstFramerateTracerClass
{
  GstTracerClass parent_class;

  /* signals */
};

G_GNUC_INTERNAL GType gst_framerate_tracer_get_type (void);

G_END_DECLS
#endif /* __GST_FRAMERATE_TRACER_H__ */
