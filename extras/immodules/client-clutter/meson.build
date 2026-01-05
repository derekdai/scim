# extras/immodules/client-clutter/meson.build
if clutter_dep.found() and clutter_imcontext_dep.found()
  shared_module('im-scim-bridge-clutter',
    'scim-bridge-client-immodule-clutter.c',
    dependencies : [libscim_bridge_client_common_dep, clutter_dep, clutter_imcontext_dep],
    install : true
  )
endif
