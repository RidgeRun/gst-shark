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
gst_proctime_init (GstProcTime * procTime)
{
  g_return_if_fail (procTime);
  procTime->elem_num = 0;
  procTime->element = NULL;
}

void
gst_proctime_finalize (GstProcTime * procTime)
{
  g_return_if_fail (procTime);
  g_free (procTime->element);
}

/* Add a new element in the list.
 * The element added must be an element with only one src pad and one 
 * sink pad.
 */
static void
gst_proctime_add_in_list (GstProcTime * procTime, gchar * name,
    GstPad * sinkpad, GstPad * srcpad)
{
  GstProcTimeElement *element;
  gint elem_idx;

  element = procTime->element;
  /* Verify if is the first new element */
  if (NULL == element) {
    procTime->elem_num++;
    element = g_malloc (sizeof (GstProcTimeElement));
    procTime->element = element;
  } else {
    procTime->elem_num++;
    element = g_realloc (element, procTime->elem_num * sizeof (GstProcTime));
    procTime->element = element;
  }
  elem_idx = procTime->elem_num - 1;
  /* Store element information (sink and src pads) */
  element[elem_idx].name = name;
  element[elem_idx].sinkPad = sinkpad;
  element[elem_idx].srcPad = srcpad;
  element[elem_idx].sinkthread = NULL;
  element[elem_idx].srcthread = NULL;
}

void
gst_proctime_add_new_element (GstProcTime * procTime, GstElement * element)
{
  GstIterator *iterator;
  gchar *name;
  GValue vPad = G_VALUE_INIT;
  gint srcPads = 0;
  gint sinkPads = 0;
  GstPad *srcpad = NULL;
  GstPad *sinkpad = NULL;

  g_return_if_fail (procTime);
  g_return_if_fail (element);

  name = GST_OBJECT_NAME (element);

  iterator = gst_element_iterate_src_pads (element);
  while (gst_iterator_next (iterator, &vPad) == GST_ITERATOR_OK) {
    srcpad = GST_PAD (g_value_get_object (&vPad));
    srcPads++;
  }
  gst_iterator_free (iterator);

  iterator = gst_element_iterate_sink_pads (element);
  while (gst_iterator_next (iterator, &vPad) == GST_ITERATOR_OK) {
    sinkpad = GST_PAD (g_value_get_object (&vPad));
    sinkPads++;
  }
  gst_iterator_free (iterator);

  /* Verify if the element only have one input and output */
  if ((1 == srcPads) & (1 == sinkPads)) {
    gst_proctime_add_in_list (procTime, name, sinkpad, srcpad);
  }
}

void
gst_proctime_proc_time (GstProcTime * procTime, GstClockTime * time,
    gchar ** name, GstPad * peerPad, GstPad * srcPad)
{
  GstProcTimeElement *element;
  GstClockTime stopTime;
  gint elem_idx;
  gint elem_num;

  g_return_if_fail (procTime);
  g_return_if_fail (time);
  g_return_if_fail (name);
  g_return_if_fail (srcPad);
  g_return_if_fail (peerPad);

  element = procTime->element;
  elem_num = procTime->elem_num;

  *name = NULL;
  /* Search the peer pad in the list 
   * The peer pad is used to identify which is the element where the 
   * buffer is received.
   */
  for (elem_idx = 0; elem_idx < elem_num; ++elem_idx) {
    if (element[elem_idx].sinkPad == peerPad) {
      element[elem_idx].startTime = gst_util_get_timestamp ();
      element[elem_idx].sinkthread = g_thread_self ();
    }
  }

  /* Search the src pad in the list 
   * The peer pad is used to identify which is the element where the 
   * buffer was processed.
   * If the src pad is not in the list, then it is a src element and the
   * precessing time is not computed
   */
  for (elem_idx = 0; elem_idx < elem_num; ++elem_idx) {
    if (element[elem_idx].srcPad == srcPad) {
      stopTime = gst_util_get_timestamp ();
      *time = stopTime - element[elem_idx].startTime;
      element[elem_idx].srcthread = g_thread_self ();
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
  if (NULL == element->sinkthread || NULL == element->srcthread) {
    return FALSE;
  }

  return element->sinkthread != element->srcthread;
}
