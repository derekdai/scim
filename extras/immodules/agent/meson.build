# extras/immodules/agent/meson.build
agent_sources = [
  'scim-bridge-agent.cpp',
  'scim-bridge-agent-accept-listener.cpp',
  'scim-bridge-agent-application.cpp',
  'scim-bridge-agent-client-listener.cpp',
  'scim-bridge-agent-imcontext.cpp',
  'scim-bridge-agent-interruption-listener.cpp',
  'scim-bridge-agent-output.cpp',
  'scim-bridge-agent-panel-listener.cpp',
  'scim-bridge-agent-signal-listener.cpp'
]

# We need to include subdirectories or handle headers properly
# 'utils' subdirectory seems to be used here
# Note: 'common' must be built before 'utils' because utils uses it.
# And 'common' is in extras/immodules/common.
# But 'utils' is used by agent.
# The top-level extras/immodules/meson.build already traverses 'common'.
# So libscim_bridge_common_dep should be available if traverse order is correct.

subdir('utils')

executable('scim-im-agent',
  agent_sources,
  dependencies : [libscim_dep, agent_utils_dep],
  install : true,
  install_dir : scim_libexecdir,
  cpp_args : [
    '-DSCIM_KEYBOARD_ICON_FILE="@0@/keyboard.png"'.format(scim_icondir),
    '-DVERSION="@0@"'.format(scim_version)
  ]
)
