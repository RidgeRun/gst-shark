noinst_PROGRAMS = dotviewer

AM_CFLAGS = \
	$(GST_CFLAGS) \
	$(GVC_CFLAGS) \
	-I$(top_srcdir)/plugins/tracers

LDADD = \
	$(GST_LIBS) \
	$(GVC_LIBS) \
	$(top_builddir)/plugins/tracers/libgstshark.la
