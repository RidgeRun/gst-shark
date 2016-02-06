EXTRA_DIST = README

if BUILD_TESTS
if HAVE_CHECK
SUBDIRS_CHECK = check
else
SUBDIRS_CHECK =
endif
else
SUBDIRS_CHECK =
endif

if BUILD_EXAMPLES
SUBDIRS_EXAMPLES = examples
else
SUBDIRS_EXAMPLES =
endif

SUBDIRS = \
	$(SUBDIRS_CHECK) \
	$(SUBDIRS_EXAMPLES)

# These are all the possible subdirs
DIST_SUBDIRS = \
	check \
	examples
