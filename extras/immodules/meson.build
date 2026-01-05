# extras/immodules/meson.build

# Common library for immodules (Must be first)
subdir('common')

# Common IM Agent (helper for immodules)
if get_option('agent')
  subdir('agent')
endif

# Client Common
subdir('client-common')

# GTK IM Modules
if get_option('gtk2-immodule') or get_option('gtk3-immodule') or get_option('gtk4-immodule')
  subdir('client-gtk')
endif

# QT IM Modules
if get_option('qt3-immodule') or get_option('qt4-immodule') or get_option('qt5-immodule')
  subdir('client-qt')
endif

# Clutter IM Module
if get_option('clutter-immodule') and clutter_dep.found()
  subdir('client-clutter')
endif

# Documentation
if get_option('documents')
  subdir('doc')
endif
