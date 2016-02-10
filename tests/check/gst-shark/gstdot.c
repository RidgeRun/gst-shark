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
#include <gst/check/gstcheck.h>
#include <string.h>

#include "gstdot.h"

static gboolean
mock_render (const gchar * dot_string, gpointer user_data)
{
  const gchar *valid_dot = "digraph pipeline {";

  fail_if (strncmp (valid_dot, dot_string, 18));

  return TRUE;
}

GST_START_TEST (test_gst_dot)
{
  GstElement *pipe;
  GstDotRender render;
  const gchar *dot_string;
  GError *e = NULL;

  pipe = gst_parse_launch ("fakesrc ! fakesink", &e);
  fail_if (!pipe);
  fail_if (e);

  dot_string = gst_dot_pipeline_to_string (GST_PIPELINE (pipe));
  fail_if (!dot_string);

  render = mock_render;

  gst_dot_do_render (dot_string, render, NULL);
}

GST_END_TEST;

static Suite *
gst_dot_suite (void)
{
  Suite *s = suite_create ("GstDot");
  TCase *tc = tcase_create ("/tracers/graphics/dot");

  suite_add_tcase (s, tc);
  tcase_add_test (tc, test_gst_dot);

  return s;
}

GST_CHECK_MAIN (gst_dot)
