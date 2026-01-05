# extras/panel/meson.build
if get_option('panel-gtk').enabled() or (get_option('panel-gtk').auto() and gtk_dep_for_utils.found())
  executable('scim-panel-gtk',
    ['scim_panel_gtk.cpp', 'scim_panel_gtk_main.cpp'],
    dependencies : [libscim_dep, libscim_gtkutils_dep, gtk_dep_for_utils],
    install : true,
    install_dir : scim_libexecdir
  )
endif
