# extras/immodules/client-qt/qt4/meson.build
if qt4_dep.found()
  shared_module('im-scim-bridge-qt4',
    'scim-bridge-client-immodule-qt.cpp',
    dependencies : [libscim_bridge_client_common_dep, qt4_dep],
    install : true
  )
endif
