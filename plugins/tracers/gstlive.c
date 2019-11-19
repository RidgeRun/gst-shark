/* GstShark - A Front End for GstTracer
 * Copyright (C) 2018 RidgeRun Engineering <michael.gruner@ridgerun.com>
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
 * SECTION:gstlive
 * @short_description: display pipeline live
 *
 * A tracing module that uses the DOT libraries in order to show the pipeline executed liveally
 */

#include "gstlive.h"
#include "gstdot.h"
#include "gstliveprofiler.h"

GST_DEBUG_CATEGORY_STATIC (gst_live_debug);
#define GST_CAT_DEFAULT gst_live_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_STATES);

struct _GstLiveTracer
{
  GstSharkTracer parent;
};

#define _do_init \
    GST_DEBUG_CATEGORY_INIT (gst_live_debug, "live", 0, "live tracer"); \
    GST_DEBUG_CATEGORY_GET (GST_CAT_STATES, "GST_STATES");

G_DEFINE_TYPE_WITH_CODE (GstLiveTracer, gst_live_tracer,
    GST_SHARK_TYPE_TRACER, _do_init);


static void 
do_element_new (GObject * self, GstClockTime ts, GstElement * element)
{
	update_new_element_event(GST_OBJECT_NAME (element), ts);
}

static void gst_live_tracer_finalize (GObject * obj);


/* tracer class */

static void
gst_live_tracer_finalize (GObject * obj)
{
  G_OBJECT_CLASS (gst_live_tracer_parent_class)->finalize (obj);
}

static void
gst_live_tracer_class_init (GstLiveTracerClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gst_live_tracer_finalize;
}

static void
gst_live_tracer_init (GstLiveTracer * self)
{
  GstTracer *tracer = GST_TRACER (self);
  gst_tracing_register_hook (tracer, "element-new",
      G_CALLBACK (do_element_new));
}
