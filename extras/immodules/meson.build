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
if get_option('gtk2-immodule') and gtk2_dep.found()
  subdir('client-gtk/gtk2')
endif

if get_option('gtk3-immodule') and gtk3_dep.found()
  subdir('client-gtk/gtk3')
endif

if get_option('gtk4-immodule') and gtk4_dep.found()
  subdir('client-gtk/gtk4')
endif

# QT IM Modules
if get_option('qt3-immodule') and qt3_dep.found()
  subdir('client-qt/qt3')
endif

if get_option('qt4-immodule') and qt4_dep.found()
  subdir('client-qt/qt4')
endif

if get_option('qt5-immodule') and qt5_dep.found()
  subdir('client-qt/qt5')
endif

# Clutter IM Module
if get_option('clutter-immodule') and clutter_dep.found()
  subdir('client-clutter')
endif
