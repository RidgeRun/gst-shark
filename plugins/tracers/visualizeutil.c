#include <ncurses.h>

#include "visualizeutil.h"
#include "gstliveprofiler.h"
#include "gstliveunit.h"

void print_pad(gpointer key, gpointer value, gpointer user_data);
void print_elements(gpointer key, gpointer value, gpointer user_data);

void milsleep(int ms) {
    struct timespec ts;
    ts.tv_sec = ms/1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

// NCurses location
int ncurses_row_current=0;
int ncurses_col_current=0;

// Iterator for Hashtable
void print_pad(gpointer key, gpointer value, gpointer user_data) {
	gchar * name = (gchar *) key;
	PadUnit * data = (PadUnit *) value;
	mvprintw(ncurses_row_current, 4, "%s", name);
	mvprintw(ncurses_row_current, ELEMENT_NAME_MAX * 2, 
			"%20d", data->framerate);
	ncurses_row_current++;
}

void print_element(gpointer key, gpointer value, gpointer user_data) {
	gchar * name = (gchar *) key;
	ElementUnit * data = (ElementUnit *) value;
	
	mvprintw(ncurses_row_current, 0, "%s", name);
	mvprintw(ncurses_row_current, ELEMENT_NAME_MAX,
			"%8ld(%8.2f)", 
			data->proctime->value, 
			data->proctime->avg);
	ncurses_row_current++;
	
	g_hash_table_foreach(data->pad, (GHFunc) print_pad, NULL);	
}

void print_connection(gpointer key, gpointer value, gpointer user_data) {
	/*
	char * connection_key = strdup((char *) key);
	char * sname = strtok(connection_key, " ");
	char * dname = strtok(NULL, " ");
	ProfilerConnection * data = (ProfilerConnection *) value;

	mvprintw(ncurses_row_current, 0, "%s", sname);
	mvprintw(ncurses_row_current, ELEMENT_NAME_MAX, "%s", dname);
	mvprintw(ncurses_row_current, ELEMENT_NAME_MAX * 2,
			"%18dns", data->interlatency);
	ncurses_row_current++;
	*/
}

void ncurses_initialize(void) 
{
	ncurses_row_current = 0;
	ncurses_col_current = 0;
	initscr();
	raw();
	keypad(stdscr, TRUE);
	curs_set(0);
	noecho();
}

inline void print_line(int * row, int * col) {
	for(int i = 0; i < 5; i++) {
		mvprintw(*row, *col + COL_SCALE * i, "---------------------");
	}

	(*row)++;
}

void * curses_loop(void *arg)
{
	Packet * packet = (Packet *) arg;
    int key_in;
    int iter = 0;
	int i;
    ncurses_initialize();

    while(1) {
        ncurses_row_current = 0;
        ncurses_col_current = 0;

		timeout(0);
        key_in = getch();
        if (key_in == 'q' || key_in == 'Q') break;
        clear();

        // draw
		mvprintw(ncurses_row_current++, 0, "it: %d", iter);
        mvprintw(ncurses_row_current++, ncurses_col_current, "Press 'q' or 'Q' to quit");
		print_line(&ncurses_row_current, &ncurses_col_current);

        //CPU Usage
        mvprintw(ncurses_row_current++, ncurses_col_current, "CPU Usage : ");

        i=0;
        while(i < packet->cpu_num) {
            mvprintw(ncurses_row_current+1, ncurses_col_current, "CPU                |");
            mvprintw(ncurses_row_current+2, ncurses_col_current, "CPU Usage          |");
            while(i < packet->cpu_num) {
                ncurses_col_current += COL_SCALE;
                mvprintw(ncurses_row_current+1, ncurses_col_current, "%20d|", i);
                mvprintw(ncurses_row_current+2, ncurses_col_current, "%19f%%|", packet->cpu_load[i]);
                i++;
            }
            ncurses_row_current += 3;
            ncurses_col_current = 0; 
        }

		print_line(&ncurses_row_current, &ncurses_col_current);

		// Proctime & Framerate
		mvprintw(ncurses_row_current, 0, "ElementName");
		mvprintw(ncurses_row_current++, ELEMENT_NAME_MAX,
			   "%20s %20s", "Proctime(ns)", "Framerate(fps)");	

		g_hash_table_foreach(packet->elements, (GHFunc) print_element, NULL);
/*
		// Interlatency
		mvprintw(ncurses_row_current, 0, "Source");
		mvprintw(ncurses_row_current, ELEMENT_NAME_MAX, "Destination");
		mvprintw(ncurses_row_current++, ELEMENT_NAME_MAX * 2, 
				"%20s", "Interlatency");

		g_hash_table_foreach(CONNECTIONS(packet), (GHFunc) print_connection, NULL);
*/
        iter++;
        refresh();
        milsleep(TIMESCALE / 4);
		
    }
    endwin();
	return NULL;
}
