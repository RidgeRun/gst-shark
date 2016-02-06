if ENABLE_DOCBOOK
SUBDIRS_DOCBOOK = 
else
SUBDIRS_DOCBOOK =
endif

if ENABLE_GTK_DOC
SUBDIRS_GTK_DOC = 
else
SUBDIRS_GTK_DIC = 
endif

BUILT_SOURCES = version.entities

SUBDIRS = $(SUBDIRS_GTK_DOC) $(SUBDIRS_DOCBOOK)
DIST_SUBDIRS = $(SUBDIRS)

EXTRA_DIST = version.entities.in

upload:
	@if test "x$(SUBDIRS)" != x; then for a in $(SUBDIRS); do cd $$a; make upload; cd ..; done; fi

include $(top_srcdir)/common/parallel-subdirs.mak
