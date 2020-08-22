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

#include "gstctfengine.h"

#include <babeltrace2/babeltrace.h>

#include "gstctfcomponent.h"

GST_DEBUG_CATEGORY_EXTERN (gst_ctf_debug);
#define GST_CAT_DEFAULT gst_ctf_debug

struct _GstCtfEngine
{
  GstObject base;

  bt_graph *graph;
  GstCtfComponent *component;
  gchar *path;
};

G_DEFINE_TYPE (GstCtfEngine, gst_ctf_engine, GST_TYPE_OBJECT);

/* GObject */
static void gst_ctf_engine_finalize (GObject * self);

static const bt_port_input *gst_ctf_engine_create_sink_component (GstCtfEngine *
    self, bt_graph * graph, const gchar * plugin_name,
    const gchar * component_name, const gchar * path);
static const bt_port_output
    * gst_ctf_engine_create_source_component (GstCtfEngine * self,
    bt_graph * graph, const gchar * plugin_name, const gchar * component_name);
static void gst_ctf_engine_log_component (GstCtfEngine * self,
    const bt_plugin * plugin, const bt_component_class * klass);

static void
gst_ctf_engine_init (GstCtfEngine * self)
{
  self->graph = NULL;
  self->component = NULL;
  self->path = NULL;
}

static void
gst_ctf_engine_class_init (GstCtfEngineClass * klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  oclass->finalize = gst_ctf_engine_finalize;
}

static void
gst_ctf_engine_log_component (GstCtfEngine * self, const bt_plugin * plugin,
    const bt_component_class * klass)
{
  guint maj = 0;
  guint min = 0;
  guint mic = 0;
  const gchar *extra = NULL;

  g_return_if_fail (self);
  g_return_if_fail (plugin);
  g_return_if_fail (klass);

  bt_plugin_get_version (plugin, &maj, &min, &mic, &extra);
  GST_INFO_OBJECT (self,
      "Found plugin:\n\tname: %s\n\tdescription: %s\n\tauthor: "
      "%s\n\tlicense: %s\n\tpath: %s\n\tversion: %d.%d.%d%s",
      bt_plugin_get_name (plugin),
      bt_plugin_get_description (plugin), bt_plugin_get_author (plugin),
      bt_plugin_get_license (plugin), bt_plugin_get_path (plugin), maj, min,
      mic, extra);

  GST_INFO_OBJECT (self,
      "Found component class:\n\tname: %s\n\tdescription: %s\n\thelp: %s",
      bt_component_class_get_name (klass),
      bt_component_class_get_description (klass),
      bt_component_class_get_help (klass));
}

static const bt_port_output *
gst_ctf_engine_create_source_component (GstCtfEngine * self, bt_graph * graph,
    const gchar * plugin_name, const gchar * component_name)
{
  const bt_port_output *port = NULL;
  const bt_plugin_set *plugins = NULL;
  const bt_plugin *plugin = NULL;
  const bt_value *params = NULL;
  const bt_component_source *comp = NULL;
  const bt_component_class_source *klass = NULL;
  const bt_component_class *cklass = NULL;
  gint ret = BT_PLUGIN_FIND_ALL_FROM_FILE_STATUS_OK;
  gint index = 0;
  bt_bool fail_on_error = TRUE;
  bt_logging_level level = BT_LOGGING_LEVEL_TRACE;

  g_return_val_if_fail (self, NULL);
  g_return_val_if_fail (graph, NULL);
  g_return_val_if_fail (plugin_name, NULL);
  g_return_val_if_fail (component_name, NULL);

  ret =
      bt_plugin_find_all_from_file (PLUGINDIR "/libgstsharktracers.so",
      fail_on_error, &plugins);
  if (BT_PLUGIN_FIND_ALL_FROM_FILE_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self, "Unable to find Babeltrace \"%s\" plug-in: %d",
        plugin_name, ret);
    goto out;
  }
  g_return_val_if_fail (plugins, FALSE);


  if (bt_plugin_set_get_plugin_count (plugins) < 1) {
    GST_ERROR_OBJECT (self, "No valid plug-in found in the set");
    goto free_plug;
  }

  plugin = bt_plugin_set_borrow_plugin_by_index_const (plugins, index);
  g_return_val_if_fail (plugin, FALSE);

  klass = bt_plugin_borrow_source_component_class_by_name_const (plugin,
      component_name);
  if (NULL == klass) {
    GST_ERROR_OBJECT (self,
        "No babeltrace component class named \"%s\" in \"%s\" plug-in",
        component_name, plugin_name);
    goto free_plug;
  }

  ret =
      bt_graph_add_source_component_with_initialize_method_data (graph, klass,
      component_name, params, &(self->component), level, &comp);
  if (BT_GRAPH_ADD_COMPONENT_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self, "Unable to create Babeltrace \"%s\" component: %d",
        component_name, ret);
    goto free_plug;
  }
  g_return_val_if_fail (comp, NULL);

  cklass = bt_component_class_source_as_component_class_const (klass);
  gst_ctf_engine_log_component (self, plugin, cklass);

  port = bt_component_source_borrow_output_port_by_index_const (comp, index);
  g_return_val_if_fail (port, NULL);

free_plug:
  bt_plugin_set_put_ref (plugins);

out:
  return port;
}

static const bt_port_input *
gst_ctf_engine_create_sink_component (GstCtfEngine * self, bt_graph * graph,
    const gchar * plugin_name, const gchar * component_name, const gchar * path)
{
  const bt_port_input *port = NULL;
  const bt_plugin *plugin = NULL;
  const bt_component_class_sink *klass = NULL;
  const bt_component_class *cklass = NULL;
  const bt_component_sink *comp = NULL;
  const gchar *param_key = "path";
  bt_value *params = NULL;
  bt_bool fail_on_error = TRUE;
  bt_bool find_in_std_env_var = TRUE;
  bt_bool find_in_user_dir = TRUE;
  bt_bool find_in_sys_dir = TRUE;
  bt_bool find_in_static = FALSE;
  gint ret = BT_PLUGIN_FIND_STATUS_OK;
  bt_logging_level level = BT_LOGGING_LEVEL_TRACE;
  gint index = 0;

  g_return_val_if_fail (self, NULL);
  g_return_val_if_fail (graph, NULL);
  g_return_val_if_fail (plugin_name, NULL);
  g_return_val_if_fail (component_name, NULL);

  ret = bt_plugin_find (plugin_name, find_in_std_env_var,
      find_in_user_dir, find_in_sys_dir, find_in_static,
      fail_on_error, &plugin);
  if (BT_PLUGIN_FIND_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self, "Unable to find Babeltrace \"%s\": %d", plugin_name,
        ret);
    goto out;
  }

  klass = bt_plugin_borrow_sink_component_class_by_name_const (plugin,
      component_name);
  if (NULL == klass) {
    GST_ERROR_OBJECT (self, "No babeltrace component class named \"%s\" in "
        "\"%s\" plug-in", component_name, plugin_name);
    goto free_plug;
  }

  params = bt_value_map_create ();
  if (NULL == params) {
    GST_ERROR_OBJECT (self, "Unable to create parameter for sink component");
    goto free_plug;
  }

  ret = bt_value_map_insert_string_entry (params, param_key, path);
  if (BT_VALUE_MAP_INSERT_ENTRY_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self, "Unable to create parameter for sink component");
    goto free_param;
  }

  g_free (self->path);
  self->path = g_strdup (path);

  ret =
      bt_graph_add_sink_component (graph, klass,
      component_name, params, level, &comp);
  if (BT_GRAPH_ADD_COMPONENT_STATUS_OK != ret) {
    GST_ERROR_OBJECT (self, "Unable to create Babeltrace component: %d", ret);
    goto free_param;
  }
  g_return_val_if_fail (comp, NULL);

  cklass = bt_component_class_sink_as_component_class_const (klass);
  gst_ctf_engine_log_component (self, plugin, cklass);

  port = bt_component_sink_borrow_input_port_by_index_const (comp, index);
  g_return_val_if_fail (port, NULL);

free_param:
  bt_value_put_ref (params);

free_plug:
  bt_plugin_put_ref (plugin);

out:
  return port;
}

gboolean
gst_ctf_engine_start (GstCtfEngine * self, const gchar * path)
{
  const guint64 mip_version = 0;
  const bt_port_input *inport = NULL;
  const bt_port_output *outport = NULL;
  bt_graph *graph = NULL;
  gboolean ret = FALSE;
  gint status = BT_GRAPH_CONNECT_PORTS_STATUS_OK;

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
   * 2. Create source component
   * 3. Create sink component
   * 4. Connect output port to input port
   */

  /* 1. Create babeltrace processing graph */
  graph = bt_graph_create (mip_version);
  if (NULL == graph) {
    GST_ERROR_OBJECT (self, "Unable to create graph");
    goto out;
  }

  bt_graph_get_ref (graph);
  self->graph = graph;

  /* 2. Create source component */
  outport =
      gst_ctf_engine_create_source_component (self, graph, GST_CTF_PLUGIN_NAME,
      GST_CTF_COMPONENT_NAME);
  if (NULL == outport) {
    goto freegraph;
  }

  /* 3. Create sink component */
  inport =
      gst_ctf_engine_create_sink_component (self, graph, "ctf", "fs", path);
  if (NULL == inport) {
    goto freegraph;
  }

  /* 4. Connect components by their ports */
  status = bt_graph_connect_ports (self->graph, outport, inport, NULL);
  if (BT_GRAPH_CONNECT_PORTS_STATUS_OK != status) {
    GST_ERROR_OBJECT (self, "Unable to connect ports");
    goto freegraph;
  }

  GST_INFO_OBJECT (self, "Successfully created Babeltrace graph");
  ret = TRUE;

freegraph:
  bt_graph_put_ref (graph);

out:
  return ret;
}

void
gst_ctf_engine_stop (GstCtfEngine * self)
{
  g_return_if_fail (self);

  bt_graph_put_ref (self->graph);
  self->graph = NULL;

  gst_clear_object (&self->component);

  g_free (self->path);
  self->path = NULL;
}

static void
gst_ctf_engine_finalize (GObject * object)
{
  GstCtfEngine *self = GST_CTF_ENGINE (object);

  gst_ctf_engine_stop (self);

  return G_OBJECT_CLASS (gst_ctf_engine_parent_class)->finalize (object);
}
