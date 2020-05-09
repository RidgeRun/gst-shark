dnl required version of autoconf
AC_PREREQ([2.69])

dnl initialize autoconf
dnl when going to/from release please set the nano (fourth number) right !
dnl releases only do Wall, git and prerelease does Werror too
dnl
AC_INIT([GstShark],
        [0.6.2.1],
	[https://github.com/RidgeRun/gst-shark],
	[gst-shark],
	[https://github.com/RidgeRun/gst-shark])
AG_GST_SHARK_INIT

dnl initialize automake (we require GNU make)
AM_INIT_AUTOMAKE([-Wno-portability 1.14 no-dist-gzip dist-xz tar-ustar subdir-objects])

dnl define PACKAGE_VERSION_* variables
AS_VERSION

dnl check if this is a release version
AS_NANO(GST_GIT="no", GST_GIT="yes")

dnl can autoconf find the source ?
AC_CONFIG_SRCDIR([plugins/tracers/gstplugin.c])

dnl define the output header for config
AC_CONFIG_HEADERS([config.h])

dnl AM_MAINTAINER_MODE only provides the option to configure to enable it
AM_MAINTAINER_MODE([enable])

dnl sets host_* variables
AC_CANONICAL_HOST

dnl use pretty build output by default
AM_SILENT_RULES([yes])

dnl GstShark versioning, this is mostly informational
GST_SHARK_VERSION_MAJOR=$PACKAGE_VERSION_MAJOR
GST_SHARK_VERSION_MINOR=$PACKAGE_VERSION_MINOR
GST_SHARK_VERSION_MICRO=$PACKAGE_VERSION_MICRO
GST_SHARK_VERSION_NANO=$PACKAGE_VERSION_NANO
AC_SUBST(GST_SHARK_VERSION_MAJOR)
AC_SUBST(GST_SHARK_VERSION_MINOR)
AC_SUBST(GST_SHARK_VERSION_MICRO)
AC_SUBST(GST_SHARK_VERSION_NANO)

dnl GstShark versioning, for code usage
AC_DEFINE_UNQUOTED(GST_SHARK_VERSION_MAJOR, $GST_SHARK_VERSION_MAJOR, GstShark Version Major)
AC_DEFINE_UNQUOTED(GST_SHARK_VERSION_MINOR, $GST_SHARK_VERSION_MINOR, GstShark Version Minor)
AC_DEFINE_UNQUOTED(GST_SHARK_VERSION_MICRO, $GST_SHARK_VERSION_MICRO, GstShark Version Micro)
AC_DEFINE_UNQUOTED(GST_SHARK_VERSION_NANO, $GST_SHARK_VERSION_NANO, GstShark Version Nano)

dnl our libraries and install dirs use GST_SHARK_API_VERSION in the filename
dnl to allow side-by-side installation of different API versions
GST_SHARK_API_VERSION=$PACKAGE_VERSION_MAJOR.0
AC_SUBST(GST_SHARK_API_VERSION)
AC_DEFINE_UNQUOTED(GST_SHARK_API_VERSION, "$GST_SHARK_API_VERSION",
  [GstShark API Version])

dnl CURRENT, REVISION, AGE
dnl - library source changed -> increment REVISION
dnl - interfaces added/removed/changed -> increment CURRENT, REVISION = 0
dnl - interfaces added -> increment AGE
dnl - interfaces removed -> AGE = 0
dnl
dnl Keep CURRENT as MINOR * 100 + MICRO
dnl Ex : 1.0.0 => 0
dnl      1.0.3 => 3
dnl      1.1.0 => 100
dnl      1.2.5 => 205
dnl      1.10.9 (who knows) => 1009
dnl
dnl sets GST_LT_LDFLAGS
_GST_SHARK_CURRENT=$(($PACKAGE_VERSION_MINOR*100 + $PACKAGE_VERSION_MICRO))
AS_LIBTOOL(GST, [$_GST_SHARK_CURRENT], 0, [$_GST_SHARK_CURRENT])

dnl *** autotools stuff ****

dnl allow for different autotools
AS_AUTOTOOLS_ALTERNATE

dnl Add parameters for aclocal (keep in sync with Makefile.am)
AC_SUBST(ACLOCAL_AMFLAGS, "-I m4 -I common/m4")

dnl *** check for arguments to configure ***

AG_GST_ARG_DISABLE_FATAL_WARNINGS
AG_GST_ARG_ENABLE_EXTRA_CHECKS

dnl Additional configure options inherited from GStreamer
AG_GST_ARG_DEBUG
AG_GST_ARG_PROFILING
AG_GST_ARG_VALGRIND
AG_GST_ARG_GCOV
AG_GST_ARG_EXAMPLES
AG_GST_ARG_WITH_PKG_CONFIG_PATH
AG_GST_PKG_CONFIG_PATH

dnl build static plugins or not
AC_MSG_CHECKING([whether to build static plugins or not])
AC_ARG_ENABLE(
  static-plugins,
  AC_HELP_STRING(
    [--enable-static-plugins],
    [build static plugins @<:@default=no@:>@]),
  [AS_CASE(
    [$enableval], [no], [], [yes], [],
    [AC_MSG_ERROR([bad value "$enableval" for --enable-static-plugins])])],
  [enable_static_plugins=no])
AC_MSG_RESULT([$enable_static_plugins])
if test "x$enable_static_plugins" = xyes; then
  AC_DEFINE(GST_SHARK_PLUGIN_BUILD_STATIC, 1,
    [Define if static plugins should be built])
  GST_SHARK_PLUGIN_LIBTOOLFLAGS=""
else
  GST_SHARK_PLUGIN_LIBTOOLFLAGS="--tag=disable-static"
fi
AC_SUBST(GST_SHARK_PLUGIN_LIBTOOLFLAGS)
AM_CONDITIONAL(GST_SHARK_PLUGIN_BUILD_STATIC, test "x$enable_static_plugins" = "xyes")

dnl building of tests
AC_ARG_ENABLE(tests,
  AS_HELP_STRING([--disable-tests],[disable building test apps]),
  [
    case "${enableval}" in
      yes) BUILD_TESTS=yes ;;
      no)  BUILD_TESTS=no ;;
      *)   AC_MSG_ERROR(bad value ${enableval} for --disable-tests) ;;
    esac
  ],
[BUILD_TESTS=yes]) dnl Default value
AM_CONDITIONAL(BUILD_TESTS,         test "x$BUILD_TESTS" = "xyes")

dnl building of tools
AC_ARG_ENABLE(tools,
  AS_HELP_STRING([--disable-tools],[disable building tools]),
  [
    case "${enableval}" in
      yes) BUILD_TOOLS=yes ;;
      no)  BUILD_TOOLS=no ;;
      *)   AC_MSG_ERROR(bad value ${enableval} for --disable-tools) ;;
    esac
  ],
[BUILD_TOOLS=yes]) dnl Default value
AM_CONDITIONAL(BUILD_TOOLS, test "x$BUILD_TOOLS" = "xyes")

dnl *** checks for platform ***

dnl * hardware/architecture *

dnl common/m4/gst-arch.m4
dnl check CPU type
AG_GST_ARCH

dnl check for platform specific settings
AG_GST_PLATFORM

dnl * software *

dnl find a compiler
AC_PROG_CC
AC_PROG_CC_STDC

dnl check if the compiler supports '-c' and '-o' options
AM_PROG_CC_C_O

AC_PATH_PROG(VALGRIND_PATH, valgrind, no)
AM_CONDITIONAL(HAVE_VALGRIND, test ! "x$VALGRIND_PATH" = "xno")

dnl check for gobject-introspection
GOBJECT_INTROSPECTION_CHECK([1.31.1])

dnl check for documentation tools
AG_GST_DOCBOOK_CHECK
GTK_DOC_CHECK([1.12])
AG_GST_PLUGIN_DOCS([1.12])

dnl *** checks for libraries ***

dnl *** checks for header files ***

dnl check if we have ANSI C header files
AC_HEADER_STDC

dnl Check for valgrind.h
dnl separate from HAVE_VALGRIND because you can have the program, but not
dnl the dev package
AC_CHECK_HEADERS([valgrind/valgrind.h], [], [], [AC_INCLUDES_DEFAULT])

dnl *** checks for types/defines ***

dnl *** checks for structures ***

dnl *** checks for compiler characteristics ***

dnl *** checks for dependency libraries ***

dnl GLib
dnl GLib
GLIB_REQ=2.44.0
AG_GST_GLIB_CHECK([$GLIB_REQ])

dnl Check for documentation xrefs
GLIB_PREFIX="`$PKG_CONFIG --variable=prefix glib-2.0`"
AC_SUBST(GLIB_PREFIX)

dnl GStreamer
GST_REQ_MAJMIN=1.0
GST_REQ_MINVER=1.7.1
AG_GST_CHECK_GST([$GST_REQ_MAJMIN], [$GST_REQ_MINVER], [yes])
AG_GST_CHECK_GST_BASE([$GST_REQ_MAJMIN], [$GST_REQ_MINVER], [yes])
AG_GST_CHECK_GST_CHECK([$GST_REQ_MAJMIN], [$GST_REQ_MINVER], [yes])

dnl Tracers require GStreamer debug support
AG_GST_CHECK_GST_DEBUG_DISABLED([
  AC_MSG_ERROR([Tracers require GStreamer debug support. Rebuild GStreamer core with --enable-gst-debug.])
])

GST_REQ_STABLE=1.8.1
PKG_CHECK_MODULES(GSTREAMER, [
  gstreamer-1.0 >= $GST_REQ_STABLE
], [
  AC_DEFINE(GST_STABLE_RELEASE, 1, [Define this to use the tracers API of GStreamer version >= $GST_REQ_STABLE])
  AC_MSG_NOTICE([
    Version of GStreamer used is >= ($GST_REQ_STABLE)dnl
  ])
],[
  AC_MSG_WARN([
    Version of GStreamer is not the latest stable revision avialable ($GST_REQ_STABLE),
    Gst-Shark will work with the minimal required version ($GST_REQ_MINVER)dnl
  ])
])

dnl Graphviz
GVC_REQ=2.38
AC_ARG_ENABLE(graphviz,
    AS_HELP_STRING([--disable-graphviz],[Disable graphviz capabilities (Enabled by default)]))

if test "x$enable_graphviz" != "xno" ; then
    	PKG_CHECK_MODULES(GVC, [
            libgvc >= $GVC_REQ
	], [
	    AC_SUBST(GVC_CFLAGS)
	    AC_SUBST(GVC_LIBS)
        ],[
	    AC_MSG_ERROR([
	        Please install libgvc development packages (libgvc) or disable it
		with --disable-graphviz
            ])
	])
        AC_DEFINE(ENABLE_GRAPHVIZ, 1, [Define this to enable Graphviz capabilities])
fi

dnl Gio
GIO_REQ=2.0
PKG_CHECK_MODULES(GIO, [
  libgio >= $GIO_REQ
], [
  AC_SUBST(GIO_CFLAGS)
  AC_SUBST(GIO_LIBS)
],[
  AC_MSG_ERROR([
    Please install libgio development packages (libgio)
  ])
])

dnl building of unit test libraries
AC_ARG_ENABLE(check,
  AS_HELP_STRING([--disable-check],[disable building unit test libraries]),
  [
    case "${enableval}" in
      yes) BUILD_CHECK=yes ;;
      no)  BUILD_CHECK=no ;;
      *)   AC_MSG_ERROR(bad value ${enableval} for --disable-check) ;;
    esac
  ], [
    dnl Default value
    case $host_os in
      mingw* | msvc* | mks*) BUILD_CHECK=no ;;
      *) BUILD_CHECK=yes ;;
    esac
])

dnl checking the OS to enable reading of CPU usage
case $host_os in
  linux*)
    AC_DEFINE(GST_CPUUSAGE_ENABLE, 1, [Define this to enable the tracer for reading of the CPU usage])
    AC_MSG_NOTICE([
      Your platform supports CPU Usage tracing dnl
    ]) ;;
  *)
    AC_MSG_WARN([
      CPU Usage tracer currently not supported on your platform dnl
    ]) ;;
esac

dnl bit of a misnomer, but keep the conditional named like this so we don't
dnl have to change too much elsewhere
AM_CONDITIONAL(HAVE_CHECK, test "x$BUILD_CHECK" = "xyes")

dnl Check for -Bsymbolic-functions linker flag used to avoid
dnl intra-library PLT jumps, if available.
AC_ARG_ENABLE(Bsymbolic,
              [AS_HELP_STRING([--disable-Bsymbolic],[avoid linking with -Bsymbolic])],,
              [SAVED_LDFLAGS="${LDFLAGS}" SAVED_LIBS="${LIBS}"
               AC_MSG_CHECKING([for -Bsymbolic-functions linker flag])
               LDFLAGS=-Wl,-Bsymbolic-functions
               LIBS=
               AC_TRY_LINK([], [return 0],
                           AC_MSG_RESULT(yes)
                           enable_Bsymbolic=yes,
                           AC_MSG_RESULT(no)
                           enable_Bsymbolic=no)
               LDFLAGS="${SAVED_LDFLAGS}" LIBS="${SAVED_LIBS}"])

dnl *** set variables based on configure arguments

dnl set license and copyright notice
GST_SHARK_LICENSE="LGPL"
AC_DEFINE_UNQUOTED(GST_SHARK_LICENSE, "$GST_SHARK_LICENSE", [GstShark license])
AC_SUBST(GST_SHARK_LICENSE)

dnl define LIBDIR, GST_DATADIR so we can inform people where we live
AS_AC_EXPAND(LIBDIR, $libdir)
AC_DEFINE_UNQUOTED(LIBDIR, "$LIBDIR", [library dir])
AS_AC_EXPAND(DATADIR, $datadir)
AC_DEFINE_UNQUOTED(GST_SHARK_DATADIR, "$DATADIR", [data dir])

dnl set location of plugin directory
GST_API_VERSION=1.0
AG_GST_SET_PLUGINDIR

dnl define an ERROR_CFLAGS Makefile variable
AG_GST_SET_ERROR_CFLAGS($FATAL_WARNINGS, [-Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wundef -Wwrite-strings -Wformat-nonliteral -Wformat-security -Wold-style-definition -Winit-self -Wmissing-include-dirs -Waddress -Waggregate-return -Wno-multichar -Wnested-externs $NO_WARNINGS])

dnl special warning flags for gst/printf
AS_COMPILER_FLAG([-Wno-format-nonliteral], [PRINTF_CFLAGS="-Wno-format-nonliteral"])
AC_SUBST(PRINTF_CFLAGS)

dnl define correct level for debugging messages
AG_GST_SET_LEVEL_DEFAULT($GST_GIT)

dnl *** finalize CFLAGS, LDFLAGS, LIBS

dnl Overview:
dnl GST_SHARK_OPTION_CFLAGS:  common cflags for profiling, debugging, errors, ...
dnl GST_SHARK_ALL_*:          vars shared by all built objects
dnl GST_SHARK_LIB_LDFLAGS:    additional linker flags for all libaries
dnl GST_SHARK_OBJ_*:          additional vars to link to the core library
dnl			include GST_SHARK_ALL_*
dnl GST_SHARK_LT_LDFLAGS:	library versioning of our libraries

dnl GST_SHARK_OPTION_CFLAGS
if test "x$USE_DEBUG" = xyes; then
   PROFILE_CFLAGS="-g"
fi
AC_SUBST(PROFILE_CFLAGS)

# GST_SHARK_DISABLE_DEPRECATED: hide the visibility of deprecated
# functionality from the API that gstreamer uses
# GST_SHARK_REMOVE_DEPRECATED: don't compile deprecated functionality (breaks ABI)
if test "x$PACKAGE_VERSION_NANO" = "x1"; then
  dnl Define _only_ when compiling from git (not for pre-releases or releases)
  DEPRECATED_CFLAGS="-DGST_DISABLE_DEPRECATED"
else
  DEPRECATED_CFLAGS=""
fi
AC_SUBST(DEPRECATED_CFLAGS)

dnl every flag in GST_SHARK_OPTION_CFLAGS can be overridden at make time via e.g.
dnl make DEPRECATED_CFLAGS=''
GST_SHARK_OPTION_CFLAGS="\$(WARNING_CFLAGS) \$(DEBUG_CFLAGS) \$(PROFILE_CFLAGS) \$(GCOV_CFLAGS) \$(OPT_CFLAGS) \$(DEPRECATED_CFLAGS)"
GST_SHARK_OPTION_CFLAGS="$GST_SHARK_OPTION_CFLAGS"
AC_SUBST(GST_SHARK_OPTION_CFLAGS)

dnl GST_SHARK_ALL_*
dnl vars common to for all internal objects (core libs, elements, applications)
dnl CFLAGS:
dnl - src and build dirs need to be added because every piece that gets built
dnl   will need the GStreamer source and generated headers
dnl LIBS: XML doesn't need to be added because we don't explicitly use symbols
dnl       from LibXML except for in the core library
GST_SHARK_ALL_CFLAGS="-I\$(top_srcdir) -I\$(top_builddir) $GLIB_CFLAGS \$(GLIB_EXTRA_CFLAGS) $XML_CFLAGS \$(GST_SHARK_OPTION_CFLAGS) \$(ERROR_CFLAGS)"

dnl FIXME: check if LTLIBINTL is needed everywhere
dnl I presume it is given that it contains the symbols that _() stuff maps to
GST_SHARK_ALL_LIBS="$GLIB_LIBS $LTLIBINTL \$(GCOV_LIBS)"

dnl LDFLAGS really should only contain flags, not libs - they get added before
dnl whatevertarget_LIBS and -L flags here affect the rest of the linking
GST_SHARK_ALL_LDFLAGS="-no-undefined"
if test "x${enable_Bsymbolic}" = "xyes"; then
  GST_SHARK_ALL_LDFLAGS="$GST_SHARK_ALL_LDFLAGS -Wl,-Bsymbolic-functions"
fi

AC_SUBST(GST_SHARK_ALL_CFLAGS)
AC_SUBST(GST_SHARK_ALL_LIBS)
AC_SUBST(GST_SHARK_ALL_LDFLAGS)

dnl GST_SHARK_OBJ_*
dnl default vars for all internal objects built on libgstsharktracers
dnl includes GST_SHARK_ALL_*
GST_SHARK_OBJ_CFLAGS="\$(GST_SHARK_ALL_CFLAGS)"
GST_SHARK_OBJ_LIBS="\$(GST_SHARK_ALL_LIBS)"
AC_SUBST(GST_SHARK_OBJ_CFLAGS)
AC_SUBST(GST_SHARK_OBJ_LIBS)

dnl GST_SHARK_PLUGIN_LDFLAGS
dnl LDFLAGS for plugins; includes GST_SHARK_ALL_LDFLAGS
GST_SHARK_PLUGIN_LDFLAGS="-module -avoid-version -export-symbols-regex '^[_]*gst_plugin_.*' $GST_SHARK_ALL_LDFLAGS"
AC_SUBST(GST_SHARK_PLUGIN_LDFLAGS, "$GST_SHARK_PLUGIN_LDFLAGS")

dnl Finally output everything
AC_CONFIG_FILES([
Makefile
plugins/Makefile
plugins/tracers/Makefile
tests/Makefile
tests/check/Makefile
tests/examples/Makefile
tests/examples/dotviewer/Makefile
docs/Makefile
docs/version.entities
tools/Makefile
])

AC_OUTPUT

