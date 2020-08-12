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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gst/gst.h>
#include <glib/gstdio.h>
#include <unistd.h>
#include "gstgraphic.h"
#include "gstcpuusage.h"
#include "gstproctime.h"
#include "gstinterlatency.h"
#include "gstscheduletime.h"
#include "gstframerate.h"
#include "gstqueuelevel.h"
#include "gstbitrate.h"
#include "gstbuffer.h"
#include "gstctf.h"

static gboolean
plugin_init (GstPlugin * plugin)
{
#ifdef GST_CPUUSAGE_ENABLE
  if (!gst_tracer_register (plugin, "cpuusage",
          gst_cpu_usage_tracer_get_type ())) {
    return FALSE;
  }
#endif
  if (!gst_tracer_register (plugin, "graphic", gst_graphic_tracer_get_type ())) {
    return FALSE;
  }
  if (!gst_tracer_register (plugin, "proctime",
          gst_proc_time_tracer_get_type ())) {
    return FALSE;
  }
  if (!gst_tracer_register (plugin, "interlatency",
          gst_interlatency_tracer_get_type ())) {
    return FALSE;
  }
  if (!gst_tracer_register (plugin, "scheduletime",
          gst_scheduletime_tracer_get_type ())) {
    return FALSE;
  }
  if (!gst_tracer_register (plugin, "framerate",
          gst_framerate_tracer_get_type ())) {
    return FALSE;
  }
  if (!gst_tracer_register (plugin, "queuelevel",
          gst_queue_level_tracer_get_type ())) {
    return FALSE;
  }
  if (!gst_tracer_register (plugin, "bitrate", gst_bitrate_tracer_get_type ())) {
    return FALSE;
  }
  if (!gst_tracer_register (plugin, "buffer", gst_buffer_tracer_get_type ())) {
    return FALSE;
  }

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR,
    sharktracers, "GstShark tracers", plugin_init, VERSION,
    GST_SHARK_LICENSE, PACKAGE_NAME, PACKAGE_URL);
