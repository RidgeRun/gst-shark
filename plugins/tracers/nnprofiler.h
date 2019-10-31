#include <gst/gst.h>

void milsleep(int ms);
void ncurses_row_shift(int i);
void ncurses_col_shift(int i);
void ncurses_initialize(void);
gboolean gst_nnprofiler_init (void);

void update_cpuusage_event (guint32 cpunum, gfloat * cpuload);
void update_proctime_event (gchar * elementname, guint64 time);
void update_framerate_event (gchar * elementname, guint64 fps);
void update_interlatency_event (gchar * originpad, 
		gchar * destinationpad, guint64 time);

