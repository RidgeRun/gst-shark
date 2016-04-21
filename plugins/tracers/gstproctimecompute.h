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

#ifndef __GST_PROC_TIME_COMPUTE_H__
#define __GST_PROC_TIME_COMPUTE_H__

#include <gst/gst.h>

#include <sys/time.h>

G_BEGIN_DECLS typedef struct
{
  gchar *name;
  GstPad *srcPad;
  GThread *srcthread;
  GstPad *sinkPad;
  GThread *sinkthread;
  GstClockTime startTime;
} GstProcTimeElement;

typedef struct
{
  gint elem_num;
  GstProcTimeElement *element;
  GstClock clock;
} GstProcTime;

void gst_proctime_init (GstProcTime * procTime);

void gst_proctime_add_new_element (GstProcTime * procTime,
    GstElement * element);

void gst_proctime_proc_time (GstProcTime * procTime,
    GstClockTime * time, gchar ** name, GstPad * peerPad, GstPad * srcPad);

void gst_proctime_finalize (GstProcTime * procTime);

G_END_DECLS
#endif //__GST_PROC_CTIME_COMPUTE_H__
