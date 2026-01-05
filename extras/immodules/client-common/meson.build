# extras/immodules/client-common/meson.build
libscim_bridge_client_common_sources = [
  'scim-bridge-client.c',
  'scim-bridge-client-output.c',
  'scim-bridge-client-debug.c'
]

libscim_bridge_client_common = static_library('scim-bridge-client-common',
  libscim_bridge_client_common_sources,
  include_directories : [inc, include_directories('../common')],
  dependencies : [dl_dep, libscim_bridge_common_dep] # May need other deps
)

libscim_bridge_client_common_dep = declare_dependency(
  link_with : libscim_bridge_client_common,
  include_directories : [inc, include_directories('../common')],
  dependencies : libscim_bridge_common_dep
)
