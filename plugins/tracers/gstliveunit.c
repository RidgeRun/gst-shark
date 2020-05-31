#include "gstliveunit.h"
#include "glib.h"
#include <stdio.h>

gboolean
is_filter (GstElement * element)
{
  GList *pads;
  GList *iterator;
  GstPad *pPad;
  gint sink = 0;
  gint src = 0;

  pads = GST_ELEMENT_PADS (element);
  iterator = g_list_first (pads);
  while (iterator != NULL) {
    pPad = iterator->data;
    if (gst_pad_get_direction (pPad) == GST_PAD_SRC)
      src++;
    if (gst_pad_get_direction (pPad) == GST_PAD_SINK)
      sink++;
    if (src > 1 || sink > 1)
      return FALSE;

    iterator = g_list_next (iterator);
  }
  if (src == 1 && sink == 1)
    return TRUE;
  else
    return FALSE;
}

static gboolean
is_queue (GstElement * element)
{
  static GstElementFactory *qfactory = NULL;
  GstElementFactory *efactory;

  g_return_val_if_fail (element, FALSE);

  if (NULL == qfactory) {
    qfactory = gst_element_factory_find ("queue");
  }

  efactory = gst_element_get_factory (element);

  return efactory == qfactory;
}

void
avg_update_value (AvgUnit * unit, guint64 value)
{
  unit->value = value;
  unit->avg += (value - unit->avg) / ++(unit->num);
}

AvgUnit *
avg_unit_new (void)
{
  AvgUnit *a = g_malloc0 (sizeof (AvgUnit));
  a->value = 0;
  a->num = 0;
  a->avg = 0;
  return a;
}

/******************************************************
 * ElementUnit-related functions                      *
 ******************************************************/

ElementUnit *
element_unit_new (GstElement * element)
{
  ElementUnit *e = g_malloc0 (sizeof (ElementUnit));
  e->element = element;
  e->pad = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, pad_unit_free);

  e->time = 0;

  e->proctime = avg_unit_new ();
  e->queue_level = 0;
  e->max_queue_level = 0;

  e->is_filter = is_filter (element);
  e->is_queue = is_queue (element);
  return e;
}

gboolean
element_unit_free (ElementUnit * element)
{
  g_hash_table_destroy (element->pad);
  g_free (element->proctime);
  g_free (element);

  return TRUE;
}

/******************************************************
 * PadUnit-related functions                          *
 ******************************************************/

PadUnit *
pad_unit_new (ElementUnit * element)
{
  PadUnit *p = g_malloc0 (sizeof (PadUnit));

  p->element = element;
  p->time_log = g_queue_new ();
  p->time = 0;

  p->buffer_size = avg_unit_new ();
  p->datarate = 0;
  p->num = 0;
  return p;
}

gboolean
pad_unit_free (PadUnit * element)
{
  g_queue_free_full (element->time_log, g_free);
  g_free (element->buffer_size);
  g_free (element);
}

PadUnit *
pad_unit_peer (GHashTable * elements, PadUnit * target)
{
  GstPad *peer = gst_pad_get_peer ((GstPad *) target->element);
  ElementUnit *peerElementUnit;
  PadUnit *peerPadUnit;

  peerElementUnit = g_hash_table_lookup (elements,
      GST_OBJECT_NAME (GST_OBJECT_PARENT (peer)));
  peerPadUnit = g_hash_table_lookup (peerElementUnit->pad,
      GST_OBJECT_NAME (peer));

  return peerPadUnit;
}

ElementUnit *
pad_unit_parent (GHashTable * elements, PadUnit * target)
{
  return g_hash_table_lookup (elements,
      GST_OBJECT_NAME (GST_OBJECT_PARENT (target->element)));
}

/******************************************************
 * Packet-related new/free functions                  *
 ******************************************************/
Packet *
packet_new (int cpu_num)
{
  Packet *packet = g_malloc (sizeof (Packet));
  packet->cpu_num = cpu_num;
  packet->cpu_load = g_malloc0 (sizeof (gfloat) * cpu_num);
  packet->elements =
      g_hash_table_new_full (g_str_hash, g_str_equal, NULL, element_unit_free);

  return packet;
}

gboolean
packet_free (Packet * packet)
{
  g_free (packet->cpu_load);
  g_hash_table_destroy (packet->elements);
  g_free (packet);
  return TRUE;
}
