# extras/immodules/client-gtk/gtk2/meson.build
if gtk2_dep.found()
  # Check for override option first
  if get_option('gtk2-im-module-dir') != ''
    gtk2_im_moduledir = get_option('gtk2-im-module-dir')
  else
    # Use default path if pkg-config variable is not found
    gtk2_im_moduledir = gtk2_dep.get_variable(pkgconfig : 'immodulesdir', default_value : get_option('libdir') / 'gtk-2.0' / '2.10.0' / 'immodules')
  endif

  im_scim_gtk2_sources = [
    '../im-scim-bridge-gtk.c',
    '../scim-bridge-client-gtk.c',
    '../scim-bridge-client-imcontext-gtk.c',
    '../scim-bridge-client-key-event-utility-gtk.c'
  ]

  shared_module('im-scim',
    im_scim_gtk2_sources,
    dependencies : [
      libscim_bridge_common_dep,
      libscim_bridge_client_common_dep,
      gtk2_dep,
      x11_dep
    ],
    include_directories : [
      include_directories('../../common'),
      include_directories('../../client-common')
    ],
    install : true,
    install_dir : gtk2_im_moduledir
  )
endif
