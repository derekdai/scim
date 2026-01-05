if scim_build_setup_ui
  # scim-setup script
  configure_file(
    input: 'scim-setup.in',
    output: 'scim-setup',
    configuration: {'bindir': join_paths(prefix, 'bin')},
    install: true,
    install_dir: join_paths(prefix, 'bin'),
    install_mode: 'rwxr-xr-x'
  )

  # setup.la
  helper_mod_dir = join_paths(scim_libdir, 'scim' + scim_epoch, scim_binary_version, 'Helper')
  library('setup',
    ['scim_setup_ui.cpp', 'scim_setup_module.cpp', 'scim_setup_helper.cpp'],
    include_directories: [inc, src_inc],
    dependencies: [libscim_dep, setup_gtk_dep],
    cpp_args: [
        '-DSCIM_DATADIR="@0@"'.format(scim_datadir),
        '-DSCIM_LOCALEDIR="@0@"'.format(scim_localedir),
        '-DSCIM_SYSCONFDIR="@0@"'.format(scim_sysconfdir),
        '-DSCIM_LIBEXECDIR="@0@"'.format(scim_libexecdir),
        '-DSCIM_ICONDIR="@0@"'.format(scim_icondir),
        '-DSCIM_MODULE_PATH="@0@"'.format(scim_module_path),
        '-DSCIM_TEMPDIR="@0@"'.format(scim_tempdir)
    ],
    install: true,
    install_dir: helper_mod_dir,
    name_prefix: '',
    override_options: ['b_lundef=false']
  )

  # scim-setup.desktop
  # Uses intltool in autotools. In meson we use i18n module.
  i18n.merge_file(
    input: 'scim-setup.desktop.in',
    output: 'scim-setup.desktop',
    type: 'desktop',
    po_dir: meson.project_source_root() + '/po',
    install: true,
    install_dir: join_paths(datadir, 'applications')
  )

  install_data(
      'scim-setup.desktop.in', # Installing the source as placeholder if merge fails or simple copy needed
      rename: 'scim-setup.desktop',
      install_dir: join_paths(datadir, 'control-center-2.0/capplets')
  )
endif
