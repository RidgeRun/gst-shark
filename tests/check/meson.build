# Based on the test: https://github.com/GStreamer/gst-rtsp-server/blob/master/tests/check/meson.build

# Tests with filename, condition when to skip the test, and link libraries
gstd_tests = [
  ['gst-shark/gstdot.c', false, [gst_shark_lib, gst_shark_tracers_plugins]],
]

# Add C Definitions for tests
test_defines = [
  '-DTESTFILE="' + meson.current_source_dir() + '/meson.build"'
]

plugins_dir = []
# Define plugins path
if gst_dep.type_name() == 'pkgconfig'
   plugins_dir =[gst_dep.get_variable(pkgconfig : 'pluginsdir')]
endif
# Add path to compiled plugins too
gst_shark_plugins_path = meson.global_build_root() + '/plugins/tracers'
plugins_dir += [gst_shark_plugins_path]

# Create environment object to  stores information about the environment
# variables set during tests.
# Define constant enviroment variable
env = environment()
env.set('GST_PLUGIN_SYSTEM_PATH_1_0', '')
env.set('CK_DEFAULT_TIMEOUT', '120')
env.set('GST_PLUGIN_PATH_1_0', plugins_dir)

# gst_ctf_init function in the plugin load is not being called, hence something results in NULL? This gets called by exporting the GST_TRACERS
# with a type of trace from the plugin: plugins/tracers/gstplugin.c. TODO: Fix this.
env.set('GST_TRACERS', 'bitrate')

# Build and run tests
foreach t : gstd_tests
  fname = t[0]
  test_name = fname.split('.')[0].underscorify()
  skip_test = t.get(1, false)
  link_with_libs = t.get(2, [])

  if not skip_test
    # Creates a new executable for each test
    exe = executable(test_name, fname,
        c_args : gst_c_args + test_defines,
        cpp_args : gst_c_args + test_defines,
        include_directories : [configinc, gst_shark_inc_dir],
        link_with : link_with_libs,
        dependencies : [test_gst_shark_deps],
    )

    # Define enviroment variable
    env.set('GST_REGISTRY', '@0@/@1@.registry'.format(meson.current_build_dir(), test_name))

    # Run tests
    test(test_name, exe, env: env, timeout : 60)
  endif
endforeach
