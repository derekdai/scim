# modules/FrontEnd/meson.build

# Socket FrontEnd
if get_option('frontend-socket') and socket_ok
  shared_module('socket-frontend',
    'scim_socket_frontend.cpp',
    dependencies : libscim_dep,
    install : true,
    install_dir : join_paths(scim_module_path, scim_binary_version, 'FrontEnd')
  )
endif

# X11 FrontEnd
if get_option('frontend-x11') and x11_dep.found()
  # IMdkit library needed for X11 frontend
  subdir('IMdkit')

  shared_module('x11-frontend',
    ['scim_x11_frontend.cpp', 'scim_x11_ic.cpp'],
    dependencies : [libscim_dep, x11_dep, imdkit_dep, libscim_x11utils_dep],
    install : true,
    install_dir : join_paths(scim_module_path, scim_binary_version, 'FrontEnd')
  )
endif

# IBus FrontEnd (skipped as IBus deps not fully checked in root yet, and likely not present)
