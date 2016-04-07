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

typedef struct _GstDotRenderThread GstDotRenderThread;

struct _GstDotRenderThread
{
  GstDotRender render;
  const gchar *dot_string;
  gpointer args;
};

static void gst_dot_pipeline_to_file (GstBin * bin, GstDebugGraphDetails flags);

void
gst_dot_pipeline_to_file (GstBin * bin, GstDebugGraphDetails flags)
{
  const gchar *trace_dir;
  gchar *full_file_name = NULL;
  FILE *out;

  trace_dir = g_getenv ("GST_SHARK_TRACE_DIR");

  if (G_LIKELY (trace_dir == NULL))
    trace_dir = g_getenv ("PWD");

  full_file_name = g_strdup_printf ("%s" G_DIR_SEPARATOR_S "pipeline.dot",
      trace_dir);

  if ((out = fopen (full_file_name, "wb"))) {
    gchar *buf;

    buf = gst_debug_bin_to_dot_data (bin, flags);
    fputs (buf, out);

    g_free (buf);
    fclose (out);

    GST_INFO ("wrote bin graph to : '%s'", full_file_name);
  } else {
    GST_WARNING ("Failed to open file '%s' for writing: %s", full_file_name,
        g_strerror (errno));
  }

  g_free (full_file_name);
}

const gchar *
gst_dot_pipeline_to_string (const GstPipeline * pipe)
{
  GstBin *bin;
  GstDebugGraphDetails flags;

  g_return_val_if_fail (pipe, NULL);
  bin = GST_BIN (pipe);
  flags = GST_DEBUG_GRAPH_SHOW_ALL;

  gst_dot_pipeline_to_file (bin, flags);

  return gst_debug_bin_to_dot_data (bin, flags);
}

static gpointer
gst_dot_render_thread (gpointer data)
{
  GstDotRenderThread *thread_args;
  GstDotRender render;
  const gchar *dot_string;
  gpointer args;

  g_return_val_if_fail (data, NULL);
  thread_args = (GstDotRenderThread *) data;

  /* Breaking the structure passed as a parameter */
  render = thread_args->render;
  dot_string = thread_args->dot_string;
  args = thread_args->args;

  /* Calling the function to get the pipeline graphic */
  render (dot_string, args);

  /* Freeing the memory allocated and killing the graphic thread */
  g_free (thread_args);
  g_thread_exit (0);

  return NULL;
}

gboolean
gst_dot_do_render (const gchar * dot_string, GstDotRender render, gpointer args)
{
  GstDotRenderThread *thread_args;

  g_return_val_if_fail (dot_string, FALSE);
  g_return_val_if_fail (render, FALSE);

  /* Allocating memory with the parameters for the graphic pipeline */
  thread_args = g_malloc (sizeof (GstDotRenderThread));

  /* Filling the structure that is going to be used to get a new thread
     for displaying the pipeline graphic */
  thread_args->render = render;
  thread_args->dot_string = dot_string;
  thread_args->args = args;

  g_thread_new ("GstDotRender", gst_dot_render_thread, thread_args);

  return TRUE;
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
