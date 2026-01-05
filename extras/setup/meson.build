# extras/setup/meson.build
if get_option('setup-ui') and gtk_dep_for_utils.found()
  configure_file(
    input : 'scim-setup.in',
    output : 'scim-setup',
    configuration : conf_data,
    install : true,
    install_dir : join_paths(prefix, 'bin')
  )

  shared_module('setup',
    ['scim_setup_ui.cpp', 'scim_setup_module.cpp', 'scim_setup_helper.cpp'],
    dependencies : [libscim_dep, libscim_gtkutils_dep, gtk_dep_for_utils],
    install : true,
    install_dir : join_paths(scim_module_path, scim_binary_version, 'Helper')
  )
endif
