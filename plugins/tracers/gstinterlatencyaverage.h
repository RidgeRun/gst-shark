/* GStreamer
 * Copyright (C) 2013 Stefan Sauer <ensonic@users.sf.net>
 * Copyright (C) 2020 RidgeRun Engineering <carlos.rodriguez@ridgerun.com> 
 *
 * gstinterlatencyaverage.h: tracing module that logs processing latencies
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

#ifndef __GST_INTERLATENCYAVERAGE_TRACER_H__
#define __GST_INTERLATENCYAVERAGE_TRACER_H__

#include "gstperiodictracer.h"

G_BEGIN_DECLS

#define GST_TYPE_INTERLATENCYAVERAGE_TRACER	\
  (gst_interlatencyaverage_tracer_get_type())
G_DECLARE_FINAL_TYPE (GstInterlatencyAverageTracer, gst_interlatencyaverage_tracer, GST, INTERLATENCYAVERAGE_TRACER, GstPeriodicTracer)

G_END_DECLS
#endif /* __GST_INTERLATENCYAVERAGE_TRACER_H__ */
