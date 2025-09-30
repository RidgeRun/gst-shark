#
# Copyright (C) RidgeRun 2021
# Authors:  Michael Gruner <michael.gruner@ridgerun.com>
#           Carlos Aguero <carlos.aguero@ridgerun.com>
#           Sebastian Fatjo <sebastian.fatjo@ridgerun.com>
#           Manuel Leiva <manuel.leiva@ridgerun.com>
#           Carlos Rodriguez <carlos.rodriguez@ridgerun.com>
#           Roberto Gutierrez <roberto.gutierrez@ridgerun.com>

project('gst-shark', 'c',
  version : '0.9.0',
  meson_version : '>= 0.58',)

# Project version
gst_shark_version = meson.project_version()
version_arr = gst_shark_version.split('.')
gst_shark_version_major = version_arr[0].to_int()
gst_shark_version_minor = version_arr[1].to_int()
gst_shark_version_micro = version_arr[2].to_int()

if version_arr.length() == 4
  gst_shark_version_nano = version_arr[3].to_int()
else
  gst_shark_version_nano = 0
endif

gst_shark_version_short = f'@gst_shark_version_major@.@gst_shark_version_minor@.@gst_shark_version_micro@'

# Create an empty configuration object to set config.h information
cdata = configuration_data()

# Get an object returns describing a compiler
cc = meson.get_compiler('c')

# Define header directories
gst_shark_inc_dir = include_directories('plugins/tracers')

# Define compiler args and include directories
gst_c_args = ['-DHAVE_CONFIG_H']
configinc = include_directories('.')

# Find external dependencies
glib_dep      = dependency('glib-2.0',           version : '>=2.44.0', required : true)
gst_dep       = dependency('gstreamer-1.0',      version : '>=1.8.1', required : true)
gst_base_dep  = dependency('gstreamer-base-1.0', version : '>=1.0.0', required : true)
xmllib_dep    = dependency('libxml-2.0',         version : '>=1.0.0', required : true)
gio_dep       = dependency('gio-2.0',            version : '>=1.0.0', required : true)
gst_check_dep = dependency('gstreamer-check-1.0',version : '>=1.0.5', required : false)
gvc_dep       = dependency('libgvc',             version : '>=2.38', required : get_option('enable-plotting'))

## Dependencies
# Define gst shark lib main dependencies
gst_shark_deps = [glib_dep, xmllib_dep, gst_dep, gio_dep, gvc_dep]
# Define gst shark tracer main dependencies
gst_shark_tracers_deps = [glib_dep, xmllib_dep, gst_dep, gvc_dep]
# Define test main dependencies
test_gst_shark_deps = [gst_base_dep, gst_check_dep, gst_dep, glib_dep]

# PLotting enablement
if gvc_dep.found() # and get_option('enable-plotting')
  cdata.set('ENABLE_GRAPHVIZ', 1, description: 'Define this to enable plotting capabilities')
  message('plotting enabled')
else
  warning('plotting disabled, if required, install Graphviz and enable it with -Denable-plotting=enabled')
endif

# Define gstreamer API version
gstreamer_api_version = '1.0'

# Define installation directories
prefix = get_option('prefix')
lib_install_dir = join_paths(prefix, get_option('libdir'))
plugins_install_dir = join_paths(lib_install_dir, 'gstreamer-1.0')

# Verify if the warning flags are available in the compiler
# If the flags is availale for the compiler it wiil be used in all compiler
# invocations with the exception of compile tests.
warning_flags = [
  '-Wmissing-declarations',
  '-Wredundant-decls',
  '-Wundef',
  '-Wwrite-strings',
  '-Wformat',
  '-Wformat-security',
  '-Winit-self',
  '-Wmissing-include-dirs',
  '-Waddress',
  '-Waggregate-return',
  '-Wno-multichar',
  '-Wpointer-arith',
]
add_project_arguments(cc.get_supported_arguments(warning_flags), language: 'c')

##############################
## Set config.h information
gst_shark_repo = 'https://github.com/RidgeRun/gst-shark'
cdata.set_quoted('GST_API_VERSION', gstreamer_api_version, description: 'GStreamer API version')
cdata.set_quoted('GST_SHARK_API_VERSION', f'@gst_shark_version_major@.0', description : 'GstShark API Version')
cdata.set_quoted('GST_SHARK_DATADIR', join_paths(prefix, get_option('datadir')), description: 'data dir')
cdata.set_quoted('GST_SHARK_LICENSE', 'LGPL', description: 'GstShark license')
cdata.set_quoted('GST_SHARK_PACKAGE_CREATED', '2016-01-29', description: 'GstShark date of creation')
cdata.set_quoted('GST_SHARK_PACKAGE_MAILINGLIST', '', description: 'GstShark mailing list address')
cdata.set_quoted('GST_SHARK_README_LONG', 'GstShark leverages GStreamers newest instrumentation subsystem by installing a set of custom hooks to the trace points. A variety of data analytics are extracted and presented to the user in a graphical, friendly way.  ', description: 'GstShark long description')
cdata.set_quoted('GST_SHARK_README_SHORT', 'A Front End for GstShark', description: 'GstShark short description')
cdata.set_quoted('GST_SHARK_REPO_BROWSE', gst_shark_repo, description: 'GstShark read-only git repo')
cdata.set_quoted('GST_SHARK_REPO_DOWNLOAD', '', description: 'GstShark download address')
cdata.set_quoted('GST_SHARK_REPO_LOCATION', 'git@github.com:RidgeRun/gst-shark.git', description: 'GstShark git repo')
cdata.set_quoted('LIBDIR', join_paths(prefix, get_option('libdir')))
cdata.set_quoted('LOCALEDIR', join_paths(prefix, get_option('localedir')))
cdata.set_quoted('PACKAGE_BUGREPORT', gst_shark_repo, description: 'Define to the address where bug reports for this package should be sent.')
cdata.set_quoted('PACKAGE_NAME', 'GstShark', description: 'Define to the full name of this package.')
cdata.set_quoted('PACKAGE_STRING', 'GstShark @0@'.format(gst_shark_version), description: 'Define to the full name and version of this package.')
cdata.set_quoted('PACKAGE_TARNAME', 'gst-shark', description: 'gst-shark')
cdata.set_quoted('PACKAGE_URL', gst_shark_repo, description: 'Define to the home page for this package.')
cdata.set_quoted('PACKAGE_VERSION', gst_shark_version, description: 'Define to the version of this package.')
cdata.set_quoted('PACKAGE', 'gst-shark', description: 'Name of package')
cdata.set_quoted('PLUGINDIR', plugins_install_dir, description: 'Directory where plugins are located')
cdata.set_quoted('VERSION', gst_shark_version, description: 'Version number of package')

if gst_shark_version_nano > 0
    # Have GST_ERROR message printed when running from git
    cdata.set('GST_LEVEL_DEFAULT', 'GST_LEVEL_ERROR', description : 'Default errorlevel to use')
else
    cdata.set('GST_LEVEL_DEFAULT', 'GST_LEVEL_NONE', description : 'Default errorlevel to use')
endif

cdata.set('GST_SHARK_VERSION_MAJOR', gst_shark_version_major, description: 'GstShark Version Major')
cdata.set('GST_SHARK_VERSION_MINOR', gst_shark_version_minor, description: 'GstShark Version Minor')
cdata.set('GST_SHARK_VERSION_MICRO', gst_shark_version_micro, description: 'GstShark Version Micro')
cdata.set('GST_SHARK_VERSION_NANO', gst_shark_version_nano, description: 'GstShark Version Nano')

# GStreamer package name and origin url
gst_package_name = get_option('package-name')
if gst_package_name == ''
  if gst_shark_version_nano == 0
    gst_package_name = 'GstShark source release'
  elif gst_shark_version_nano == 1
    gst_package_name = 'GStreamer git'
  else
    gst_package_name = 'GStreamer prerelease'
  endif
endif
cdata.set_quoted('GST_PACKAGE_NAME', gst_package_name)
cdata.set_quoted('GST_PACKAGE_ORIGIN', get_option('package-origin'))

cdata.set('GST_STABLE_RELEASE', 1, description: 'Define this to use the tracers API of GStreamer version >= 1.8.1')

# Checking the OS to enable reading of CPU usage
if host_machine.system() == 'linux'
    cdata.set('GST_CPUUSAGE_ENABLE', 1, description : 'Define this to enable the tracer for reading of the CPU usage')
    message('Your platform supports CPU Usage tracing')
else
    warning('CPU Usage tracer currently not supported on your platform')
endif

# These are only needed/used by the ABI tests
host_defines = [
  [ 'x86', 'HAVE_CPU_I386' ],
  [ 'x86_64', 'HAVE_CPU_X86_64' ],
  [ 'arm', 'HAVE_CPU_ARM' ],
  [ 'aarch64', 'HAVE_CPU_AARCH64' ],
  [ 'mips', 'HAVE_CPU_MIPS' ],
  [ 'powerpc', 'HAVE_CPU_PPC' ],
  [ 'powerpc64', 'HAVE_CPU_PPC64' ],
  [ 'alpha', 'HAVE_CPU_ALPHA' ],
  [ 'sparc', 'HAVE_CPU_SPARC' ],
  [ 'ia64', 'HAVE_CPU_IA64' ],
  [ 'hppa', 'HAVE_CPU_HPPA' ],
  [ 'm68k', 'HAVE_CPU_M68K' ],
  [ 's390', 'HAVE_CPU_S390' ],
]
foreach h : host_defines
  cdata.set(h.get(1), h.get(0) == host_machine.cpu(), description : 'Define if the target CPU is ' + h.get(0).to_upper())
endforeach

cdata.set_quoted('HOST_CPU', host_machine.cpu())

# Verify if the specified header exists
check_headers = [
  'dlfcn.h',
  'inttypes.h',
  'memory.h',
  'poll.h',
  'stdint.h',
  'stdio_ext.h',
  'stdlib.h',
  'strings.h',
  'string.h',
  'sys/param.h',
  'sys/poll.h',
  'sys/prctl.h',
  'sys/socket.h',
  'sys/stat.h',
  'sys/times.h',
  'sys/time.h',
  'sys/types.h',
  'sys/utsname.h',
  'sys/wait.h',
  'ucontext.h',
  'unistd.h',
  'valgrind/valgrind.h',
  'sys/resource.h',
]

foreach h : check_headers
  if cc.has_header(h)
    define = 'HAVE_' + h.underscorify().to_upper()
    cdata.set(define, 1, description : f'Define to 1 if you have the <@h@> header file.')
  endif
endforeach
## Set config.h information done
##############################

# Gtk documentation
gnome = import('gnome')

# Imports pkgconfig module
pkgconfig = import('pkgconfig')

# Meson will generate a header file all the entries in the configuration data object
configure_file(output : 'config.h', configuration : cdata)

# Enter to each subdirectory and execute the meson.build
subdir('plugins')
subdir('tests')
subdir('docs')
