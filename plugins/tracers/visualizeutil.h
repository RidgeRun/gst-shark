#include <gst/gst.h>
#include <time.h>

void milsleep (int ms);

void *curses_loop (void *arg);

G_BEGIN_DECLS
#define COL_SCALE 21
#define TIMESCALE 400
#define ELEMENT_NAME_MAX 20
#define packet_set_cpu_num(packet, val) \
	packet->cpu_num = val
#define packet_set_cpu_load(packet, val) \
	packet->cpu_load = val
#define packet_set_elements(packet, val) \
	packet->elements = val;
#define packet_set_connections(packet, val) \
	packet->connections = val;
typedef struct _Packet Packet;

struct _Packet
{
  gint cpu_num;
  gfloat *cpu_load;
  GHashTable *elements;
  GHashTable *connections;
};

G_END_DECLS
