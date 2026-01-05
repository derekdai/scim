scim_bridge_common_src = [
    'scim-bridge-messenger.c',
    'scim-bridge-message.c',
    'scim-bridge-string.c',
    'scim-bridge-attribute.c',
    'scim-bridge-display.c',
    'scim-bridge-key-event.c',
    'scim-bridge-path.c'
]

# This seems to be built into other targets, not a library by itself usually?
# In Makefile.am it's EXTRA_DIST or noinst_LTLIBRARIES.
# It is used by agent and clients.

libscim_bridge_common = static_library('scim-bridge-common',
    scim_bridge_common_src,
    include_directories: [include_directories('.'), inc],
    dependencies: [libltdl, libdl]
)

scim_bridge_common_dep = declare_dependency(
    link_with: libscim_bridge_common,
    include_directories: include_directories('.')
)
