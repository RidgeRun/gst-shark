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

#include "gstctf.h"

#include "gstctfengine.h"

static GstCtfEngine *_ctf = NULL;
static GMutex _ctf_mutex;
static const gchar *_ctf_location = NULL;

GST_DEBUG_CATEGORY (gst_ctf_debug);
#define GST_CAT_DEFAULT gst_ctf_debug

static void gst_ctf_load_env (void);
static GstCtfRecord *gst_ctf_register_event_valist (const gchar * name,
    const gchar * firstfield, va_list var_args);

/* Should be called locked */
static void
gst_ctf_load_env (void)
{
  _ctf_location = g_getenv (GST_CTF_LOCATION);
  GST_INFO ("Setting CTF location to \"%s\"", _ctf_location);
}

void
gst_ctf_init (void)
{
  g_mutex_lock (&_ctf_mutex);

  if (NULL == gst_ctf_debug) {
    GST_DEBUG_CATEGORY_INIT (gst_ctf_debug, "ctf", 0, "ctf debug");
  }

  /* We already have an engine, ref it */
  if (NULL != _ctf) {
    GST_INFO ("Grabbed a reference to the CTF engine");
    gst_object_ref (_ctf);
    goto out;
  }

  gst_ctf_load_env ();

  /* Shortcircuit if user doesn't want to write CTF */
  if (NULL == _ctf_location) {
    GST_INFO ("No CTF requested");
    goto out;
  }

  if (NULL == _ctf) {
    _ctf = g_object_new (GST_TYPE_CTF_ENGINE, NULL);
  }

  if (FALSE == gst_ctf_engine_start (_ctf, _ctf_location)) {
    GST_ERROR ("Unable to start CTF engine");
    gst_clear_object (&_ctf);
    goto out;
  }

  GST_INFO ("Initialized CTF");

out:
  g_mutex_unlock (&_ctf_mutex);
}

void
gst_ctf_deinit (void)
{
  g_mutex_lock (&_ctf_mutex);
  if (NULL != _ctf) {
    GST_INFO ("Released CTF engine reference");
    gst_object_unref (_ctf);
  }
  g_mutex_unlock (&_ctf_mutex);
}

static GstCtfRecord *
gst_ctf_register_event_valist (const gchar * name,
    const gchar * firstfield, va_list var_args)
{
  GstCtfRecord *record = NULL;

  g_return_val_if_fail (name, NULL);
  g_return_val_if_fail (firstfield, NULL);

  g_mutex_lock (&_ctf_mutex);
  if (NULL == _ctf) {
    GST_DEBUG ("CTF engine not initialized, not logging traces");
  } else {
    record = gst_ctf_engine_register_event_valist (_ctf, name, firstfield,
        var_args);
  }
  g_mutex_unlock (&_ctf_mutex);

  return record;

}

GstCtfRecord *
gst_ctf_register_event (const gchar * name, const gchar * firstfield, ...)
{
  GstCtfRecord *record = NULL;
  va_list var_args;

  g_return_val_if_fail (name, NULL);
  g_return_val_if_fail (firstfield, NULL);

  va_start (var_args, firstfield);
  record = gst_ctf_register_event_valist (name, firstfield, var_args);
  va_end (var_args);

  return record;
}
