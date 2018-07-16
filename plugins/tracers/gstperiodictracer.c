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

#include "gstperiodictracer.h"

GST_DEBUG_CATEGORY_STATIC (gst_periodic_debug);
#define GST_CAT_DEFAULT gst_periodic_debug

#define GST_PERIODIC_TRACER_PRIVATE(o) \
  G_TYPE_INSTANCE_GET_PRIVATE((o), GST_TYPE_PERIODIC_TRACER, GstPeriodicTracerPrivate)

typedef struct _GstPeriodicTracerPrivate GstPeriodicTracerPrivate;
struct _GstPeriodicTracerPrivate
{

};

G_DEFINE_TYPE_WITH_PRIVATE (GstPeriodicTracer, gst_periodic_tracer,
    GST_SHARK_TYPE_TRACER);

static void
gst_periodic_tracer_class_init (GstPeriodicTracerClass * klass)
{
  GST_DEBUG_CATEGORY_INIT (gst_periodic_debug, "periodictracer", 0,
      "base periodic tracer");

  klass->timer_callback = NULL;
  klass->reset = NULL;
}

static void
gst_periodic_tracer_init (GstPeriodicTracer * self)
{
  //GstPeriodicTracerPrivate *priv = GST_PERIODIC_TRACER_PRIVATE (self);
}
