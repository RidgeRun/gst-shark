#include <gst/gst.h>

gboolean gst_nnprofiler_init (void);

void update_cpuusage_event (guint32 cpunum, gfloat * cpuload);
void update_proctime_event (gchar * elementname, guint64 time);
void update_framerate_event (gchar * elementname, guint64 fps);
void update_interlatency_event (gchar * originpad, 
		gchar * destinationpad, guint64 time);

