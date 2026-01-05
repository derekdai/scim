if get_option('frontend-x11')
  subdir('IMdkit')
  
  shared_module('x11',
    ['scim_x11_frontend.cpp'],
    dependencies: [libscim_dep, x11_dep, imdkit_dep, libscim_x11utils_dep],
    install: true,
    install_dir: join_paths(scim_module_path, scim_binary_version, 'FrontEnd'),
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

if get_option('frontend-socket') and socket_ok
  shared_module('socket',
    'scim_socket_frontend.cpp',
    dependencies: [libscim_dep],
    install: true,
    install_dir: join_paths(scim_module_path, scim_binary_version, 'FrontEnd'),
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

if get_option('frontend-ibus') and scim_has_sd and scim_has_dbus
    shared_module('ibus',
        ['scim_ibus_frontend.cpp'],
        dependencies: [libscim_dep, libsystemd_dep, dbus_dep],
        install: true,
        install_dir: join_paths(scim_module_path, scim_binary_version, 'FrontEnd'),
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
