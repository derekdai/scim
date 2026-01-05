executable('scim',
  '../scim.cpp',
  include_directories: inc,
  dependencies: [libscim_dep],
  install: true
)

executable('scim-config-agent',
  '../scim_config_agent.cpp',
  include_directories: inc,
  dependencies: [libscim_dep],
  install: true
)

# Libexec programs
# libexecdir is defined in root meson.build but scim_libexecdir is what autotools uses for install_dir?
# Autotools: libexecdir = $(libdir)/scim@SCIM_EPOCH@
# meson.build root: scim_libexecdir = join_paths(libdir, 'scim' + scim_epoch)
# So we install to scim_libexecdir.

executable('scim-launcher',
  '../scim_launcher.cpp',
  include_directories: inc,
  dependencies: [libscim_dep],
  install: true,
  install_dir: scim_libexecdir
)

executable('scim-helper-launcher',
  '../scim_helper_launcher.cpp',
  include_directories: inc,
  dependencies: [libscim_dep],
  install: true,
  install_dir: scim_libexecdir
)

executable('scim-helper-manager',
  '../scim_helper_manager_server.cpp',
  include_directories: inc,
  dependencies: [libscim_dep],
  install: true,
  install_dir: scim_libexecdir
)
