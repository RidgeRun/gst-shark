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

GST_DEBUG_CATEGORY_STATIC (gst_ctf_debug);
#define GST_CAT_DEFAULT gst_ctf_debug

struct _GstCtf
{
  GstObject base;

  bt_graph *graph;
};

G_DEFINE_TYPE (GstCtf, gst_ctf, GST_TYPE_OBJECT);

/* GObject */
static void gst_ctf_class_finalize (GObject * self);

/* Babeltrace */
static bt_message_iterator_class_next_method_status
gst_ctf_iterator_next (bt_self_message_iterator * self_message_iterator,
    bt_message_array_const messages, guint64 capacity, guint64 * count);

static void
gst_ctf_init (GstCtf * self)
{
  self->graph = NULL;
}

static void
gst_ctf_class_init (GstCtfClass * klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->finalize = gst_ctf_class_finalize;

  GST_DEBUG_CATEGORY_INIT (gst_ctf_debug, "ctf", 0, "ctf debug");
}

gboolean
gst_ctf_start (GstCtf * self)
{
  const guint64 mip_version = 0;
  bt_graph *graph = NULL;
  gboolean ret = FALSE;
  bt_bool fail_on_error = BT_TRUE;
  bt_plugin_find_all_from_file_status plug_ret =
      BT_PLUGIN_FIND_ALL_FROM_FILE_STATUS_OK;
  bt_graph_add_component_status comp_ret = BT_GRAPH_ADD_COMPONENT_STATUS_OK;
  const bt_plugin_set *plugins = NULL;
  const bt_plugin *plugin = NULL;
  const guint64 index = 0;
  const gchar *name = "tracers";
  const bt_component_class_source *klass = NULL;
  const bt_component_class *bklass = NULL;
  const bt_value *params = NULL;
  const bt_logging_level level = BT_LOGGING_LEVEL_WARNING;
  const bt_component_source *comp = NULL;
  guint maj = 0, min = 0, mic = 0;
  const gchar *extra = NULL;

  g_return_val_if_fail (self, FALSE);

  /*
   * Build the babel trace processing graph:
   *
   * .-------------------.    .-------------.
   * | source.gst.tracer | -> | sink.ctf.fs |
   * '-------------------'    '-------------'
   * Get data from tracers     Convert to CTF
   *
   * The process is:
   * 1. Build graph
   * 2. Find plugin set from .so
   * 3. Get the first (and only) and only plugin from the set
   * 4. Print plugin info
   * 5. Find our "tracer" source component class
   * 6. Print component  class info
   * 7. Create component and add it to source.
   */

  /* 1. Create babeltrace processing graph */
  graph = bt_graph_create (mip_version);
  if (NULL == graph) {
    GST_ERROR_OBJECT (self, "Unable to create graph");
    goto out;
  }

  bt_graph_get_ref (graph);
  self->graph = graph;

  /* 2. Find plugin set from our library */
  plug_ret =
      bt_plugin_find_all_from_file (PLUGINDIR "/libgstsharktracers.so",
      fail_on_error, &plugins);
  if (BT_PLUGIN_FIND_ALL_FROM_FILE_STATUS_OK != plug_ret) {
    GST_ERROR_OBJECT (self, "Unable to find Babeltrace plug-in: %d", plug_ret);
    goto freegraph;
  }
  g_return_val_if_fail (plugins, FALSE);


  /* 3. Verify and grab the first and only babeltrace plug-in from the set */
  if (bt_plugin_set_get_plugin_count (plugins) < 1) {
    GST_ERROR_OBJECT (self, "No valid plug-in found in the set");
    goto freegraph;
  }

  plugin = bt_plugin_set_borrow_plugin_by_index_const (plugins, index);
  g_return_val_if_fail (plugin, FALSE);

  /* 4. Print plug-in info */
  bt_plugin_get_version (plugin, &maj, &min, &mic, &extra);
  GST_INFO_OBJECT (self,
      "Found plugin:\n\tname: %s\n\tdescription: %s\n\tauthor: %s\n\tlicense: %s\n\tpath:"
      "%s\n\tversion: %d.%d.%d%s", bt_plugin_get_name (plugin),
      bt_plugin_get_description (plugin), bt_plugin_get_author (plugin),
      bt_plugin_get_license (plugin), bt_plugin_get_path (plugin), maj, min,
      mic, extra);

  /* 5. Find our "tracer" source component class */
  klass = bt_plugin_borrow_source_component_class_by_name_const (plugin, name);
  if (NULL == klass) {
    GST_ERROR_OBJECT (self, "No babeltrace component class named \"%s\"", name);
    goto freegraph;
  }

  /* 6. Print component class info */
  bklass = bt_component_class_source_as_component_class_const (klass);
  GST_INFO_OBJECT (self,
      "Found component class:\n\tname: %s\n\tdescription: %s\n\thelp: %s",
      bt_component_class_get_name (bklass),
      bt_component_class_get_description (bklass),
      bt_component_class_get_help (bklass));

  /* 7. Create component and add it to source. */
  comp_ret =
      bt_graph_add_source_component_with_initialize_method_data (graph, klass,
      name, params, NULL, level, &comp);
  if (BT_GRAPH_ADD_COMPONENT_STATUS_OK != comp_ret) {
    GST_ERROR_OBJECT (self, "Unable to create Babeltrace component: %d",
        comp_ret);
    goto freeplug;
  }
  g_return_val_if_fail (comp, FALSE);

  GST_INFO_OBJECT (self, "Successfully created Babeltrace graph");
  ret = TRUE;

freeplug:
  bt_plugin_set_put_ref (plugins);

freegraph:
  bt_graph_put_ref (graph);

out:
  return ret;
}

void
gst_ctf_stop (GstCtf * self)
{
  g_return_if_fail (self);

  bt_graph_put_ref (self->graph);
  self->graph = NULL;
}

static void
gst_ctf_class_finalize (GObject * object)
{
  GstCtf *self = GST_CTF (object);

  gst_ctf_stop (self);

  return G_OBJECT_CLASS (gst_ctf_parent_class)->finalize (object);
}


/* Babeltrace plugin */
static bt_message_iterator_class_next_method_status
gst_ctf_iterator_next (bt_self_message_iterator * self_message_iterator,
    bt_message_array_const messages, guint64 capacity, guint64 * count)
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
