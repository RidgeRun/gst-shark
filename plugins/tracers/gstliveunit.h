#ifndef __GST_LIVE_UNIT_H__
#define __GST_LIVE_UNIT_H__

#include <gst/gst.h>

G_BEGIN_DECLS typedef struct _AvgUnit AvgUnit;
typedef struct _ElementUnit ElementUnit;
typedef struct _PadUnit PadUnit;

struct _AvgUnit
{
  guint64 value;
  guint64 num;
  gdouble avg;
};

struct _ElementUnit
{
  GstElement *element;
  GHashTable *pad;

  guint64 time;

  AvgUnit *proctime;

  gboolean is_filter;
  guint32 queue_level;
  guint32 max_queue_level;
};

struct _PadUnit
{
  GstElement *element;

  GQueue *time_log;
  guint64 time;

  AvgUnit *buffer_size;
  gdouble datarate;
  guint32 num;
};

void avg_update_value (AvgUnit * unit, guint64 value);
AvgUnit *avg_unit_new (void);
ElementUnit *element_unit_new (void);

PadUnit *pad_unit_new (void);
PadUnit *pad_unit_peer (GHashTable * elements, PadUnit * target);
ElementUnit *pad_unit_parent (GHashTable * elements, PadUnit * target);

G_END_DECLS
#endif
