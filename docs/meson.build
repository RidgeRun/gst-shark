if get_option('enable-docs')
    docconf = configuration_data()
    docconf.set('GST_SHARK_API_VERSION', gstreamer_api_version)
    docconf.set('PACKAGE_VERSION', gst_shark_version)
    version_entities = configure_file(input : 'version.entities.in',
    output : 'version.entities',
    configuration : docconf)
endif
