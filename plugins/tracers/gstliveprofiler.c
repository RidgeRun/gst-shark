#include <gst/gst.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/sysinfo.h>

#include "gstliveprofiler.h"
#include "visualizeutil.h"

#define NN_CPU_NUM _cpu_num
#define NN_CPU_LOAD _cpu_load
#define NN_ELEMENTS _prof_elements
#define NN_CONNECTIONS _prof_connections

#define NN_CPU_LOAD_SET(src) \
	memcpy (_cpu_load, src, _cpu_num * sizeof(gfloat))

#define NN_GET_ELEMENT(name) \
	g_hash_table_lookup(_prof_elements, name)
#define NN_PUT_ELEMENT(name, element) \
	g_hash_table_insert(_prof_elements, name, element);
#define NN_MODIFY_PROCTIME(element, value) \
	element->proctime = value
#define NN_MODIFY_FRAMERATE(element, value) \
	element->framerate = value

#define NN_GET_CONNECTION(key) \
	g_hash_table_lookup(_prof_connections, key)
#define NN_PUT_CONNECTION(key, data) \
	g_hash_table_insert(_prof_connections, key, data);
#define NN_MODIFY_INTERLATENCY(connection, value) \
	connection->interlatency = value

// Plugin Data
gint _cpu_num;
gfloat * _cpu_load;
GHashTable * _prof_elements;
GHashTable * _prof_connections;
GSList ** _elements_list_p;

gboolean
gst_liveprofiler_init (void) 
{
	// Allocating global variable for CPU load
	NN_CPU_NUM = get_nprocs();
	NN_CPU_LOAD = g_malloc0 (sizeof(gfloat) * NN_CPU_NUM);
	g_return_val_if_fail(NN_CPU_LOAD, FALSE);

	// Allocating global variable for each element
	NN_ELEMENTS = g_hash_table_new (g_str_hash, g_str_equal);
	NN_CONNECTIONS = g_hash_table_new (g_str_hash, g_str_equal);

	// Check coe
	_elements_list_p = g_malloc0 (sizeof(GSList *));
	*(_elements_list_p) = NULL;

	// Create thread for NCurses
	Packet * packet = malloc (sizeof(Packet));
	packet->cpu_num = _cpu_num;
	packet->cpu_load = _cpu_load;
	packet->prof_elements = _prof_elements;
	packet->prof_connections = _prof_connections;
	packet->elements = _elements_list_p;

	pthread_t thread;
	pthread_create(&thread, NULL, curses_loop, packet);
	pthread_detach(thread);

	return TRUE;
}

void 
update_cpuusage_event (guint32 cpunum, gfloat * cpuload) 
{
	NN_CPU_LOAD_SET(cpuload);
}

void 
update_proctime_event (gchar * elementname, guint64 time) 
{
	gchar * key = strdup(elementname);
	ProfilerElement * pElement, *pTemp;

	pElement = NN_GET_ELEMENT(key);
	if (pElement == NULL)
	{
		pTemp = malloc (sizeof(ProfilerElement));
		pTemp->proctime = time;
		pTemp->framerate = 0;
		NN_PUT_ELEMENT(key, pTemp);
	}
	else
	{
		NN_MODIFY_PROCTIME(pElement, time);
	}

	return;
}

void 
update_framerate_event (gchar * elementname, guint64 fps) 
{
	gchar * key = strdup(elementname);
	ProfilerElement * pElement, *pTemp;

	pElement = NN_GET_ELEMENT(key);
	if (pElement == NULL)
	{
		pTemp = malloc (sizeof(ProfilerElement));
		pTemp->proctime = 0;
		pTemp->framerate = fps;
		NN_PUT_ELEMENT(key, pTemp);
	}
	else
	{
		NN_MODIFY_FRAMERATE(pElement, fps);
	}

	return;
}

void 
update_interlatency_event (gchar * originpad, 
		gchar * destinationpad, guint64 time) 
{
	char generated_key[60];
	char * key;
	ProfilerConnection * pElement, * pTemp;

	strcpy(generated_key, originpad);
	strcat(generated_key, " ");
	strcat(generated_key, destinationpad);
	key = strdup(generated_key);
	
	pElement = NN_GET_CONNECTION(key);
	if(pElement == NULL)
	{
		pTemp = malloc (sizeof(ProfilerConnection));
		pTemp->interlatency = time;
		NN_PUT_CONNECTION(key, pTemp);
	}
	else
	{
		NN_MODIFY_INTERLATENCY(pElement, time);
	}
	return;
}

void
update_new_element_event (gchar * elementname, GstClockTime ts) 
{
	printf("[%s]\n", elementname);
	*(_elements_list_p) = g_slist_append(*(_elements_list_p), elementname);
}
