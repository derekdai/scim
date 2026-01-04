scim_bridge_client_common_src = [
    'scim-bridge-client.c',
    'scim-bridge-client-output.c',
    'scim-bridge-client-debug.c'
]

libscim_bridge_client_common = static_library('scim-bridge-client-common',
    scim_bridge_client_common_src,
    dependencies: scim_bridge_common_dep,
    include_directories: include_directories('.')
)

scim_bridge_client_common_dep = declare_dependency(
    link_with: libscim_bridge_client_common,
    include_directories: include_directories('.')
)
