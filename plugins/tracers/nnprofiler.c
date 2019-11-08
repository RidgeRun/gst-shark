#include <gst/gst.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "nnprofiler.h"

#define TIMESCALE 400
#define COL_SCALE 21
#define ELEMENT_NAME_MAX 20

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


// NCurses location
int ncurses_row_current=0;
int ncurses_col_current=0;

// Iterator for Hashtable
void print_element(gpointer key, gpointer value, gpointer user_data) {
	char * name = (char *) key;
	ProfilerElement * data = (ProfilerElement *) value;
	
	mvprintw(ncurses_row_current, 0, "%s", name);
	mvprintw(ncurses_row_current, ELEMENT_NAME_MAX,
			"%18dns %17dfps", data->proctime, data->framerate);
	ncurses_row_current++;
}

void print_connection(gpointer key, gpointer value, gpointer user_data) {
	char * connection_key = strdup((char *) key);
	char * sname = strtok(connection_key, " ");
	char * dname = strtok(NULL, " ");
	ProfilerConnection * data = (ProfilerConnection *) value;


	mvprintw(ncurses_row_current, 0, "%s", sname);
	mvprintw(ncurses_row_current, ELEMENT_NAME_MAX, "%s", dname);
	mvprintw(ncurses_row_current, ELEMENT_NAME_MAX * 2,
			"%18dns", data->interlatency);
	ncurses_row_current++;
}


// Plugin Data
gint _cpu_num;
gfloat * _cpu_load;
GHashTable * _prof_elements;
GHashTable * _prof_connections;

void milsleep(int ms) {
    struct timespec ts;
    ts.tv_sec = ms/1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void ncurses_initialize(void) {
	initscr();
	raw();
	keypad(stdscr, TRUE);
	curs_set(0);
	noecho();
}

void* curses_loop(void *arg){
    ncurses_initialize();

    int key_in;
    int iter = 0;
    while(1) {
        ncurses_row_current = 0;
        ncurses_col_current = 0;
        timeout(0);
        key_in = getch();
        if (key_in == 'q' || key_in == 'Q') break;
        clear();

        // draw
        mvprintw(ncurses_row_current++, ncurses_col_current, "Press 'q' or 'Q' to quit");
        int i=0;
        while(i<=5) {
            mvprintw(ncurses_row_current, ncurses_col_current+COL_SCALE*i, "---------------------");
            i++;
        }
        ncurses_row_current++;

        //CPU Usage
        mvprintw(ncurses_row_current++, ncurses_col_current, "CPU Usage : ");
        i=0;
        while(i<NN_CPU_NUM) {
            mvprintw(ncurses_row_current+1, ncurses_col_current, "CPU                |");
            mvprintw(ncurses_row_current+2, ncurses_col_current, "CPU Usage          |");
            while(i<NN_CPU_NUM && ncurses_col_current<COL_MAX) {
                ncurses_col_current += COL_SCALE;
                mvprintw(ncurses_row_current+1, ncurses_col_current, "%20d|", i);
                mvprintw(ncurses_row_current+2, ncurses_col_current, "%19f%%|", NN_CPU_LOAD[i]);
                i++;
            }
            ncurses_row_current += 3;
            ncurses_col_current = 0;            
        }

        i=0;
        while(i<=5) {
            mvprintw(ncurses_row_current, ncurses_col_current+COL_SCALE*i, "---------------------");
            i++;
        }
        ncurses_row_current++;

		// Proctime & Framerate
		mvprintw(ncurses_row_current, 0, "ElementName");
		mvprintw(ncurses_row_current++, ELEMENT_NAME_MAX,
			   "%20s %20s", "Proctime", "Framerate");	

		g_hash_table_foreach(NN_ELEMENTS, (GHFunc) print_element, NULL);

		// Interlatency
		mvprintw(ncurses_row_current, 0, "Source");
		mvprintw(ncurses_row_current, ELEMENT_NAME_MAX, "Destination");
		mvprintw(ncurses_row_current++, ELEMENT_NAME_MAX * 2, 
				"%20s", "Interlatency");

		g_hash_table_foreach(NN_CONNECTIONS, (GHFunc) print_connection, NULL);

        iter++;
        refresh();
        milsleep(TIMESCALE);
		
    }

    endwin();
}

gboolean
gst_nnprofiler_init (void) 
{
	// Allocating global variable for CPU load
	NN_CPU_NUM = get_nprocs();
	NN_CPU_LOAD = g_malloc0 (sizeof(gfloat) * NN_CPU_NUM);
	g_return_val_if_fail(NN_CPU_LOAD, FALSE);

	// Allocating global variable for each element
	NN_ELEMENTS = g_hash_table_new (g_str_hash, g_str_equal);
	NN_CONNECTIONS = g_hash_table_new (g_str_hash, g_str_equal);

	// Create thread for NCurses
	pthread_t thread;
	pthread_create(&thread, NULL, curses_loop, NULL);
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
