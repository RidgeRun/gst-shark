#include <gst/gst.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/sysinfo.h>

#include "gstliveprofiler.h"
#include "gstliveunit.h"
#include "visualizeutil.h"

// Plugin Data
Packet * packet;

gboolean
is_filter (GstElement * element)
{
	GList * pads;
	GList * iterator;
	GstPad * pPad;
	gint sink = 0;
	gint src = 0;

	pads = GST_ELEMENT_PADS (element);
	iterator = g_list_first (pads);
	while(iterator != NULL) {
		pPad = iterator->data;
		if(gst_pad_get_direction(pPad) == GST_PAD_SRC) 
			src++;
		if(gst_pad_get_direction(pPad) == GST_PAD_SINK)
			sink++;
		if(src > 1 || sink > 1)
			return FALSE;

		iterator = g_list_next(iterator);
	}
	if(src == 1 && sink == 1)
		return TRUE;
	else
		return FALSE;
}

/*
 * Adds children of element recursively to the Hashtable.
 */
void
add_children_recursively (GstElement * element, GHashTable * table)
{
	GList * children, * iter;
	ElementUnit * eUnit;
	PadUnit * pUnit;

	if (GST_IS_BIN (element)) {
		children = GST_BIN_CHILDREN (element);
		iter = g_list_first (children);
		
		while(iter != NULL) {
			add_children_recursively ((GstElement *) iter->data, table);
			iter = g_list_next (iter);
		}
	}
	else {
		eUnit = element_unit_new();
		eUnit->element = element;
		eUnit->pad = g_hash_table_new (g_str_hash, g_str_equal);
		eUnit->is_filter = is_filter(element);

		children = GST_ELEMENT_PADS (element);
		iter = g_list_first (children);

		while(iter != NULL) {
			pUnit = pad_unit_new();
			pUnit->element = iter->data;
			g_hash_table_insert(eUnit->pad, GST_OBJECT_NAME (iter->data), pUnit);
			
			iter = g_list_next (iter);
		}
		g_hash_table_insert(table, GST_OBJECT_NAME (element), eUnit);
	}
}

gboolean
gst_liveprofiler_init (void) 
{
	gint cpu_num;
	gfloat * cpu_load;
	GHashTable * elements;
	GHashTable * connections;
	
	cpu_num = get_nprocs();
	cpu_load = g_malloc0 (sizeof(gfloat) * cpu_num);
	elements = g_hash_table_new (g_str_hash, g_str_equal);
	connections = g_hash_table_new (g_str_hash, g_str_equal);

	packet = g_malloc0 (sizeof(Packet));
	packet_set_cpu_num (packet, cpu_num);
	packet_set_cpu_load (packet, cpu_load);
	packet_set_elements (packet, elements);
	packet_set_connections (packet, connections);

	pthread_t thread;
	pthread_create(&thread, NULL, curses_loop, packet);
	pthread_detach(thread);

	return TRUE;
}

void 
update_cpuusage_event (guint32 cpunum, gfloat * cpuload) 
{
	gint cpu_num = packet->cpu_num;
	gfloat * cpu_load = packet->cpu_load;

	memcpy (cpu_load, cpuload, cpu_num * sizeof(gfloat));
}

void
update_queue_level_event (const gchar * elementname, guint size_buffer,
		guint max_size_buffer)
{
	GHashTable * elements = packet->elements;
	ElementUnit * pElement;
	
	pElement = g_hash_table_lookup(elements, elementname);
	g_return_if_fail(pElement);
	pElement->queue_level = size_buffer;
	pElement->max_queue_level = max_size_buffer;
}

void
element_push_buffer_pre (gchar * elementname, gchar * padname, guint64 ts, guint64 buffer_size) 
{
	//printf("[%ld]%s-%s pre\n", ts, elementname, padname);
	GHashTable * elements = packet->elements;
	ElementUnit * pElement, * pPeerElement;
	PadUnit * pPad, * pPeerPad;
	gdouble datarate;
	guint length;
	guint64 * pTime;

	pElement = g_hash_table_lookup (elements, elementname);
	g_return_if_fail (pElement);
	pPad = g_hash_table_lookup (pElement->pad, padname);
	g_return_if_fail (pPad);
	pPeerPad = pad_unit_peer(elements, pPad);
	g_return_if_fail (pPeerPad);
	pPeerElement = pad_unit_parent(elements, pPeerPad);
	g_return_if_fail (pPeerElement);

	if(pPeerElement->is_filter) {
		pPeerElement->time = ts;
	}	
	
	if(pElement->is_filter) {
		if(ts - pElement->time > 0) {
			avg_update_value(pElement->proctime, ts - pElement->time);
		}
	}

	length = g_queue_get_length(pPad->time_log);

	if(length == 0) {
		pTime = g_malloc(sizeof(gdouble));
		*pTime = ts;
	}
	else {
		if(length > 10) {
			pTime = (guint64 *) g_queue_pop_tail(pPad->time_log);
		}
		else {
			pTime = (guint64 *) g_queue_peek_tail(pPad->time_log);
		}
		
		datarate = 1e9 * length /  (ts - *pTime);
		pPad->datarate = datarate;
		pPeerPad->datarate = datarate;
		
		pTime = g_malloc(sizeof(gdouble));
		*pTime = ts;
	}
	g_queue_push_head(pPad->time_log, pTime);

	avg_update_value(pPad->buffer_size, buffer_size);
	avg_update_value(pPeerPad->buffer_size, buffer_size);
}

void
element_push_buffer_post (gchar * elementname, gchar * padname, guint64 ts)
{
	//printf("[%ld]%s-%s post\n", ts, elementname, padname);
}

void
element_push_buffer_list_pre (gchar * elementname, gchar * padname, guint64 ts)
{
	//printf("[%ld]%s-%s pre list\n", ts, elementname, padname);
}
void
element_push_buffer_list_post (gchar * elementname, gchar * padname, guint64 ts)
{
	//printf("[%ld]%s-%s post list\n", ts, elementname, padname);
}
void
element_pull_range_pre (gchar * elementname, gchar * padname, guint64 ts)
{
	//printf("[%ld]%s-%s pre pull\n", ts, elementname, padname);
}

void
element_pull_range_post (gchar * elementname, gchar * padname, guint64 ts)
{
	//printf("[%ld]%s-%s post pull\n", ts, elementname, padname);
}



void
update_pipeline_init (GstPipeline * element) 
{
	add_children_recursively ((GstElement *) element, packet->elements);
}
