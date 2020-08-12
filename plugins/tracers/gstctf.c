/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016-2020 RidgeRun Engineering <support@ridgerun.com>
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
#include "config.h"
#endif

#include "gstctf.h"

#include <babeltrace2/babeltrace.h>

static bt_message_iterator_class_next_method_status
gst_ctf_iterator_next (bt_self_message_iterator * self_message_iterator,
    bt_message_array_const messages, uint64_t capacity, uint64_t * count)
{
  bt_message_iterator_class_next_method_status status =
      BT_MESSAGE_ITERATOR_CLASS_NEXT_METHOD_STATUS_OK;


  return status;
}

/* Define plugin and source component */
BT_PLUGIN_MODULE ();
BT_PLUGIN (gst);
BT_PLUGIN_SOURCE_COMPONENT_CLASS (tracers, gst_ctf_iterator_next);

/* Plugin metadata */
BT_PLUGIN_AUTHOR ("RidgeRun <support@ridgerun.com>");
BT_PLUGIN_DESCRIPTION ("GStreamer tracing subsystem");
BT_PLUGIN_LICENSE (GST_SHARK_LICENSE);
BT_PLUGIN_VERSION (GST_VERSION_MAJOR, GST_VERSION_MINOR,
    GST_VERSION_MICRO, GST_VERSION_NANO);

/* Component metadata */
BT_PLUGIN_SOURCE_COMPONENT_CLASS_DESCRIPTION (tracers,
    "Connect to GStreamer tracers");
BT_PLUGIN_SOURCE_COMPONENT_CLASS_HELP (tracers,
    "See https://gstreamer.freedesktop.org/documentation/additional/"
    "design/tracing.html for more details");
