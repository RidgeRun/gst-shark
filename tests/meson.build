if get_option('enable-tests')

  # Examples tests
  message('Building example tests')
  subdir('examples')

  # Check tests, require gst-check
  if get_option('enable-check')
    if gst_check_dep.found()
      message('Building check tests')
      subdir('check')
    else
      warning('Check dependency is not found, please install it or disable check test with -Denable-check=false')
    endif
  endif

else
  message('Not building tests')
endif
