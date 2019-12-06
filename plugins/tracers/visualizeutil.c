#include <ncurses.h>
#include <string.h>
#include <time.h>

#include "visualizeutil.h"
#include "gstliveprofiler.h"
#include "gstliveunit.h"

void initialize(void);
void print_pad(gpointer key, gpointer value, gpointer user_data);
void print_elements(gpointer key, gpointer value, gpointer user_data);

void milsleep(int ms) {
    struct timespec ts;
    ts.tv_sec = ms/1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

// NCurses location
int row_current=0;
int col_current=0;
int row_offset=0; //for scrolling

// NCurses color scheme
#define INVERT_PAIR	1
#define TITLE_PAIR	2

// View Mode
char currentModeText[14];
int currentMode=0; //0-list, 1-matrix, 2-graph

// Iterator for Hashtable
void print_pad(gpointer key, gpointer value, gpointer user_data) {
	gchar * name = (gchar *) key;
	PadUnit * data = (PadUnit *) value;

	mvprintw(row_offset+row_current, 4, "%s", name);

	mvprintw(row_offset+row_current, ELEMENT_NAME_MAX * 4 + 2, 
			"%20d", data->framerate);
	row_current++;
}

//print each element
void print_element(gpointer key, gpointer value, gpointer user_data) {
	gchar * name = (gchar *) key;
	ElementUnit * data = (ElementUnit *) value;
	
	attron(A_BOLD);
	mvprintw(row_offset+row_current, 0, "%s", name);
	attroff(A_BOLD);

	mvprintw(row_offset+row_current, ELEMENT_NAME_MAX,
			"%20ld %20f %17d/%2d", 
			data->proctime->value, 
			data->proctime->avg,
			data->queue_level,
			data->max_queue_level);
	row_current++;
	
	g_hash_table_foreach(data->pad, (GHFunc) print_pad, NULL);	
}

void print_connection(gpointer key, gpointer value, gpointer user_data) {
	/*
	char * connection_key = strdup((char *) key);
	char * sname = strtok(connection_key, " ");
	char * dname = strtok(NULL, " ");
	ProfilerConnection * data = (ProfilerConnection *) value;

	mvprintw(row_current, 0, "%s", sname);
	mvprintw(row_current, ELEMENT_NAME_MAX, "%s", dname);
	mvprintw(row_current, ELEMENT_NAME_MAX * 2,
			"%18dns", data->interlatency);
	row_current++;
	*/
}

void initialize(void) 
{
	row_current = 0;
	col_current = 0;
	row_offset = 0; //for scrolling
	strcpy(currentModeText, "list");
	currentMode = 0;
	initscr();
	raw();
	start_color();
	init_pair(INVERT_PAIR, COLOR_BLACK, COLOR_WHITE);
	init_pair(TITLE_PAIR, COLOR_BLUE, COLOR_BLACK);
	keypad(stdscr, TRUE);
	curs_set(0);
	noecho();
}

inline void print_line_absolute(int * row, int * col) {
	for(int i = 0; i < 5; i++) {
		mvprintw(*row, *col + COL_SCALE * i, "---------------------");
	}

	(*row)++;
}

inline void print_line(int * row, int * col) {
	for(int i = 0; i < 5; i++) {
		mvprintw(row_offset+*row, *col + COL_SCALE * i, "---------------------");
	}

	(*row)++;
}

void * curses_loop(void *arg)
{
    Packet * packet = (Packet *) arg;
    time_t tmp_t; //for getting time
    struct tm tm; //for getting time
    int key_in;
    int iter = 0;
    int i;

    initialize();

    while(1) {
        row_current = 0;
        col_current = 0;

	timeout(0);
        key_in = getch();
	//key binding
        if (key_in == 'q' || key_in == 'Q') break;
	
	switch(key_in) {//key value
		case 'l':
		case 'L':
			if (currentMode != 0) { // mode switch
				currentMode = 0; //list
				strcpy(currentModeText, "list");
				row_offset = 0; //reset offset
			}
			break;
		case 'm':
		case 'M':
			if (currentMode != 1) { // mode switch
				currentMode = 1; //matrix
				strcpy(currentModeText, "matrix");
				row_offset = 0; //reset offset
			}
			break;
		case 'g':
		case 'G':
			if (currentMode != 2) { // mode switch
				currentMode = 2; //graph
				strcpy(currentModeText, "graph");
				row_offset = 0; //reset offset
			}
			break;
		case 259: //arrow up
			if (row_offset < 0) row_offset++;
			break;
		case 258: //arrow down
			row_offset--; 
			break;
		default:
			break;
	}

	//get time
	tmp_t = time(NULL);
	tm = *localtime(&tmp_t);
        // draw
        clear();
		mvprintw(row_offset+row_current, 36, "key"); //for debug
		mvprintw(row_offset+row_current, 40, "%08d",key_in); //for debug
	attron(A_BOLD);
	mvprintw(row_offset+row_current, 63, "%4d-%02d-%02d %2d:%02d:%02d\n",
		tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec); //time indicator
	attron(COLOR_PAIR(INVERT_PAIR));
        mvprintw(row_offset+row_current++, col_current, "Press 'q' or 'Q' to quit");
	attroff(A_BOLD);
	attroff(COLOR_PAIR(INVERT_PAIR));
	mvprintw(row_offset+row_current, col_current, "Current mode: ");
	mvprintw(row_offset+row_current, col_current + 14, currentModeText);
	mvprintw(row_offset+row_current++, col_current + 28, "('l': list, 'm': matrix, 'g': graph)");
	print_line(&row_current, &col_current);

	switch(currentMode) {
		case 0: //list
			//CPU Usage
			attron(A_BOLD);
			attron(COLOR_PAIR(TITLE_PAIR));
			mvprintw(row_offset+row_current++, col_current, "CPU Usage");
			attroff(A_BOLD);
			attroff(COLOR_PAIR(TITLE_PAIR));
			i=0;
			while(i < packet->cpu_num) {
				attron(A_BOLD);
				mvprintw(row_offset+row_current, col_current, "CPU%2d", i);
				attroff(A_BOLD);
				mvprintw(row_offset+row_current++, col_current+7+4, "%3.1f%%", packet->cpu_load[i]);
				i++;
			}

			print_line(&row_current, &col_current);

			// Proctime & Framerate
			attron(A_BOLD);
			attron(COLOR_PAIR(TITLE_PAIR));
			mvprintw(row_offset+row_current, 0, "ElementName");
			mvprintw(row_offset+row_current++, ELEMENT_NAME_MAX,
				   "%20s %20s %20s %20s", "Proctime(ns)", "Avg_proctime(ns)", "queuelevel", "Framerate(fps)");	
			attroff(COLOR_PAIR(TITLE_PAIR));
			attroff(A_BOLD);

			g_hash_table_foreach(packet->elements, (GHFunc) print_element, NULL);
			/*
				// Interlatency
				mvprintw(row_offset+row_current, 0, "Source");
				mvprintw(row_offset+row_current, ELEMENT_NAME_MAX, "Destination");
				mvprintw(row_offset+row_current++, ELEMENT_NAME_MAX * 2, 
						"%20s", "Interlatency");

				g_hash_table_foreach(CONNECTIONS(packet), (GHFunc) print_connection, NULL);
			*/
			break;
		case 1: //matrix
			//CPU Usage
			mvprintw(row_offset+row_current++, col_current, "CPU Usage : ");

			i=0;
			while(i < packet->cpu_num) {
			    mvprintw(row_offset+row_current+1, col_current, "CPU                |");
			    mvprintw(row_offset+row_current+2, col_current, "CPU Usage          |");
			    while(i < packet->cpu_num) {
				col_current += COL_SCALE;
				mvprintw(row_offset+row_current+1, col_current, "%20d|", i);
				mvprintw(row_offset+row_current+2, col_current, "%19f%%|", packet->cpu_load[i]);
				i++;
			    }
			    row_current += 3;
			    col_current  = 0; 
			}

			print_line(&row_current, &col_current);

			// Proctime & Framerate
			mvprintw(row_offset+row_current, 0, "ElementName");
			mvprintw(row_offset+row_current++, ELEMENT_NAME_MAX,
				   "%20s %20s %20s", "Proctime(ns)", "Avg_proc(ns)", "queuelevel", "Framerate(fps)");	

			g_hash_table_foreach(packet->elements, (GHFunc) print_element, NULL);
			/*
				// Interlatency
				mvprintw(row_offset+row_current, 0, "Source");
				mvprintw(row_offset+row_current, ELEMENT_NAME_MAX, "Destination");
				mvprintw(row_offset+row_current++, ELEMENT_NAME_MAX * 2, 
						"%20s", "Interlatency");

				g_hash_table_foreach(CONNECTIONS(packet), (GHFunc) print_connection, NULL);
			*/
			break;
		case 2:	//graph
			break;
		default:
			break;
	}
        iter++;
        refresh();
        milsleep(TIMESCALE / 4);
		
    }
    endwin();
    return NULL;
}
