lib_LTLIBRARIES = libgstshark-@GST_SHARK_API_VERSION@.la

libgstshark_@GST_SHARK_API_VERSION@_la_SOURCES = \
	gstdot.c \
	gstcpuusagecompute.c \
	gstproctimecompute.c \
	gstctf.c

libgstshark_@GST_SHARK_API_VERSION@_la_CFLAGS = \
	$(GST_SHARK_OBJ_CFLAGS) \
	$(GST_CFLAGS) \
	$(GVC_CFLAGS)  \
	-DGST_USE_UNSTABLE_API \
	-Wno-undef

libgstshark_@GST_SHARK_API_VERSION@_la_LIBADD = \
	$(GST_SHARK_OBJ_LIBS) \
	$(GST_LIBS) \
	$(GVC_LIBS)

plugin_LTLIBRARIES = libgstsharktracers-@GST_SHARK_API_VERSION@.la

libgstsharktracers_@GST_SHARK_API_VERSION@_la_SOURCES = \
	gstsharktracers.c \
	gstgraphic.c \
	gstcpuusage.c \
	gstproctime.c \
	gstinterlatency.c \
    gstscheduletime.c \
	gstframerate.c

libgstsharktracers_@GST_SHARK_API_VERSION@_la_CFLAGS = \
	$(GST_SHARK_OBJ_CFLAGS) \
	$(GST_CFLAGS) \
	-DGST_USE_UNSTABLE_API

libgstsharktracers_@GST_SHARK_API_VERSION@_la_LIBADD = \
	$(GST_SHARK_OBJ_LIBS) \
	$(GST_LIBS) \
	$(top_builddir)/plugins/tracers/libgstshark-@GST_SHARK_API_VERSION@.la 

libgstsharktracers_@GST_SHARK_API_VERSION@_la_LDFLAGS = \
	$(GST_SHARK_PLUGIN_LDFLAGS)

libgstsharktracers_@GST_SHARK_API_VERSION@_la_LIBTOOLFLAGS = \
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
	gstctf.h

CLEANFILES = *.gcno *.gcda *.gcov *.gcov.out

%.c.gcov: .libs/libgstshark_@GST_SHARK_API_VERSION@_la-%.gcda %.c
	$(GCOV) -b -f -o $^ > $@.out

gcov: $(libgstshark_@GST_SHARK_API_VERSION@_la_SOURCES:=.gcov)

