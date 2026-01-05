# modules/SetupUI/meson.build

if get_option('setup-ui') and gtk_dep_for_utils.found()
  setup_module_dir = join_paths(scim_module_path, scim_binary_version, 'SetupUI')

  # aaa-frontend-setup
  shared_module('aaa-frontend-setup',
    'scim_frontend_setup.cpp',
    dependencies : [libscim_dep, libscim_gtkutils_dep, gtk_dep_for_utils],
    install : true,
    install_dir : setup_module_dir
  )

  # aaa-imengine-setup
  shared_module('aaa-imengine-setup',
    'scim_imengine_setup.cpp',
    dependencies : [libscim_dep, libscim_gtkutils_dep, gtk_dep_for_utils],
    install : true,
    install_dir : setup_module_dir
  )

  # panel-gtk-setup
  if get_option('panel-gtk').enabled() or (get_option('panel-gtk').auto() and gtk_dep_for_utils.found())
      shared_module('panel-gtk-setup',
        'scim_panel_gtk_setup.cpp',
        dependencies : [libscim_dep, gtk_dep_for_utils],
        install : true,
        install_dir : setup_module_dir
      )
  endif
endif
