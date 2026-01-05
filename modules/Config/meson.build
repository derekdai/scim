# modules/Config/meson.build

if get_option('config-simple')
  shared_module('simple',
    'scim_simple_config.cpp',
    dependencies : libscim_dep,
    install : true,
    install_dir : join_paths(scim_module_path, 'Config')
  )
endif

if get_option('config-socket') and socket_ok
  shared_module('socket',
    'scim_socket_config.cpp',
    dependencies : libscim_dep,
    install : true,
    install_dir : join_paths(scim_module_path, 'Config')
  )
endif
