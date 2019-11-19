#include <gst/gst.h>

G_BEGIN_DECLS

#define COL_SCALE 21
#define TIMESCALE 400
#define ELEMENT_NAME_MAX 20

#define CPU_NUM(packet) \
	(packet->cpu_num)
#define CPU_LOAD(packet) \
	(packet->cpu_load)
#define ELEMENTS(packet) \
	(packet->elements)
#define CONNECTIONS(packet) \
	(packet->connections)

typedef struct _Packet Packet;

struct _Packet 
{
	gint cpu_num;
	gfloat * cpu_load;
	GHashTable * elements;
	GHashTable * connections;
};

G_END_DECLS
