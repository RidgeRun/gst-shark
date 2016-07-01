DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc --enable-docbook

# note: keep in sync with configure.ac
ACLOCAL_AMFLAGS=-I m4 -I common/m4

SUBDIRS= plugins \
         tests \
         docs

if BUILD_TOOLS
SUBDIRS += tools
endif

DIST_SUBDIRS = plugins \
	       tests \
               docs \
               tools

EXTRA_DIST = \
	configure.ac autogen.sh depcomp \
	MAINTAINERS RELEASE \
	gst-shark.doap gst-shark.doap.in

DISTCLEANFILES = _stdint.h

include $(top_srcdir)/common/release.mak

if GST_GCOV_ENABLED
clean-gcov:
	find -name "*.da" -o -name "*.gcov" | xargs rm || true

clean-bbg:
	find -name "*.bbg" -o -name "*.bb" | xargs rm || true

GCOV_DIRS=

## .PHONY so it always rebuilds it
.PHONY: coverage-report.txt

coverage-report.txt:
	BBG_FILES=`find $(GCOV_DIRS) -name "*.bbg"` ;			  \
	C_FILES= ;							  \
	for F in $$BBG_FILES ; do					  \
		F_nolibs=`echo $$F | sed -e 's/.libs\///g'` ;		  \
		C=`echo $$F_nolibs | sed -e 's/.bbg/.c/g'` ;		  \
		B=`basename $$F .bbg` ;					  \
		D=`dirname $$F` ;					  \
		DA=`echo $$F | sed -e 's/.bbg/.da/g'` ;			  \
		DA_libs=`echo $$D/.libs/$$B/.da` ;			  \
		if test -e $$DA || test -e $$DA_libs; then		  \
			C_FILES="$$C_FILES $$C" ;			  \
		fi ;							  \
	done ;								  \
	echo $$C_FILES ;						  \
	$(top_builddir)/testsuite/decode-gcov --report $$C_FILES > coverage-report.txt

check-coverage: clean-gcov all check coverage-report.txt
	cat coverage-report.txt

else
coverage-report.txt:
	echo "Need to reconfigure with --enable-gcov"

check-coverage:
	echo "Need to reconfigure with --enable-gcov"
endif

if HAVE_CHECK
check-valgrind:
	$(MAKE) -C tests/check check-valgrind

check-torture:
	$(MAKE) -C tests/check torture

build-checks:
	$(MAKE) -C tests/check build-checks
else
check-valgrind:
	echo "'check' library not installed, skipping"

check-torture:
	echo "'check' library not installed, skipping"

build-checks:
	echo "'check' library not installed, skipping"
endif

include $(top_srcdir)/common/coverage/lcov.mak

CRUFT_FILES = \
	$(top_builddir)/common/shave \
	$(top_builddir)/common/shave-libtool \
	$(top_builddir)/common/m4/codeset.m4 \
	$(top_builddir)/common/m4/gettext.m4 \
	$(top_builddir)/common/m4/glibc2.m4 \
	$(top_builddir)/common/m4/glibc21.m4 \
	$(top_builddir)/common/m4/iconv.m4 \
	$(top_builddir)/common/m4/intdiv0.m4 \
	$(top_builddir)/common/m4/intl.m4 \
	$(top_builddir)/common/m4/intldir.m4 \
	$(top_builddir)/common/m4/intlmacosx.m4 \
	$(top_builddir)/common/m4/intmax.m4 \
	$(top_builddir)/common/m4/inttypes-pri.m4 \
	$(top_builddir)/common/m4/inttypes_h.m4 \
	$(top_builddir)/common/m4/lcmessage.m4 \
	$(top_builddir)/common/m4/lib-ld.m4 \
	$(top_builddir)/common/m4/lib-link.m4 \
	$(top_builddir)/common/m4/lib-prefix.m4 \
	$(top_builddir)/common/m4/libtool.m4 \
	$(top_builddir)/common/m4/lock.m4 \
	$(top_builddir)/common/m4/longlong.m4 \
	$(top_builddir)/common/m4/ltoptions.m4 \
	$(top_builddir)/common/m4/ltsugar.m4 \
	$(top_builddir)/common/m4/ltversion.m4 \
	$(top_builddir)/common/m4/lt~obsolete.m4 \
	$(top_builddir)/common/m4/nls.m4 \
	$(top_builddir)/common/m4/po.m4 \
	$(top_builddir)/common/m4/printf-posix.m4 \
	$(top_builddir)/common/m4/progtest.m4 \
	$(top_builddir)/common/m4/size_max.m4 \
	$(top_builddir)/common/m4/stdint_h.m4 \
	$(top_builddir)/common/m4/uintmax_t.m4 \
	$(top_builddir)/common/m4/visibility.m4 \
	$(top_builddir)/common/m4/wchar_t.m4 \
	$(top_builddir)/common/m4/wint_t.m4 \
	$(top_builddir)/common/m4/xsize.m4

CRUFT_DIRS = 

include $(top_srcdir)/common/cruft.mak
