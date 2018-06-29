/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016-2018 RidgeRun Engineering <sebastian.fatjo@ridgerun.com>
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

#include "gstsharktracer.h"

GST_DEBUG_CATEGORY_STATIC (gst_shark_debug);
#define GST_CAT_DEFAULT gst_shark_debug

#define GST_SHARK_TRACER_PRIVATE(o) \
  G_TYPE_INSTANCE_GET_PRIVATE((o), GST_SHARK_TYPE_TRACER, GstSharkTracerPrivate)

typedef struct _GstSharkTracerPrivate GstSharkTracerPrivate;
struct _GstSharkTracerPrivate
{
  GHashTable *params;
};

static void gst_shark_tracer_constructed (GObject * object);
static void gst_shark_tracer_finalize (GObject * object);
static void gst_shark_tracer_save_params (GstSharkTracer * self);
static void gst_shark_tracer_dump_params (GstSharkTracer * self);
static void gst_shark_tracer_free_params (GstSharkTracerPrivate * priv);

G_DEFINE_TYPE_WITH_PRIVATE (GstSharkTracer, gst_shark_tracer, GST_TYPE_TRACER);

static void
gst_shark_tracer_class_init (GstSharkTracerClass * klass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  GST_DEBUG_CATEGORY_INIT (gst_shark_debug, "sharktracer", 0,
      "base shark tracer");

  oclass->constructed = gst_shark_tracer_constructed;
  oclass->finalize = gst_shark_tracer_finalize;
}

static void
gst_shark_tracer_init (GstSharkTracer * self)
{
  GstSharkTracerPrivate *priv = GST_SHARK_TRACER_PRIVATE (self);

  priv->params = g_hash_table_new (g_str_hash, g_str_equal);
}

static void
gst_shark_tracer_constructed (GObject * object)
{
  GstSharkTracer *self = GST_SHARK_TRACER (object);

  gst_shark_tracer_save_params (self);
}

static void
gst_shark_tracer_free_params (GstSharkTracerPrivate * priv)
{
  GHashTableIter iter;
  gpointer key, value;

  g_hash_table_iter_init (&iter, priv->params);
  while (g_hash_table_iter_next (&iter, &key, &value)) {
    g_free (key);
    g_list_free_full (value, g_free);
  }
}

static void
gst_shark_tracer_finalize (GObject * object)
{
  GstSharkTracerPrivate *priv = GST_SHARK_TRACER_PRIVATE (object);

  gst_shark_tracer_free_params (priv);
  g_hash_table_unref (priv->params);

  G_OBJECT_CLASS (gst_shark_tracer_parent_class)->finalize (object);
}

static void
gst_shark_tracer_save_params (GstSharkTracer * self)
{
  GstSharkTracerPrivate *priv = GST_SHARK_TRACER_PRIVATE (self);
  gchar **tokens;
  gchar *params;
  gint i;

  g_object_get (self, "params", &params, NULL);

  if (NULL == params) {
    return;
  }

  tokens = g_strsplit_set (params, ",", 0);
  for (i = 0; tokens[i]; ++i) {
    GList *param;
    gchar **keyvalue;

    keyvalue = g_strsplit_set (tokens[i], "=", 2);
    if (NULL == keyvalue[1]) {
      GST_ERROR_OBJECT (self, "Malformed parameter %s", tokens[i]);
      goto cont;
    }

    GST_DEBUG_OBJECT (self, "Found param %s with value %s", keyvalue[0],
        keyvalue[1]);

    /* If there is no param with the given key yet, the lookup will
       return NULL. NULL is a valid value for prepend, in which case a
       new list will be started.
     */
    param = g_hash_table_lookup (priv->params, keyvalue[0]);
    param = g_list_append (param, g_strdup (keyvalue[1]));
    g_hash_table_replace (priv->params, g_strdup (keyvalue[0]), param);

    GST_INFO_OBJECT (self, "Appended %s to %s", keyvalue[1], keyvalue[0]);

  cont:
    g_strfreev (keyvalue);
  }
  g_strfreev (tokens);


  if (i > 0) {
    gst_shark_tracer_dump_params (self);
  }
}

static void
gst_shark_tracer_dump_params (GstSharkTracer * self)
{
  GstSharkTracerPrivate *priv = GST_SHARK_TRACER_PRIVATE (self);
  GList *keys;
  GList *key;

  GST_INFO_OBJECT (self, "Dumping %d parameters:",
      g_hash_table_size (priv->params));

  keys = g_hash_table_get_keys (priv->params);

  for (key = keys; NULL != key; key = g_list_next (key)) {
    GList *values = g_hash_table_lookup (priv->params, key->data);
    GList *value;

    GST_INFO_OBJECT (self, "\t- %s (%d)", (gchar *) key->data,
        g_list_length (values));
    for (value = values; NULL != value; value = g_list_next (value)) {
      GST_INFO_OBJECT (self, "\t\t- %s", (gchar *) value->data);
    }
  }

  g_list_free (keys);
}
