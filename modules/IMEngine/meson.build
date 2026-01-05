# modules/IMEngine/meson.build

# RawCode
if get_option('im-rawcode')
  shared_module('rawcode',
    'scim_rawcode_imengine.cpp',
    dependencies : libscim_dep,
    install : true,
    install_dir : join_paths(scim_module_path, scim_binary_version, 'IMEngine')
  )
endif

# Socket
if get_option('im-socket') and socket_ok
  shared_module('socket',
    'scim_socket_imengine.cpp',
    dependencies : libscim_dep,
    install : true,
    install_dir : join_paths(scim_module_path, scim_binary_version, 'IMEngine')
  )
endif
