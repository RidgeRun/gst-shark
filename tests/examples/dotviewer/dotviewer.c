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

#include <stdlib.h>

#include "gstdot.h"

int
main (int argc, char *argv[])
{
  GstElement *pipe;
  const gchar *dot_string;
  GstDotRender render;

  gst_init (&argc, &argv);

  pipe = gst_parse_launch ("fakesrc ! fakesink", NULL);
  render = gst_dot_x11_render;

  dot_string = gst_dot_pipeline_to_string (GST_PIPELINE (pipe));
  gst_dot_do_render (dot_string, render, NULL);

  g_object_unref (pipe);

  return EXIT_SUCCESS;
}
