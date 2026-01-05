# extras/immodules/client-qt/qt3/meson.build
# Assuming sources are here or logic is similar to GTK
# NOTE: QT3 is very old and likely requires special handling not easily guessable without seeing Makefile.am
# For now, adding a placeholder if source exists.
if qt3_dep.found()
  shared_module('im-scim-bridge-qt3',
    'scim-bridge-client-immodule-qt.cpp',
    dependencies : [libscim_bridge_client_common_dep, qt3_dep],
    install : true
  )
endif
