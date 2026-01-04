if get_option('filter-sctc')
  shared_module('sctc',
    'scim_sctc_filter.cpp',
    dependencies: [libscim_dep],
    install: true,
    install_dir: join_paths(scim_module_path, 'Filter'),
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
