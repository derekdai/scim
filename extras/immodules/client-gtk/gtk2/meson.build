# extras/immodules/client-gtk/gtk2/meson.build
gtk2_im_moduledir = get_option('libdir') / 'gtk-2.0' / '2.10.0' / 'immodules'

shared_module('im-scim-bridge',
  'scim-bridge-client-immodule-gtk.c',
  dependencies : [libscim_bridge_client_common_dep, gtk2_dep],
  install : true,
  install_dir : gtk2_im_moduledir,
  c_args : '-DSCIM_BRIDGE_GTK_IMMODULE_GTK2'
)
