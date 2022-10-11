project('gst-shark', 'c',
  version : '0.7.5.1',
  meson_version : '>= 0.50',)

gst_shark_version = meson.project_version()
version_arr = gst_shark_version.split('.')
gst_shark_version_major = version_arr[0].to_int()
gst_shark_version_minor = version_arr[1].to_int()
gst_shark_version_micro = version_arr[2].to_int()
gst_shark_version_nano = version_arr[3].to_int()

# Find external dependencies
gst_dep       = dependency('gstreamer-1.0',      version : '>=1.8.0')
gst_base_dep  = dependency('gstreamer-base-1.0', version : '>=1.0.0')
gst_check_dep = dependency('gstreamer-check-1.0',version : '>=1.0.5')

cc = meson.get_compiler('c')

## Dependencies
# Define gst shark dependencies
gst_shark_deps = [gst_dep]
# Define test dependencies
test_gst_shark_deps=[gst_base_dep, gst_check_dep]

# Define header directories
gst_shark_inc_dir = include_directories('plugins/tracers')

# Define compiler args and include directories
gst_c_args = ['-DHAVE_CONFIG_H']
configinc = include_directories('.')

# Define gstreamer API version
apiversion = '1.0'

# Define installation directories
prefix = get_option('prefix')
lib_install_dir = join_paths(get_option('prefix'), get_option('libdir'))


# Get an object returns describing a compiler
cc = meson.get_compiler('c')

# Verify if the warning flags are available in the compiler
# If the flags is availale for the compiler it wiil be used in all compiler
# invocations with the exception of compile tests.
warning_flags = [
  '-Wmissing-declarations',
  #'-Wmissing-prototypes',
  '-Wredundant-decls',
  '-Wundef',
  '-Wwrite-strings',
  '-Wformat',
  #'-Wformat-nonliteral',
  '-Wformat-security',
  #'-Wold-style-definition',
  '-Winit-self',
  '-Wmissing-include-dirs',
  '-Waddress',
  '-Waggregate-return',
  '-Wno-multichar',
  #'-Wdeclaration-after-statement',
  #'-Wvla',
  '-Wpointer-arith',
]
foreach extra_arg : warning_flags
  if cc.has_argument (extra_arg)
    # Add flag to the compiler command line
    add_project_arguments([extra_arg], language: 'c')
  endif
endforeach

# Create an empty configuration object to set config.h information
cdata = configuration_data()

pkg_gst_shark_url='https://developer.ridgerun.com/wiki/index.php?title=GStreamer_Daemon'
# Set config.h information
cdata.set_quoted('GST_API_VERSION', apiversion)
cdata.set_quoted('LOCALEDIR', join_paths(prefix, get_option('localedir')))
cdata.set_quoted('LIBDIR', join_paths(prefix, get_option('libdir')))
cdata.set_quoted('GST_API_VERSION', '1.0')
cdata.set_quoted('GST_LICENSE', 'LGPL')
cdata.set_quoted('PACKAGE', 'gst-shark')
cdata.set_quoted('PACKAGE_NAME', 'gst-shark')
cdata.set_quoted('PACKAGE_STRING', 'gst-shark @0@'.format(gst_shark_version))
cdata.set_quoted('PACKAGE_TARNAME', 'gst-shark')
cdata.set_quoted('PACKAGE_BUGREPORT', ' https://github.com/RidgeRun/gst-shark.git')
cdata.set_quoted('PACKAGE_URL', pkg_gst_shark_url)
cdata.set_quoted('PACKAGE_VERSION', gst_shark_version)
cdata.set_quoted('PLUGINDIR', lib_install_dir)
cdata.set_quoted('VERSION', gst_shark_version)

if gst_shark_version_nano > 0
    # Have GST_ERROR message printed when running from git
    cdata.set('GST_LEVEL_DEFAULT', 'GST_LEVEL_ERROR')
else
    cdata.set('GST_LEVEL_DEFAULT', 'GST_LEVEL_NONE')
endif

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
  if h.get(0) == host_machine.cpu()
    cdata.set(h.get(1), 1)
  else
    cdata.set(h.get(1), false)
  endif
endforeach

cdata.set_quoted('HOST_CPU', host_machine.cpu())

# Verify if the specified header exists
check_headers = [
  'dlfcn.h',
  'inttypes.h',
  'memory.h',
  'poll.h',
  'stdint.h',
  'stdlib.h',
  'stdio_ext.h',
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
    cdata.set(define, 1)
  endif
endforeach

# Gtk documentation
gnome = import('gnome')

# Imports pkgconfig module
pkgconfig = import('pkgconfig')

# Meson will generate a header file all the entries in the configuration data object
configure_file(output : 'config.h', configuration : cdata)

# Enter to each subdirectory and execute the meson.build
subdir('plugins')