/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016-2018 RidgeRun Engineering <sebastian.fatjo@ridgerun.com>
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

#include "gstsharktracer.h"

GST_DEBUG_CATEGORY_STATIC (gst_shark_debug);
#define GST_CAT_DEFAULT gst_shark_debug

typedef struct _GstSharkTracerPrivate GstSharkTracerPrivate;
struct _GstSharkTracerPrivate
{

};

G_DEFINE_TYPE_WITH_PRIVATE (GstSharkTracer, gst_shark_tracer, GST_TYPE_TRACER);

static void
gst_shark_tracer_class_init (GstSharkTracerClass * klass)
{
  GST_DEBUG_CATEGORY_INIT (gst_shark_debug, "shark", 0, "base shark tracer");
}

static void
gst_shark_tracer_init (GstSharkTracer * self)
{
}
