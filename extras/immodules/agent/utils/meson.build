# extras/immodules/agent/utils/meson.build

agent_utils_lib = static_library('agent_utils',
  ['scim-bridge-agent-panel-client.cpp'],
  dependencies : [libscim_dep, libscim_bridge_common_dep],
  include_directories : [include_directories('.'), include_directories('../../common')]
)

agent_utils_dep = declare_dependency(
  link_with : agent_utils_lib,
  include_directories : [include_directories('.'), include_directories('../../common')],
  dependencies : libscim_bridge_common_dep
)
