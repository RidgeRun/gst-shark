#include <gst/gst.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/sysinfo.h>

#include "gstliveprofiler.h"
#include "visualizeutil.h"

// Plugin Data
Packet * packet;

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
		eUnit = g_malloc0 (sizeof(ElementUnit));
		eUnit->element = element;
		eUnit->pad = g_hash_table_new (g_str_hash, g_str_equal);

		children = GST_ELEMENT_PADS (element);
		iter = g_list_first (children);

		while(iter != NULL) {
			pUnit = g_malloc0 (sizeof(PadUnit));
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
update_proctime_event (gchar * elementname, guint64 time) 
{
	GHashTable * elements = packet->elements;
	gchar * key = strdup(elementname);
	ElementUnit * pElement;

	pElement = g_hash_table_lookup(elements, key);
	g_return_if_fail(pElement);

	pElement->proctime = time;
}

void 
update_framerate_event (gchar * elementname, gchar *padname, guint64 fps) 
{
	GHashTable * elements = packet->elements;
	ElementUnit * pElement;
	PadUnit * pPad;

	pElement = g_hash_table_lookup(elements, elementname);
	g_return_if_fail(pElement);
	pPad = g_hash_table_lookup(pElement->pad, padname);
	g_return_if_fail(pPad);

	pPad->framerate = fps;		
}

void 
update_interlatency_event (gchar * originpad, 
		gchar * destinationpad, guint64 time) 
{
	/*
	char generated_key[60];
	char * key;
	ProfilerConnection * pElement, * pTemp;

	strcpy(generated_key, originpad);
	strcat(generated_key, " ");
	strcat(generated_key, destinationpad);
	key = strdup(generated_key);
	
	pElement = LIVE_GET_CONNECTION(key);
	if(pElement == NULL)
	{
		pTemp = malloc (sizeof(ProfilerConnection));
		pTemp->interlatency = time;
		LIVE_PUT_CONNECTION(key, pTemp);
	}
	else
	{
		LIVE_MODIFY_INTERLATENCY(pElement, time);
	}
	*/
	return;
}

void
update_pipeline_init (GstPipeline * element) 
{
	add_children_recursively ((GstElement *) element, packet->elements);
}
