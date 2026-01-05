# extras/immodules/client-gtk/gtk3/meson.build
gtk3_im_moduledir = get_option('libdir') / 'gtk-3.0' / '3.0.0' / 'immodules'

shared_module('im-scim-bridge',
  'scim-bridge-client-immodule-gtk.c',
  dependencies : [libscim_bridge_client_common_dep, gtk3_dep],
  install : true,
  install_dir : gtk3_im_moduledir,
  c_args : '-DSCIM_BRIDGE_GTK_IMMODULE_GTK3'
)
