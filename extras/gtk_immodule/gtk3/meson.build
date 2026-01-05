# extras/gtk_immodule/gtk3/meson.build
gtk3_im_moduledir = get_option('libdir') / 'gtk-3.0' / '3.0.0' / 'immodules' # Placeholder path
shared_module('im-scim',
  ['gtkimcontextscim.cpp', 'gtkimcontextscim.h'],
  dependencies : [libscim_dep, gtk3_dep],
  install : true,
  install_dir : gtk3_im_moduledir
)
