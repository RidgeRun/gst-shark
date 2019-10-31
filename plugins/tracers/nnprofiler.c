#include <gst/gst.h>
#include <curses.h>

#include "nnprofiler.h"

gboolean 
gst_nnprofiler_init (void) 
{
	//TODO: Initialize visual form for nnprofiler
	// input: none
	// output: TRUE if successfully initialized,
	//         FALSE if failed initializing
	
	return TRUE;
}

void 
update_cpuusage_event (guint32 cpunum, gfloat * cpuload) 
{
	//TODO: Update output with cpuusage
	// input: cpunum (number of cpu)
	//        cpuload (array of load of each cpu)
}

void 
update_proctime_event (gchar * elementname, guint64 time) 
{
	//TODO: Update output with proctime
	// input: elementname (name of the element)
	//        time (tiem consumed in element)
}

void 
update_framerate_event (gchar * elementname, guint64 fps) 
{
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
}
