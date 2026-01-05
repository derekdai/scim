# modules/Filter/meson.build

if get_option('filter-sctc')
  shared_module('sctc',
    'scim_sctc_filter.cpp',
    dependencies : libscim_dep,
    install : true,
    install_dir : join_paths(scim_module_path, scim_binary_version, 'Filter')
  )
endif
