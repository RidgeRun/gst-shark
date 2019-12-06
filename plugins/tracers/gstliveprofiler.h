#include <gst/gst.h>

void ncurses_row_shift(int i);
void ncurses_col_shift(int i);
void ncurses_initialize(void);
gboolean gst_liveprofiler_init (void);

void print_element(gpointer key, gpointer value, gpointer user_data);
void print_connection(gpointer key, gpointer value, gpointer user_data);

void * curses_loop(void *arg);

void update_cpuusage_event (guint32 cpunum, gfloat * cpuload);
void update_proctime_event (gchar * elementname, guint64 time);
void update_framerate_event (gchar * elementname, gchar * padname,  guint64 fps);
void update_interlatency_event (gchar * originpad, 
		gchar * destinationpad, guint64 time);
void update_pipeline_init (GstPipeline * element);
	
G_BEGIN_DECLS
G_END_DECLS
