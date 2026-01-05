# extras/immodules/client-gtk/gtk3/meson.build
if gtk3_dep.found()
  gtk3_im_moduledir = gtk3_dep.get_variable(pkgconfig : 'immodulesdir', default_value : get_option('libdir') / 'gtk-3.0' / '3.0.0' / 'immodules')

  im_scim_gtk3_sources = [
    '../im-scim-bridge-gtk.c',
    '../scim-bridge-client-gtk.c',
    '../scim-bridge-client-imcontext-gtk.c',
    '../scim-bridge-client-key-event-utility-gtk.c'
  ]

  shared_module('im-scim',
    im_scim_gtk3_sources,
    dependencies : [
      libscim_bridge_common_dep,
      libscim_bridge_client_common_dep,
      gtk3_dep,
      x11_dep
    ],
    include_directories : [
      include_directories('../../common'),
      include_directories('../../client-common')
    ],
    install : true,
    install_dir : gtk3_im_moduledir
  )
endif
