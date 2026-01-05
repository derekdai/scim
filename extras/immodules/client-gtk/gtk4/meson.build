# extras/immodules/client-gtk/gtk4/meson.build
if gtk4_dep.found()
  if get_option('gtk4-im-module-dir') != ''
      gtk4_im_moduledir = get_option('gtk4-im-module-dir')
  else
      gtk4_im_moduledir = gtk4_dep.get_variable(pkgconfig : 'immodulesdir', default_value : get_option('libdir') / 'gtk-4.0' / '4.0.0' / 'immodules')
  endif

  im_scim_gtk4_sources = [
    '../im-scim-bridge-gtk.c',
    '../scim-bridge-client-gtk.c',
    '../scim-bridge-client-imcontext-gtk.c',
    '../scim-bridge-client-key-event-utility-gtk.c'
  ]

  shared_module('im-scim',
    im_scim_gtk4_sources,
    dependencies : [
      libscim_bridge_common_dep,
      libscim_bridge_client_common_dep,
      gtk4_dep,
      x11_dep
    ],
    include_directories : [
      include_directories('../../common'),
      include_directories('../../client-common')
    ],
    install : true,
    install_dir : gtk4_im_moduledir
  )
endif
