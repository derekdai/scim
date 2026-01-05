# extras/immodules/client-gtk/gtk4/meson.build
gtk4_im_moduledir = get_option('libdir') / 'gtk-4.0' / '4.0.0' / 'immodules'

shared_module('im-scim-bridge',
  'scim-bridge-client-immodule-gtk.c',
  dependencies : [libscim_bridge_client_common_dep, gtk4_dep],
  install : true,
  install_dir : gtk4_im_moduledir,
  c_args : '-DSCIM_BRIDGE_GTK_IMMODULE_GTK4'
)
