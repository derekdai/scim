libscim_sources = [
  'scim.cpp',
  'scim_backend.cpp',
  'scim_chartraits.cpp',
  'scim_compose_key.cpp',
  'scim_config_agent.cpp',
  'scim_config_base.cpp',
  'scim_config_module.cpp',
  'scim_connection.cpp',
  'scim_debug.cpp',
  'scim_event.cpp',
  'scim_filter.cpp',
  'scim_filter_manager.cpp',
  'scim_filter_module.cpp',
  'scim_frontend.cpp',
  'scim_frontend_module.cpp',
  'scim_global_config.cpp',
  'scim_helper.cpp',
  'scim_helper_launcher.cpp',
  'scim_helper_manager.cpp',
  'scim_helper_manager_server.cpp',
  'scim_helper_module.cpp',
  'scim_hotkey.cpp',
  'scim_iconv.cpp',
  'scim_imengine.cpp',
  'scim_imengine_module.cpp',
  'scim_launcher.cpp',
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
  'scim_utility.cpp',
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
  'scim_filter_manager.h',
  'scim_filter_module.h',
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
  'scim_keyboard_layout_data.h',
  'scim_keyevent_data.h',
  'scim_lookup_table.h',
  'scim_module.h',
  'scim_object.h',
  'scim_panel_agent.h',
  'scim_panel_client.h',
  'scim_panel_common.h',
  'scim_pointer.h',
  'scim_private.h',
  'scim_property.h',
  'scim_signals.h',
  'scim_slot.h',
  'scim_socket.h',
  'scim_stl_map.h',
  'scim_trans_commands.h',
  'scim_transaction.h',
  'scim_utility.h'
]

# Generate scim_types.h
scim_types_h = configure_file(
  input: 'scim_types.h.in',
  output: 'scim_types.h',
  configuration: conf_data
)

# AM_CPPFLAGS	= -DSCIM_DATADIR=\"@SCIM_DATADIR@\" \
#			  -DSCIM_LOCALEDIR=\"@SCIM_LOCALEDIR@\" \
#			  -DSCIM_SYSCONFDIR=\"@SCIM_SYSCONFDIR@\" \
#			  -DSCIM_LIBEXECDIR=\"@SCIM_LIBEXECDIR@\" \
#			  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
#			  -DSCIM_MODULE_PATH=\"@SCIM_MODULE_PATH@\" \
#			  -DSCIM_TEMPDIR=\"@SCIM_TEMPDIR@\"

libscim_cpp_args = [
  '-DSCIM_DATADIR="@0@"'.format(scim_datadir),
  '-DSCIM_LOCALEDIR="@0@"'.format(scim_localedir),
  '-DSCIM_SYSCONFDIR="@0@"'.format(scim_sysconfdir),
  '-DSCIM_LIBEXECDIR="@0@"'.format(scim_libexecdir),
  '-DSCIM_ICONDIR="@0@"'.format(scim_icondir),
  '-DSCIM_MODULE_PATH="@0@"'.format(scim_module_path),
  '-DSCIM_TEMPDIR="@0@"'.format(scim_tempdir),
  '-DSCIM_VERSION="@0@"'.format(scim_version),
  '-DSCIM_BINARY_VERSION="@0@"'.format(scim_binary_version)
]

libscim = library('scim-' + scim_epoch,
  libscim_sources + [scim_types_h],
  version: '8.2.3',
  soversion: '8',
  include_directories: inc,
  dependencies: [libltdl, libdl],
  cpp_args: libscim_cpp_args,
  install: true
)

libscim_dep = declare_dependency(
  link_with: libscim,
  include_directories: [inc, src_inc]
)

install_headers(libscim_headers, subdir: 'scim')
install_headers(scim_types_h, subdir: 'scim')
