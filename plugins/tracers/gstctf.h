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

#ifndef __GST_CTF_H__
#define __GST_CTF_H__

#include <gst/gst.h>

G_BEGIN_DECLS

GST_DEBUG_CATEGORY_EXTERN (gst_ctf_debug);

void gst_ctf_init (void);
void gst_ctf_deinit (void);

G_END_DECLS

#endif /*__GST_CTF_H__*/
