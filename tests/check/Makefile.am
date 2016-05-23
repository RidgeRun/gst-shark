include $(top_srcdir)/common/check.mak

CHECK_REGISTRY = $(top_builddir)/tests/check/test-registry.reg

REGISTRY_ENVIRONMENT = \
	GST_REGISTRY=$(CHECK_REGISTRY)

AM_TESTS_ENVIRONMENT = \
	CK_DEFAULT_TIMEOUT=120                                  \
	$(REGISTRY_ENVIRONMENT)					\
	GST_PLUGIN_SYSTEM_PATH_1_0=				\
	GST_PLUGIN_PATH_1_0=$(top_builddir)/plugins/tracers:$(GST_PLUGINS_DIR) \
	GST_PLUGIN_LOADING_WHITELIST="gstreamer:gst-sharktracers@$(top_builddir)"


# the core dumps of some machines have PIDs appended, test registry and
# profiling data
CLEANFILES = core core.* test-registry.* *.gcno *.gcda

SUPPRESSIONS = $(top_srcdir)/common/gst.supp

clean-local: clean-local-check

$(CHECK_REGISTRY):
	$(AM_TESTS_ENVIRONMENT)

check_PROGRAMS = gstdot

# failing tests
noinst_PROGRAMS =

TESTS = $(check_PROGRAMS)

noinst_HEADERS =

AM_CFLAGS = \
	$(GST_SHARK_OBJ_CFLAGS) \
	$(GST_CFLAGS) \
	$(GST_CHECK_FLAGS) \
	-I$(top_srcdir)/plugins/tracers 
LDADD =	\
	$(top_builddir)/plugins/tracers/libgstshark.la \
	$(GST_SHARK_OBJ_LIBS) \
	$(GST_LIBS) \
	$(GST_CHECK_LIBS)

gstdot_SOURCES = gst-shark/gstdot.c

# valgrind testing
# these just need valgrind fixing, period
VALGRIND_TO_FIX = 

VALGRIND_IGNORE = 

# these need fixing because the threads cause segfaults under valgrind
TESTS_THREADED =

VALGRIND_TESTS_DISABLE = 					\
	$(TESTS_THREADED)					\
	$(VALGRIND_IGNORE)					\
	$(VALGRIND_TO_FIX)

# indexers does not get tested yet
COVERAGE_DIRS = plugins/tracers

COVERAGE_FILES = $(foreach dir,$(COVERAGE_DIRS),$(wildcard $(top_builddir)/$(dir)/*.gcov))
COVERAGE_FILES_REL = $(subst $(top_builddir)/,,$(COVERAGE_FILES))
COVERAGE_OUT_FILES = $(foreach dir,$(COVERAGE_DIRS),$(wildcard $(top_builddir)/$(dir)/*.gcov.out))
COVERAGE_OUT_FILES_REL = $(subst $(top_builddir)/,,$(COVERAGE_OUT_FILES))

debug:
	echo $(GST_PLUGINS_DIR)
	echo $(COVERAGE_FILES)
	echo $(COVERAGE_FILES_REL)

.PHONY: coverage
if GST_GCOV_ENABLED
# we rebuild a registry and do gst-inspect so that all the get/set codepaths
# are also covered
coverage:
	for file in `find $(top_builddir) -name '*.gcda'`; do rm $$file; done
	-rm $(CHECK_REGISTRY)
	echo "Inspecting all elements"
	for e in `$(GST_INSPECT) | head -n -2 | cut -d: -f2`; do $(GST_INSPECT) $$e > /dev/null 2>&1; done
	make check
	make coverage-report
else
coverage:
	echo "You need to configure with --enable-gcov to get coverage data"
	exit 1
endif

coverage-report:
	rm -r coverage
	for dir in $(COVERAGE_DIRS); do 				\
	  mkdir -p coverage/$$dir; 					\
	  make -C $(top_builddir)/$$dir gcov;				\
        done
	for dir in $(COVERAGE_DIRS); do					\
	    files="`ls $(top_builddir)/$$dir/*.gcov.out 2> /dev/null`"; \
          if test ! -z "$$files"; then					\
	    perl $(top_srcdir)/common/coverage/coverage-report.pl 	\
	      $(top_builddir)/$$dir/*.gcov.out >			\
	      coverage/$$dir/index.xml;					\
	    xsltproc $(top_srcdir)/common/coverage/coverage-report.xsl	\
	      coverage/$$dir/index.xml > coverage/$$dir/index.html;	\
	  fi; \
        done
	for file in $(COVERAGE_FILES_REL); do				\
	  echo Generating coverage/$$file.html; 			\
	  perl $(top_srcdir)/common/coverage/coverage-report-entry.pl   \
	    $(top_builddir)/$$file > coverage/$$file.html; 		\
	done
