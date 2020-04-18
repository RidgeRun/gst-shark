#include <gst/gst.h>
#include "gstliveunit.h"

void ncurses_row_shift (int i);
void ncurses_col_shift (int i);
void ncurses_initialize (void);
gboolean gst_liveprofiler_init (void);

void print_element (gpointer key, gpointer value, gpointer user_data);
void print_connection (gpointer key, gpointer value, gpointer user_data);


void update_cpuusage_event (guint32 cpunum, gfloat * cpuload);
void update_pipeline_init (GstPipeline * element);

void update_proctime (ElementUnit * element, ElementUnit * peerElement,
    guint64 ts);
void update_datatrate (PadUnit * pad, PadUnit * peerPad, guint64 ts);
void update_buffer_size (PadUnit * pad, PadUnit * peerPad, guint64 size);

void element_push_buffer_pre (gchar * elementname, gchar * padname, guint64 ts,
    guint64 buffer_size);
void element_push_buffer_post (gchar * elementname, gchar * padname,
    guint64 ts);
void element_push_buffer_list_pre (gchar * elementname, gchar * padname,
    guint64 ts);
void element_push_buffer_list_post (gchar * elementname, gchar * padname,
    guint64 ts);
void element_pull_range_pre (gchar * elementname, gchar * padname, guint64 ts);
void element_pull_range_post (gchar * elementname, gchar * padname, guint64 ts);


G_BEGIN_DECLS G_END_DECLS
