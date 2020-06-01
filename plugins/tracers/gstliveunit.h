#ifndef __GST_LIVE_UNIT_H__
#define __GST_LIVE_UNIT_H__

#include <gst/gst.h>

G_BEGIN_DECLS typedef struct _AvgUnit AvgUnit;
typedef struct _ElementUnit ElementUnit;
typedef struct _PadUnit PadUnit;
typedef struct _LogUnit LogUnit;
typedef struct _Packet Packet;

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
  guint32 queue_level;
  guint32 max_queue_level;

  guint32 elem_idx;             // for log metadata

  gboolean is_filter;
  gboolean is_queue;
};

struct _PadUnit
{
  GstElement *element;
  GQueue *time_log;
  guint64 time;

  guint32 elem_idx;             // for log metadata

  AvgUnit *buffer_size;
  gdouble datarate;
  guint32 num;
};

struct _LogUnit
{
  guint64 proctime;
  guint32 queue_level;
  guint32 max_queue_level;
  guint32 bufrate;
};

struct _Packet
{
  gint cpu_num;
  gfloat *cpu_load;
  GHashTable *elements;
};

gboolean is_filter (GstElement * element);
static gboolean is_queue (GstElement * element);

void avg_update_value (AvgUnit * unit, guint64 value);
AvgUnit *avg_unit_new (void);
ElementUnit *element_unit_new (GstElement * element);
gboolean element_unit_free (ElementUnit * element);

PadUnit *pad_unit_new (ElementUnit * element);
gboolean pad_unit_free (PadUnit * element);
PadUnit *pad_unit_peer (GHashTable * elements, PadUnit * target);
ElementUnit *pad_unit_parent (GHashTable * elements, PadUnit * target);

Packet *packet_new (int cpu_num);
gboolean packet_free (Packet * packet);

G_END_DECLS
#endif
