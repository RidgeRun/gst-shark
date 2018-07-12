/* GStreamer
 * Copyright (C) 2013 Stefan Sauer <ensonic@users.sf.net>
 * Copyright (C) 2016 RidgeRun Engineering <carlos.rodriguez@ridgerun.com> 
 *
 * gstinterlatency.h: tracing module that logs processing latencies
 * stats between source and intermediate elements
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GST_INTERLATENCY_TRACER_H__
#define __GST_INTERLATENCY_TRACER_H__

#include <gst/gst.h>
#include <gst/gsttracer.h>

G_BEGIN_DECLS

#define GST_TYPE_INTERLATENCY_TRACER (gst_interlatency_tracer_get_type ())
G_DECLARE_FINAL_TYPE (GstInterlatencyTracer, gst_interlatency_tracer, GST, INTERLATENCY_TRACER, GstTracer)

G_END_DECLS

#endif /* __GST_INTERLATENCY_TRACER_H__ */
