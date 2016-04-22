/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016 RidgeRun Engineering <manuel.leiva@ridgerun.com>
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

#include "gstproctimecompute.h"

static gboolean gst_proctime_element_is_async (GstProcTimeElement * element);

void
gst_proctime_init (GstProcTime * proc_time)
{
  g_return_if_fail (proc_time);
  proc_time->elem_num = 0;
  proc_time->element = NULL;
}

void
gst_proctime_finalize (GstProcTime * proc_time)
{
  g_return_if_fail (proc_time);
  g_free (proc_time->element);
}

/* Add a new element in the list.
 * The element added must be an element with only one src pad and one 
 * sink pad.
 */
static void
gst_proctime_add_in_list (GstProcTime * proc_time, gchar * name,
    GstPad * sink_pad, GstPad * src_pad)
{
  GstProcTimeElement *element;
  gint elem_idx;

  element = proc_time->element;
  /* Verify if is the first new element */
  if (NULL == element) {
    proc_time->elem_num++;
    element = g_malloc (sizeof (GstProcTimeElement));
    proc_time->element = element;
  } else {
    proc_time->elem_num++;
    element = g_realloc (element, proc_time->elem_num * sizeof (GstProcTime));
    proc_time->element = element;
  }
  elem_idx = proc_time->elem_num - 1;
  /* Store element information (sink and src pads) */
  element[elem_idx].name = name;
  element[elem_idx].sink_pad = sink_pad;
  element[elem_idx].src_pad = src_pad;
  element[elem_idx].sink_thread = NULL;
  element[elem_idx].src_thread = NULL;
}

void
gst_proctime_add_new_element (GstProcTime * proc_time, GstElement * element)
{
  GstIterator *iterator;
  gchar *name;
  GValue vpad = G_VALUE_INIT;
  gint num_src_pads = 0;
  gint num_sink_pads = 0;
  GstPad *src_pad = NULL;
  GstPad *sink_pad = NULL;

  g_return_if_fail (proc_time);
  g_return_if_fail (element);

  name = GST_OBJECT_NAME (element);

  iterator = gst_element_iterate_src_pads (element);
  while (gst_iterator_next (iterator, &vpad) == GST_ITERATOR_OK) {
    src_pad = GST_PAD (g_value_get_object (&vpad));
    num_src_pads++;
  }
  gst_iterator_free (iterator);

  iterator = gst_element_iterate_sink_pads (element);
  while (gst_iterator_next (iterator, &vpad) == GST_ITERATOR_OK) {
    sink_pad = GST_PAD (g_value_get_object (&vpad));
    num_sink_pads++;
  }
  gst_iterator_free (iterator);

  /* Verify if the element only have one input and output */
  if ((1 == num_src_pads) & (1 == num_sink_pads)) {
    gst_proctime_add_in_list (proc_time, name, sink_pad, src_pad);
  }
}

void
gst_proctime_proc_time (GstProcTime * proc_time, GstClockTime * time,
    gchar ** name, GstPad * peer_pad, GstPad * src_pad)
{
  GstProcTimeElement *element;
  GstClockTime stop_time;
  gint elem_idx;
  gint elem_num;

  g_return_if_fail (proc_time);
  g_return_if_fail (time);
  g_return_if_fail (name);
  g_return_if_fail (src_pad);
  g_return_if_fail (peer_pad);

  element = proc_time->element;
  elem_num = proc_time->elem_num;

  *name = NULL;
  /* Search the peer pad in the list 
   * The peer pad is used to identify which is the element where the 
   * buffer is received.
   */
  for (elem_idx = 0; elem_idx < elem_num; ++elem_idx) {
    if (element[elem_idx].sink_pad == peer_pad) {
      element[elem_idx].start_time = gst_util_get_timestamp ();
      element[elem_idx].sink_thread = g_thread_self ();
    }
  }

  /* Search the src pad in the list 
   * The peer pad is used to identify which is the element where the 
   * buffer was processed.
   * If the src pad is not in the list, then it is a src element and the
   * precessing time is not computed
   */
  for (elem_idx = 0; elem_idx < elem_num; ++elem_idx) {
    if (element[elem_idx].src_pad == src_pad) {
      stop_time = gst_util_get_timestamp ();
      *time = stop_time - element[elem_idx].start_time;
      element[elem_idx].src_thread = g_thread_self ();
      if (FALSE == gst_proctime_element_is_async (&element[elem_idx])) {
        *name = element[elem_idx].name;
      }
    }
  }
}

static gboolean
gst_proctime_element_is_async (GstProcTimeElement * element)
{
  /* If threads are not defined yet we can't tell if its async or not */
  if (NULL == element->sink_thread || NULL == element->src_thread) {
    return FALSE;
  }

  return element->sink_thread != element->src_thread;
}
