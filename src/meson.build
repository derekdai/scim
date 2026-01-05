libscim_sources = [
  'scim_backend.cpp',
  'scim_chartraits.cpp',
  'scim_compose_key.cpp',
  'scim_config_base.cpp',
  'scim_config_module.cpp',
  'scim_connection.cpp',
  'scim_debug.cpp',
  'scim_event.cpp',
  'scim_filter.cpp',
  'scim_filter_module.cpp',
  'scim_filter_manager.cpp',
  'scim_frontend.cpp',
  'scim_frontend_module.cpp',
  'scim_global_config.cpp',
  'scim_helper.cpp',
  'scim_helper_manager.cpp',
  'scim_helper_module.cpp',
  'scim_hotkey.cpp',
  'scim_iconv.cpp',
  'scim_imengine.cpp',
  'scim_imengine_module.cpp',
  'scim_lookup_table.cpp',
  'scim_module.cpp',
  'scim_object.cpp',
  'scim_panel_agent.cpp',
  'scim_panel_client.cpp',
  'scim_private.cpp',
  'scim_signals.cpp',
  'scim_slot.cpp',
  'scim_socket.cpp',
  'scim_transaction.cpp',
  'scim_utility.cpp'
]

libscim_headers = [
  'scim.h',
  'scim_attribute.h',
  'scim_backend.h',
  'scim_bind.h',
  'scim_compose_key.h',
  'scim_config_base.h',
  'scim_config_module.h',
  'scim_config_path.h',
  'scim_connection.h',
  'scim_debug.h',
  'scim_event.h',
  'scim_exception.h',
  'scim_filter.h',
  'scim_filter_module.h',
  'scim_filter_manager.h',
  'scim_frontend.h',
  'scim_frontend_module.h',
  'scim_global_config.h',
  'scim_helper.h',
  'scim_helper_manager.h',
  'scim_helper_module.h',
  'scim_hotkey.h',
  'scim_iconv.h',
  'scim_imengine.h',
  'scim_imengine_module.h',
  'scim_lookup_table.h',
  'scim_module.h',
  'scim_object.h',
  'scim_panel_agent.h',
  'scim_panel_client.h',
  'scim_panel_common.h',
  'scim_pointer.h',
  'scim_property.h',
  'scim_signals.h',
  'scim_slot.h',
  'scim_socket.h',
  'scim_trans_commands.h',
  'scim_transaction.h',
  # 'scim_types.h', # Generated in root build dir
  'scim_utility.h'
]

# Add config.h defines
add_project_arguments('-DSCIM_DATADIR="@0@"'.format(scim_datadir), language : 'cpp')
add_project_arguments('-DSCIM_LOCALEDIR="@0@"'.format(scim_localedir), language : 'cpp')
add_project_arguments('-DSCIM_SYSCONFDIR="@0@"'.format(scim_sysconfdir), language : 'cpp')
add_project_arguments('-DSCIM_LIBEXECDIR="@0@"'.format(scim_libexecdir), language : 'cpp')
add_project_arguments('-DSCIM_ICONDIR="@0@"'.format(scim_icondir), language : 'cpp')
add_project_arguments('-DSCIM_MODULE_PATH="@0@"'.format(scim_module_path), language : 'cpp')
add_project_arguments('-DSCIM_BINARY_VERSION="@0@"'.format(scim_binary_version), language : 'cpp')
add_project_arguments('-DHAVE_CONFIG_H', language : 'cpp')

# Find libintl
intl_dep = cc.find_library('intl', required : false)

link_args = []
if get_option('ld-version-script')
  link_args += '-Wl,--version-script=' + meson.current_source_dir() + '/libscim.version-script'
endif

# ltdladvise handling is mostly about how modules are loaded, which affects source code,
# but if it implies linking flags, we would add them here.
# configure.ac just defines SCIM_LTDLADVISE in config.h.

libscim = shared_library('scim' + scim_epoch,
  libscim_sources,
  dependencies : [dl_dep, ltdl_dep, intl_dep],
  version : '@0@.@1@.@2@'.format(scim_current, scim_revision, scim_age),
  soversion : scim_current,
  install : true,
  include_directories : inc,
  link_args : link_args
)

libscim_dep = declare_dependency(
  link_with : libscim,
  include_directories : inc,
  dependencies : [dl_dep, ltdl_dep, intl_dep]
)

install_headers(libscim_headers, subdir : 'scim' + scim_epoch)

# Executables
executable('scim',
  'scim.cpp',
  dependencies : libscim_dep,
  install : true
)

executable('scim-config-agent',
  'scim_config_agent.cpp',
  dependencies : libscim_dep,
  install : true
)

executable('scim-launcher',
  'scim_launcher.cpp',
  dependencies : libscim_dep,
  install : true,
  install_dir : scim_libexecdir
)

executable('scim-helper-launcher',
  'scim_helper_launcher.cpp',
  dependencies : libscim_dep,
  install : true,
  install_dir : scim_libexecdir
)

executable('scim-helper-manager',
  'scim_helper_manager_server.cpp',
  dependencies : libscim_dep,
  install : true,
  install_dir : scim_libexecdir
)
