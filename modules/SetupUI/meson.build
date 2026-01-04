if scim_build_setup_ui
  
  # Frontend Setup
  shared_module('aaa-frontend-setup',
    'scim_frontend_setup.cpp',
    dependencies: [
        setup_gtk_dep,
        libscim_dep,
        libscim_gtkutils_dep
    ],
    install: true,
    install_dir: join_paths(scim_module_path, scim_binary_version, 'SetupUI'),
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
  
  # IMEngine Setup
  shared_module('aaa-imengine-setup',
    'scim_imengine_setup.cpp',
    dependencies: [
        setup_gtk_dep,
        libscim_dep,
        libscim_gtkutils_dep
    ],
    install: true,
    install_dir: join_paths(scim_module_path, scim_binary_version, 'SetupUI'),
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

  # Panel GTK Setup
  if enable_panel_gtk
      shared_module('panel-gtk-setup',
        'scim_panel_gtk_setup.cpp',
        dependencies: [
            setup_gtk_dep,
            libscim_dep,
            libscim_gtkutils_dep
        ],
        install: true,
        install_dir: join_paths(scim_module_path, scim_binary_version, 'SetupUI'),
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
endif
