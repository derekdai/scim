if get_option('im-rawcode')
  shared_module('rawcode',
    'scim_rawcode_imengine.cpp',
    dependencies: [libscim_dep],
    install: true,
    install_dir: join_paths(scim_module_path, 'IMEngine'),
    name_prefix: '',
    cpp_args: [
      '-DSCIM_DATADIR="@0@"'.format(scim_datadir),
      '-DSCIM_LOCALEDIR="@0@"'.format(scim_localedir),
      '-DSCIM_SYSCONFDIR="@0@"'.format(scim_sysconfdir),
      '-DSCIM_LIBEXECDIR="@0@"'.format(scim_libexecdir),
      '-DSCIM_ICONDIR="@0@"'.format(scim_icondir),
      '-DSCIM_MODULE_PATH="@0@"'.format(scim_module_path),
      '-DSCIM_TEMPDIR="@0@"'.format(scim_tempdir),
    ]
  )
endif

if get_option('im-socket') and socket_ok
  shared_module('socket',
    'scim_socket_imengine.cpp',
    dependencies: [libscim_dep],
    install: true,
    install_dir: join_paths(scim_module_path, 'IMEngine'),
    name_prefix: '',
    cpp_args: [
      '-DSCIM_DATADIR="@0@"'.format(scim_datadir),
      '-DSCIM_LOCALEDIR="@0@"'.format(scim_localedir),
      '-DSCIM_SYSCONFDIR="@0@"'.format(scim_sysconfdir),
      '-DSCIM_LIBEXECDIR="@0@"'.format(scim_libexecdir),
      '-DSCIM_ICONDIR="@0@"'.format(scim_icondir),
      '-DSCIM_MODULE_PATH="@0@"'.format(scim_module_path),
      '-DSCIM_TEMPDIR="@0@"'.format(scim_tempdir),
    ]
  )
endif
