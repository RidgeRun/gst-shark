gstctfbt2plugin=libgstctf.la
gstctfincludedir=$(includedir)/gstreamer-1.0/gst/ctf/

lib_LTLIBRARIES = $(gstctfbt2plugin)

libgstctf_la_SOURCES = \
        gstctf.c \
	gstctfcomponent.c \
	gstctfengine.c

libgstctf_la_CFLAGS = \
	$(GST_SHARK_OBJ_CFLAGS) \
	$(GST_CFLAGS) \
	$(BT2_CFLAGS) \
	-DGST_CTF_BT2_PLUGIN=\"$(gstctfbt2plugin)\"

libgstctf_la_LIBADD = \
	$(GST_SHARK_OBJ_LIBS) \
	$(GST_LIBS) \
        $(BT2_LIBS)

gstctfinclude_HEADERS = \
        gstctf.h \
	gstctfcomponent.h \
	gstctfengine.h

