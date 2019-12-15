#include "gstliveunit.h"
#include "glib.h"
#include <stdio.h>

void avg_update_value (AvgUnit * unit, guint64 value)
{
	unit->value = value;
	unit->avg += (value - unit->avg) / ++(unit->num);
}

AvgUnit * avg_unit_new(void)
{
	AvgUnit * a = g_malloc0(sizeof(AvgUnit));
	a->num = 0;
	a->avg = 0;
	return a;
}

ElementUnit * element_unit_new(void)
{
	ElementUnit * e = g_malloc0 (sizeof(ElementUnit));
	e->proctime = avg_unit_new();
	e->time = 0;
	return e;
}

PadUnit * pad_unit_new(void)
{
	PadUnit * p = g_malloc0 (sizeof(PadUnit));
	p->time = 0;
	p->time_log = g_queue_new();
	p->data_rate = 0;
	p->num = 0;
	return p;
}

PadUnit * pad_unit_peer(GHashTable * elements, PadUnit * target)
{
	GstPad * peer = gst_pad_get_peer((GstPad *) target->element);
	ElementUnit * peerElementUnit;
	PadUnit * peerPadUnit;
	
	peerElementUnit = g_hash_table_lookup(elements,
		GST_OBJECT_NAME (GST_OBJECT_PARENT (peer)));
	peerPadUnit = g_hash_table_lookup(peerElementUnit->pad,
		GST_OBJECT_NAME(peer));

	return peerPadUnit;
}

ElementUnit * pad_unit_parent(GHashTable * elements, PadUnit * target)
{
	return g_hash_table_lookup(elements,
			GST_OBJECT_NAME (GST_OBJECT_PARENT (target->element)));
}
