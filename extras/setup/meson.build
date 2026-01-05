# extras/setup/meson.build
if get_option('setup-ui') and gtk_dep_for_utils.found()
  executable('scim-setup',
    'scim_setup.cpp',
    dependencies : [libscim_dep, libscim_gtkutils_dep, gtk_dep_for_utils],
    install : true,
    install_dir : join_paths(prefix, 'bin')
  )
endif
