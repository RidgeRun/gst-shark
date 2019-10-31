#include <gst/gst.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#include "nnprofiler.h"

#define ROW_MAX 100
#define COL_MAX 100
#define TIMESCALE 400
#define RUNTIME_MINUTE 1
#define NUMBER_OF_CPU 4
#define NUMBER_OF_NN 20
#define NUMBER_OF_EDGE 30*2
#define COL_SCALE 21

//global_variables
int ncurses_row_current=0;
int ncurses_col_current=0;

float ncurses_cpuload[NUMBER_OF_CPU];

struct ncurses_NN {
    gboolean used;
    gchar name[30];
    guint64 proctime;
    guint64 framerate;

}ncurses_NN[NUMBER_OF_NN];
int max_element = 0;

struct ncurses_NN_edge {
    gboolean used; 
    gchar sname[30];
    gchar dname[30];
    guint64 latency;
}ncurses_NN_edge[NUMBER_OF_EDGE];
int max_edge = 0;

void *curses_loop(void* arg);

void milsleep(int ms) {
    struct timespec ts;
    ts.tv_sec = ms/1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void ncurses_row_shift(int i) {
    ncurses_row_current = ncurses_row_current + i;
    return;
}
void ncurses_col_shift(int i) {
    ncurses_col_current = ncurses_col_current + i;
    return;
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
        while(i<NUMBER_OF_CPU) {
            mvprintw(ncurses_row_current+1, ncurses_col_current, "CPU                |");
            mvprintw(ncurses_row_current+2, ncurses_col_current, "CPU Usage          |");
            while(i<NUMBER_OF_CPU && ncurses_col_current<COL_MAX) {
                ncurses_col_current += COL_SCALE;
                mvprintw(ncurses_row_current+1, ncurses_col_current, "%20d|", i);
                mvprintw(ncurses_row_current+2, ncurses_col_current, "%19f%%|", ncurses_cpuload[i]);
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

        //Proctime&Framerate
        mvprintw(ncurses_row_current++, ncurses_col_current, "Proctime&Framerate : ");
        i=0;

        while(i<max_element && ncurses_NN[i].used ) {
            mvprintw(ncurses_row_current+1, ncurses_col_current, "NeuralNetwork      |");
            mvprintw(ncurses_row_current+2, ncurses_col_current, "Proctime           |");
            mvprintw(ncurses_row_current+3, ncurses_col_current, "Framerate          |");
            while(i<max_element && ncurses_col_current<COL_MAX && ncurses_NN[i].used) {
                ncurses_col_current += COL_SCALE;
                mvprintw(ncurses_row_current+1, ncurses_col_current, "%20s|", ncurses_NN[i].name);
                mvprintw(ncurses_row_current+2, ncurses_col_current, "%18lums|", ncurses_NN[i].proctime);
                mvprintw(ncurses_row_current+3, ncurses_col_current, "%17lufps|", ncurses_NN[i].framerate);
                i++;
            }  
            ncurses_row_current += 4;
            ncurses_col_current = 0;         
        }


        i=0;
        while(i<=5) {
            mvprintw(ncurses_row_current, ncurses_col_current+COL_SCALE*i, "---------------------");
            i++;
        }
        ncurses_row_current++;
        
        //Interlatency
        mvprintw(ncurses_row_current++, ncurses_col_current, "Interlatency : ");
        i=0;
        while(i<max_edge && ncurses_NN_edge[i].used) {
            mvprintw(ncurses_row_current+1, ncurses_col_current, "Source             |");
            mvprintw(ncurses_row_current+2, ncurses_col_current, "Destination        |");
            mvprintw(ncurses_row_current+3, ncurses_col_current, "latency            |");
            while(i < max_edge && ncurses_col_current<COL_MAX && ncurses_NN_edge[i].used) {
                ncurses_col_current += COL_SCALE;
                mvprintw(ncurses_row_current+1, ncurses_col_current, "%20s|", ncurses_NN_edge[i].sname);
                mvprintw(ncurses_row_current+2, ncurses_col_current, "%20s|", ncurses_NN_edge[i].dname);
                mvprintw(ncurses_row_current+3, ncurses_col_current, "%18lums|", ncurses_NN_edge[i].latency);
                i++;
            }
            ncurses_row_current += 4;
            ncurses_col_current = 0;                
        }

        i=0;
        while(i<=5) {
            mvprintw(ncurses_row_current, ncurses_col_current+COL_SCALE*i, "---------------------");
            i++;
        }
        ncurses_row_current++;

        iter++;
        refresh();
        milsleep(TIMESCALE);
    }

    endwin();
}

gboolean
gst_nnprofiler_init (void) 
{
	//TODO: Initialize visual form for nnprofiler
	// input: none
	// output: TRUE if successfully initialized,
	//         FALSE if failed initializing
	pthread_t thread;
	pthread_create(&thread, NULL, curses_loop, NULL);
	pthread_detach(thread);

	return TRUE;
}



void 
update_cpuusage_event (guint32 cpunum, gfloat * cpuload) 
{	
	for(int i=0; i<cpunum; i++) {
		ncurses_cpuload[i] = cpuload[i];
	}
	//TODO: Update output with cpuusage
	// input: cpunum (number of cpu)
	//        cpuload (array of load of each cpu)
}

void 
update_proctime_event (gchar * elementname, guint64 time) 
{
	for(int i=0; i<max_element; i++) {
		if(strcmp(ncurses_NN[i].name, elementname) == 0) {
			ncurses_NN[i].used = TRUE;
			ncurses_NN[i].proctime = time;
			return;
		}
	}
	strcpy(ncurses_NN[max_element].name, elementname);
	ncurses_NN[max_element].used = TRUE;
	ncurses_NN[max_element].proctime = time;
	ncurses_NN[max_element].framerate = 0;
	max_element += 1;
	
	return;
	//TODO: Update output with proctime
	// input: elementname (name of the element)
	//        time (tiem consumed in element)
}

void 
update_framerate_event (gchar * elementname, guint64 fps) 
{
	for(int i=0; i<max_element; i++) {
		if(strcmp(ncurses_NN[i].name, elementname) == 0) {
			ncurses_NN[i].used = TRUE;
			ncurses_NN[i].framerate = fps;
			return;
		}
	}
	ncurses_NN[max_element].used = TRUE;
	strcpy(ncurses_NN[max_element].name, elementname);
	ncurses_NN[max_element].proctime = 0;
	ncurses_NN[max_element].framerate = fps;
	max_element += 1;

	return;
	//TODO: Update output with framerate
	// input: elementname (name of the element)
	//        fps (framerate of element)
}

void 
update_interlatency_event (gchar * originpad, 
		gchar * destinationpad, guint64 time) 
{
	//TODO: Update output with interlatency
	// input: originpad (name of the pad buffer started)
	//        destinationpad (name of the pad buffer arrived)
	//        time (interlatency between two pads)
	for(int i=0; i < max_edge; i++) {
		if((strcmp(ncurses_NN_edge[i].sname, originpad) == 0) && (strcmp(ncurses_NN_edge[i].dname, destinationpad) == 0)) {
			ncurses_NN_edge[i].used = TRUE;
			ncurses_NN_edge[i].latency = time;
			return;
		}
	}
	ncurses_NN_edge[max_edge].used = TRUE;
	strcpy(ncurses_NN_edge[max_edge].sname, originpad);
	strcpy(ncurses_NN_edge[max_edge].dname, destinationpad);
	ncurses_NN_edge[max_edge].latency = time;
	max_edge += 1;

	return;
}
