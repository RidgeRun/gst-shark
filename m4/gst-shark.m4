dnl AG_GST_SHARK_INIT
dnl sets up use of GstD configure.ac macros
dnl all GstD autoconf macros are prefixed
dnl with AG_GST_SHARK_ for public macros
dnl with _AG_GST_SHARK_ for private macros
dnl

AC_DEFUN([AG_GST_SHARK_INIT],
[
  m4_pattern_forbid(^_?AG_GST_SHARK_)

  dnl FIXME: find a better way to do this
  GST_SHARK_README_LONG=`sed -En '/^## Description/,/^#/{ /^## Description/{n;n;}; /^#/q; p;}' README | tr '\n' ' '`
  GST_SHARK_README_SHORT=`sed -En 's/^> (.*)$/\1/p' README | tr '\n' ' '`

  GST_SHARK_PACKAGE_CREATED=2016-01-29
  GST_SHARK_PACKAGE_MAILINGLIST=

  GST_SHARK_REPO_DOWNLOAD=
  GST_SHARK_REPO_LOCATION=git@github.com:RidgeRun/gst-shark.git
  GST_SHARK_REPO_BROWSE=https://github.com/RidgeRun/gst-shark

  dnl Make everything public
  AC_SUBST([GST_SHARK_README_LONG])
  AC_SUBST([GST_SHARK_README_SHORT])
  AC_SUBST([GST_SHARK_PACKAGE_MAILINGLIST])
  AC_SUBST([GST_SHARK_PACKAGE_CREATED])
  AC_SUBST([GST_SHARK_REPO_DOWNLOAD])
  AC_SUBST([GST_SHARK_REPO_LOCATION])
  AC_SUBST([GST_SHARK_REPO_BROWSE])

  AC_DEFINE_UNQUOTED([GST_SHARK_README_LONG], "$GST_SHARK_README_LONG", GstShark long description)
  AC_DEFINE_UNQUOTED([GST_SHARK_README_SHORT], "$GST_SHARK_README_SHORT", GstShark short description)
  AC_DEFINE_UNQUOTED([GST_SHARK_PACKAGE_MAILINGLIST], "$GST_SHARK_PACKAGE_MAILINGLIST", GstShark mailing list address)
  AC_DEFINE_UNQUOTED([GST_SHARK_PACKAGE_CREATED], "$GST_SHARK_PACKAGE_CREATED", GstShark date of creation)
  AC_DEFINE_UNQUOTED([GST_SHARK_REPO_DOWNLOAD], "$GST_SHARK_REPO_DOWNLOAD", GstShark download address)
  AC_DEFINE_UNQUOTED([GST_SHARK_REPO_LOCATION], "$GST_SHARK_REPO_LOCATION", GstShark git repo)
  AC_DEFINE_UNQUOTED([GST_SHARK_REPO_BROWSE], "$GST_SHARK_REPO_BROWSE", GstShark read-only git repo)

  AC_MSG_NOTICE([Initialized $PACKAGE_NAME])
])
