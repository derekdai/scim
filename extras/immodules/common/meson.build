# extras/immodules/common/meson.build
common_sources = [
  'scim-bridge-attribute.c',
  'scim-bridge-display.c',
  'scim-bridge-key-event.c',
  'scim-bridge-message.c',
  'scim-bridge-messenger.c',
  'scim-bridge-path.c',
  'scim-bridge-string.c'
]

libscim_bridge_common = static_library('scim-bridge-common',
  common_sources,
  include_directories : [inc],
  dependencies : [dl_dep]
)

libscim_bridge_common_dep = declare_dependency(
  link_with : libscim_bridge_common,
  include_directories : include_directories('.')
)
