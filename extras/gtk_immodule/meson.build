# extras/gtk_immodule/meson.build
# This handles the "original" GTK IM modules if requested.
# The user might want the modern immodules in extras/immodules instead.

if get_option('orig-gtk2-immodule') and gtk2_dep.found()
  subdir('gtk2')
endif

if get_option('orig-gtk3-immodule') and gtk3_dep.found()
  subdir('gtk3')
endif
