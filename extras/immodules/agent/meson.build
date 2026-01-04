executable('scim-im-agent',
    ['scim-bridge-agent.cpp', 
     'scim-bridge-agent-application.cpp', 
     'scim-bridge-agent-client-listener.cpp', 
     'scim-bridge-agent-imcontext.cpp', 
     'scim-bridge-agent-signal-listener.cpp',
     'scim-bridge-agent-accept-listener.cpp',
     'scim-bridge-agent-interruption-listener.cpp',
     'scim-bridge-agent-output.cpp',
     'scim-bridge-agent-panel-listener.cpp'],
    dependencies: [scim_bridge_common_dep, libltdl, libdl, libscim_dep],
    install: true,
    install_dir: scim_libexecdir
)
