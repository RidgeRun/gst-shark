lib_LTLIBRARIES = libgstshark.la

libgstshark_la_SOURCES = \
	gstdot.c \
	gstcpuusagecompute.c \
	gstproctimecompute.c \
	gstctf.c \
	gstparser.c

libgstshark_la_CFLAGS = \
	$(GST_SHARK_OBJ_CFLAGS) \
	$(GST_CFLAGS) \
	$(GVC_CFLAGS)  \
	$(GIO_CFLAGS) \
	-DGST_USE_UNSTABLE_API \
	-Wno-undef

libgstshark_la_LIBADD = \
	$(GST_SHARK_OBJ_LIBS) \
	$(GST_LIBS) \
	$(GVC_LIBS) \
	$(GIO_LIBS)

plugin_LTLIBRARIES = libgstsharktracers.la

libgstsharktracers_la_SOURCES = \
	gstplugin.c \
	gstsharktracer.c \
	gstgraphic.c \
	gstcpuusage.c \
	gstproctime.c \
	gstinterlatency.c \
	gstscheduletime.c \
	gstframerate.c \
	gstqueuelevel.c \
	gstbitrate.c \
	gstbuffer.c \
	gstperiodictracer.c

libgstsharktracers_la_CFLAGS = \
	$(GST_SHARK_OBJ_CFLAGS) \
	$(GST_CFLAGS) \
	-DGST_USE_UNSTABLE_API

libgstsharktracers_la_LIBADD = \
	$(GST_SHARK_OBJ_LIBS) \
	$(GST_LIBS) \
	libgstshark.la

libgstsharktracers_la_LDFLAGS = \
	$(GST_SHARK_PLUGIN_LDFLAGS)

libgstsharktracers_la_LIBTOOLFLAGS = \
	$(GST_SHARK_PLUGIN_LIBTOOLFLAGS)

noinst_HEADERS = \
	gstdot.h \
	gstcpuusage.h \
	gstcpuusagecompute.h \
	gstgraphic.h \
	gstproctime.h \
	gstproctimecompute.h \
	gstinterlatency.h \
	gstscheduletime.h \
	gstframerate.h \
	gstctf.h \
	gstparser.h \
	gstqueuelevel.h \
	gstbitrate.h \
	gstbuffer.h \
	gstsharktracer.h \
	gstperiodictracer.h

CLEANFILES = *.gcno *.gcda *.gcov *.gcov.out

%.c.gcov: .libs/libgstshark_la-%.gcda %.c
	$(GCOV) -b -f -o $^ > $@.out

gcov: $(libgstshark_la_SOURCES:=.gcov)

