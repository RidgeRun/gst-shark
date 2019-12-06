#ifndef __GST_LIVE_UNIT_H__
#define __GST_LIVE_UNIT_H__

#include <gst/gst.h>

G_BEGIN_DECLS

typedef struct _AvgUnit AvgUnit;
typedef struct _ElementUnit ElementUnit;
typedef struct _PadUnit PadUnit;
typedef struct _ConnectionUnit ConnectionUnit;

struct _AvgUnit
{
	guint64 value;
	guint64 num;
	gdouble avg;
};

struct _ElementUnit 
{
	GstElement * element;
	GHashTable * pad;

	AvgUnit * proctime;

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


void avg_update_value (AvgUnit * unit, guint64 value);
AvgUnit * avg_unit_new(void);
ElementUnit * element_unit_new(void);typedef struct _AvgUnit AvgUnit;

G_END_DECLS

#endif
