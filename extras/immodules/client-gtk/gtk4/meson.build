# GTK4 IM Module (Bridge)
gtk4_im_module_dir = get_option('gtk4-im-module-dir')
if gtk4_im_module_dir == ''
  gtk4_im_module_dir = gtk4_dep.get_pkgconfig_variable('libdir') + '/gtk-4.0/' + gtk4_dep.get_pkgconfig_variable('gtk_binary_version') + '/immodules'
endif

shared_module('im-scim-bridge',
    ['../scim-bridge-client-gtk.c', '../scim-bridge-client-imcontext-gtk.c', '../scim-bridge-client-key-event-utility-gtk.c'],
    dependencies: [gtk4_dep, scim_bridge_common_dep, scim_bridge_client_common_dep],
    install: true,
    install_dir: gtk4_im_module_dir,
    include_directories: include_directories('..'),
    name_prefix: ''
)
