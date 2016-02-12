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
#endif /*  */

#include <gst/gst.h>
#include <gvc.h>
#include <cgraph.h>

#include "gstdot.h"

const gchar *
gst_dot_pipeline_to_string (const GstPipeline * pipe)
{
  GstBin *bin;
  GstDebugGraphDetails flags;
  g_return_val_if_fail (pipe, NULL);
  bin = GST_BIN (pipe);
  flags = GST_DEBUG_GRAPH_SHOW_ALL;
  return gst_debug_bin_to_dot_data (bin, flags);
}

gboolean
gst_dot_do_render (const gchar * dot_string, GstDotRender render, gpointer args)
{
  g_return_val_if_fail (dot_string, FALSE);
  g_return_val_if_fail (render, FALSE);
  return render (dot_string, args);
}

gboolean
gst_dot_x11_render (const gchar * dot_string, gpointer args)
{
  Agraph_t *G;
  GVC_t *gvc;
  g_return_val_if_fail (dot_string, FALSE);
  gvc = gvContext ();
  G = agmemread (dot_string);
  gvLayout (gvc, G, "dot");
  gvRender (gvc, G, "x11", NULL);
  gvFreeLayout (gvc, G);
  agclose (G);
  gvFreeContext (gvc);
  return TRUE;
}
