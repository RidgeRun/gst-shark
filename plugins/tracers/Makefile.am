
plugin_LTLIBRARIES = libgstsharktracers.la

libgstsharktracers_la_DEPENDENCIES = 
libgstsharktracers_la_SOURCES = \
	gstsharktracers.c

libgstsharktracers_la_CFLAGS = $(GST_SHARK_OBJ_CFLAGS) $(GST_CFLAGS) \
  -DGST_USE_UNSTABLE_API
libgstsharktracers_la_LIBADD = $(GST_LIBS) \
	$(GST_SHARK_OBJ_LIBS)
libgstsharktracers_la_LDFLAGS = $(GST_SHARK_PLUGIN_LDFLAGS)
libgstsharktracers_la_LIBTOOLFLAGS = $(GST_SHARK_PLUGIN_LIBTOOLFLAGS)

noinst_HEADERS =

CLEANFILES = *.gcno *.gcda *.gcov *.gcov.out

%.c.gcov: .libs/libgstsharktracers_la-%.gcda %.c
	$(GCOV) -b -f -o $^ > $@.out

gcov: $(libgstsharktracers_la_SOURCES:=.gcov)

