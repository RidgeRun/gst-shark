#include <gst/gst.h>
#include <time.h>

void milsleep(int ms);

G_BEGIN_DECLS

#define COL_SCALE 21
#define TIMESCALE 400
#define ELEMENT_NAME_MAX 20

#define CPU_NUM(packet) \
	(packet->cpu_num)
#define CPU_LOAD(packet) \
	(packet->cpu_load)
#define ELEMENTS(packet) \
	(packet->prof_elements)
#define CONNECTIONS(packet) \
	(packet->prof_connections)
#define ELEMENTS_LIST(packet) \
	(*(packet->elements))

typedef struct _Packet Packet;

struct _Packet 
{
	gint cpu_num;
	gfloat * cpu_load;
	GHashTable * prof_elements;
	GHashTable * prof_connections;
	GSList ** elements;
};

G_END_DECLS
