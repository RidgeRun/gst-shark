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

typedef struct _ProfilerElement ProfilerElement;
typedef struct _ProfilerConnection ProfilerConnection;
typedef struct _ElementUnit ElementUnit;
typedef struct _PadUnit PadUnit;
typedef struct _ConnectionUnit ConnectionUnit;

struct _ProfilerElement 
{
	guint64 proctime;
	guint64 framerate;
};

struct _ProfilerConnection
{
	guint64 interlatency;
};

struct _ElementUnit 
{
	GstElement * element;
	GHashTable * pad;

	guint64 proctime;
	guint64 memuse;
	guint64 queuelevel;
};

struct _PadUnit
{
	GstElement * element;

	guint64 framerate;	
};

struct _ConnectionUnit
{
	GstElement * src;
	GstElement * dest;
};

G_END_DECLS
