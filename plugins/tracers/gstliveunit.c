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
	return e;
}

PadUnit * pad_unit_new(void)
{
	PadUnit * p = g_malloc0 (sizeof(PadUnit));
	return p;
}
