# GTK2 IM Module (Bridge)
gtk2_im_module_dir = get_option('gtk2-im-module-dir')
if gtk2_im_module_dir == ''
  gtk2_im_module_dir = gtk2_dep.get_pkgconfig_variable('libdir') + '/gtk-2.0/' + gtk2_dep.get_pkgconfig_variable('gtk_binary_version') + '/immodules'
endif

shared_module('im-scim-bridge',
    ['../scim-bridge-client-gtk.c', '../scim-bridge-client-imcontext-gtk.c', '../scim-bridge-client-key-event-utility-gtk.c'],
    dependencies: [gtk2_dep, scim_bridge_common_dep, scim_bridge_client_common_dep],
    install: true,
    install_dir: gtk2_im_module_dir,
    include_directories: include_directories('..'),
    name_prefix: ''
)
