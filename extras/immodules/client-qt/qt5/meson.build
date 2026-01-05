# extras/immodules/client-qt/qt5/meson.build
if qt5_dep.found()
  shared_module('im-scim-bridge-qt5',
    'scim-bridge-client-immodule-qt.cpp',
    dependencies : [libscim_bridge_client_common_dep, qt5_dep, qt5_widgets_dep, qt5_x11extras_dep],
    install : true
  )
endif
