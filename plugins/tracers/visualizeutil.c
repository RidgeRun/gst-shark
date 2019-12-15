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
#define SELECT_ELEMENT_PAIR 3
#define SELECT_PAD_PAIR 4

// View Mode
char currentModeText[14];

GList * elementIterator = NULL;
GList * padIterator = NULL;
gchar * pairPad = NULL;
gchar * pairElement = NULL;

// Iterator for Hashtable
void print_pad(gpointer key, gpointer value, gpointer user_data) {
	gchar * name = (gchar *) key;
	PadUnit * data = (PadUnit *) value;
	gboolean * selected = (gboolean *) user_data;

	if(padIterator && *selected && strcmp(key, padIterator->data) == 0) {
		attron(A_BOLD); attron(COLOR_PAIR(SELECT_PAD_PAIR));
		mvprintw(row_offset+row_current, 4, "%s", name);
		attroff(A_BOLD); attroff(COLOR_PAIR(SELECT_PAD_PAIR));
		
		pairElement = GST_OBJECT_NAME (GST_OBJECT_PARENT (data->element));
		pairPad = GST_OBJECT_NAME (data->element);
	}
	else {
		mvprintw(row_offset+row_current, 4, "%s", name);
	}

	mvprintw(row_offset+row_current, ELEMENT_NAME_MAX * 4 + 2, 
			"%20f", data->datarate);
	row_current++;
}

//print each element
void print_element(gpointer key, gpointer value, gpointer user_data) {
	gchar * name = (gchar *) key;
	ElementUnit * data = (ElementUnit *) value;
	gboolean * selected = g_malloc(sizeof(gboolean *));

	attron(A_BOLD);
	if(elementIterator && strcmp(key, elementIterator->data) == 0) {
		attron(COLOR_PAIR(SELECT_ELEMENT_PAIR));
		mvprintw(row_offset+row_current, 0, "%s", name);
		attroff(COLOR_PAIR(SELECT_ELEMENT_PAIR));
		*selected = TRUE;
	}
	else {
		mvprintw(row_offset+row_current, 0, "%s", name);
		*selected = FALSE;
	}	
	attroff(A_BOLD);

	mvprintw(row_offset+row_current, ELEMENT_NAME_MAX,
			"%20ld %20f %17d/%2d", 
			data->proctime->value, 
			data->proctime->avg,
			data->queue_level,
			data->max_queue_level);
	row_current++;
	
	g_hash_table_foreach(data->pad, (GHFunc) print_pad, selected);

	g_free(selected);	
}

void initialize(void) 
{
	row_current = 0;
	col_current = 0;
	row_offset = 0; //for scrolling
	strcpy(currentModeText, "list");
	initscr();
	raw();
	start_color();
	init_pair(INVERT_PAIR, COLOR_BLACK, COLOR_WHITE);
	init_pair(TITLE_PAIR, COLOR_BLUE, COLOR_BLACK);
	init_pair(SELECT_ELEMENT_PAIR, COLOR_RED, COLOR_BLACK);
	init_pair(SELECT_PAD_PAIR, COLOR_YELLOW, COLOR_BLACK);
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
	ElementUnit * element = NULL;
	GList * element_key = NULL;
	GList * pad_key = NULL;

	printf("DONE INITIALIZE\n");
    initialize();

    while(1) {
		if(element_key == NULL) {
			element_key = g_hash_table_get_keys(packet->elements);
			elementIterator = g_list_last(element_key);
		}
		else if(pad_key == NULL) {
			element = g_hash_table_lookup(packet->elements, elementIterator->data);
			pad_key = g_hash_table_get_keys(element->pad);
			padIterator = g_list_last(pad_key);		
		}

        row_current = 0;
        col_current = 0;

	timeout(0);
        key_in = getch();
	//key binding
        if (key_in == 'q' || key_in == 'Q') break;
	
	switch(key_in) {//key value
		case 'w': //arrow right
			if(g_list_next(elementIterator)) {
				elementIterator = g_list_next(elementIterator);
				element = g_hash_table_lookup(packet->elements, elementIterator->data);
				pad_key = g_hash_table_get_keys(element->pad);
				padIterator = g_list_last(pad_key);
			}
			break;
		case 's': //arrow left
			if(g_list_previous(elementIterator)) {
				elementIterator = g_list_previous(elementIterator);
				element = g_hash_table_lookup(packet->elements, elementIterator->data);
				pad_key = g_hash_table_get_keys(element->pad);
				padIterator = g_list_last(pad_key);
			}
			break;
		case 'a':
			if(g_list_next(padIterator)) {
				padIterator = g_list_next(padIterator);
			}
			break;
		case 'd':
			if(g_list_previous(padIterator)) {
				padIterator = g_list_previous(padIterator);
			}
			break;
		case 32:
			if(element_key && pad_key) {
				elementIterator = g_list_first(element_key);
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
	print_line(&row_current, &col_current);
	
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

    iter++;
    refresh();
    milsleep(TIMESCALE / 4);
		
    }
    endwin();
    return NULL;
}
