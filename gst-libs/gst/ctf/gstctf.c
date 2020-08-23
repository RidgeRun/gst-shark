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
const gchar *_gst_ctf_location = NULL;

GST_DEBUG_CATEGORY (gst_ctf_debug);
#define GST_CAT_DEFAULT gst_ctf_debug

static void
gst_ctf_load_env (void)
{
  _gst_ctf_location = g_getenv (GST_CTF_LOCATION);

  GST_INFO ("Setting CTF location to \"%s\"", _gst_ctf_location);
}

void
gst_ctf_init (void)
{
  if (NULL == gst_ctf_debug) {
    GST_DEBUG_CATEGORY_INIT (gst_ctf_debug, "ctf", 0, "ctf debug");
  }

  gst_ctf_load_env ();

  /* Shortcircuit if user doesn't want to write CTF */
  if (NULL == _gst_ctf_location) {
    GST_INFO ("No CTF requested");
    return;
  }

  if (NULL == _ctf) {
    _ctf = g_object_new (GST_TYPE_CTF_ENGINE, NULL);
  }

  if (FALSE == gst_ctf_engine_start (_ctf, _gst_ctf_location)) {
    GST_ERROR ("Unable to start CTF engine");
    gst_clear_object (&_ctf);
    return;
  }

  GST_INFO ("Initialized CTF");
}

void
gst_ctf_deinit (void)
{
  gst_clear_object (&_ctf);

  GST_INFO ("Deinitialized CTF");
}
