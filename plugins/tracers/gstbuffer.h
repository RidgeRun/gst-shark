/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016-2018 RidgeRun Engineering <carlos.rodriguez@ridgerun.com>
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

#define GST_TYPE_BUFFER_TRACER (gst_buffer_tracer_get_type ())
G_DECLARE_FINAL_TYPE (GstBufferTracer, gst_buffer_tracer, GST, BUFFER_TRACER, GstSharkTracer)

G_END_DECLS
#endif /* __GST_BUFFER_TRACER_H__ */
