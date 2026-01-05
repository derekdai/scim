# extras/gtk_immodule/gtk2/meson.build
gtk2_im_moduledir = get_option('libdir') / 'gtk-2.0' / '2.10.0' / 'immodules' # Placeholder path
shared_module('im-scim',
  ['gtkimcontextscim.cpp', 'gtkimcontextscim.h'],
  dependencies : [libscim_dep, gtk2_dep],
  install : true,
  install_dir : gtk2_im_moduledir
)
