project('scim', 'cpp', 'c',
  version : '1.4.19',
  default_options : ['cpp_std=c++11']
)

# Version info
scim_major_version = 1
scim_minor_version = 4
scim_micro_version = 19
scim_version = '@0@.@1@.@2@'.format(scim_major_version, scim_minor_version, scim_micro_version)
scim_current = 10
scim_revision = 7
scim_age = 2
scim_epoch = '-1.0'
scim_binary_version = '1.4.0'
scim_lib_version = '8.2.3'
scim_so_major = '8'
scim_datadir = join_paths(get_option('prefix'), get_option('datadir'), 'scim')
scim_sysconfdir = join_paths(get_option('prefix'), get_option('sysconfdir'))
scim_icondir = join_paths(scim_datadir, 'icons')
scim_module_path = join_paths(get_option('prefix'), get_option('libdir'), 'scim' + scim_epoch)
scim_libexecdir = join_paths(get_option('prefix'), get_option('libdir'), 'scim' + scim_epoch)
scim_localedir = join_paths(get_option('prefix'), get_option('datadir'), 'locale')
scim_libdir = join_paths(get_option('prefix'), get_option('libdir'))

# Compiler setup
cpp = meson.get_compiler('cpp')
cc = meson.get_compiler('c')

add_project_arguments('-DSCIM_VERSION="@0@"'.format(scim_version), language : ['c', 'cpp'])
add_project_arguments('-DSCIM_BINARY_VERSION="@0@"'.format(scim_binary_version), language : ['c', 'cpp'])
add_project_arguments('-DSCIM_EPOCH="@0@"'.format(scim_epoch), language : ['c', 'cpp'])
add_project_arguments('-DSCIM_DATADIR="@0@"'.format(scim_datadir), language : ['c', 'cpp'])
add_project_arguments('-DSCIM_SYSCONFDIR="@0@"'.format(scim_sysconfdir), language : ['c', 'cpp'])
add_project_arguments('-DSCIM_ICONDIR="@0@"'.format(scim_icondir), language : ['c', 'cpp'])
add_project_arguments('-DSCIM_MODULE_PATH="@0@"'.format(scim_module_path), language : ['c', 'cpp'])
add_project_arguments('-DSCIM_LIBEXECDIR="@0@"'.format(scim_libexecdir), language : ['c', 'cpp'])
add_project_arguments('-DSCIM_LOCALEDIR="@0@"'.format(scim_localedir), language : ['c', 'cpp'])
add_project_arguments('-DSCIM_LIBDIR="@0@"'.format(scim_libdir), language : ['c', 'cpp'])
add_project_arguments('-DGETTEXT_PACKAGE="scim"', language : ['c', 'cpp'])

# Dependencies
x11_dep = dependency('x11', required : false)
gtk2_dep = dependency('gtk+-2.0', version : '>= 2.4.0', required : false)
gtk3_dep = dependency('gtk+-3.0', version : '>= 3.0.0', required : false)
gtk4_dep = dependency('gtk4', version : '>= 4.0.0', required : false)
qt5_dep = dependency('Qt5Core', required : false)
qt5_gui_dep = dependency('Qt5Gui', required : false)
qt5_widgets_dep = dependency('Qt5Widgets', required : false)
qt3_dep = dependency('qt3', required : false) # Placeholder for Qt3 dependency
qt4_dep = dependency('qt4', required : false) # Placeholder for Qt4 dependency
clutter_dep = dependency('clutter-1.0', required : false) # Placeholder for Clutter dependency
# libsystemd_dep = dependency('libsystemd', version : '>= 245', required : false)
gio_dep = dependency('gio-2.0', required : false)
gio_unix_dep = dependency('gio-unix-2.0', required : false)
glib_dep = dependency('glib-2.0', required : false)
dbus_dep = dependency('dbus-1', required : false)
dl_dep = cpp.find_library('dl', required : false)

if get_option('ltdladvise')
    add_project_arguments('-DSCIM_LTDLADVISE=1', language : ['c', 'cpp'])
endif

# Check headers
check_headers = [
  'langinfo.h', 'libintl.h', 'string.h', 'dirent.h', 'stdint.h',
  'sys/time.h', 'unistd.h'
]

# Check for socket support
socket_ok = true # We assume socket support is available on modern systems

foreach h : check_headers
  if cpp.has_header(h)
    add_project_arguments('-DHAVE_' + h.to_upper().replace('/', '_').replace('.', '_'), language : ['c', 'cpp'])
  endif
endforeach

# Generate scim_types.h
scim_types_data = configuration_data()
if cpp.has_header('stdint.h')
  scim_types_data.set('INCLUDE_STDINT', '#include <stdint.h>')
  scim_types_data.set('UINT16', 'uint16_t')
  scim_types_data.set('UINT32', 'uint32_t')
  scim_types_data.set('UINT64', 'uint64_t')
else
  scim_types_data.set('INCLUDE_STDINT', '')
  scim_types_data.set('UINT16', 'unsigned short')
  scim_types_data.set('UINT32', 'unsigned int')
  scim_types_data.set('UINT64', 'unsigned long long')
endif

configure_file(
  input : 'src/scim_types.h.in',
  output : 'scim_types.h',
  configuration : scim_types_data,
  install : true,
  install_dir : join_paths(get_option('includedir'), 'scim-1.0')
)

# Generate config.h
conf_data = configuration_data()
conf_data.set('ICONV_CONST', '')
conf_data.set('SCIM_VERSION', scim_version)
conf_data.set('SCIM_BINARY_VERSION', scim_binary_version)
conf_data.set('GETTEXT_PACKAGE', 'scim')
conf_data.set('ENABLE_DEBUG', get_option('buildtype') == 'debug' ? 1 : 0)
configure_file(output : 'config.h', configuration : conf_data)

# Include directories
inc = include_directories('.', 'src', 'utils', 'extras/panel')

# Subprojects
ltdl_dep = dependency('ltdl', fallback: ['ltdl', 'ltdl_dep'])

# Subdirectories
subdir('src')
subdir('utils')
subdir('modules')
subdir('extras')
subdir('data')
subdir('po')

# Tests
if get_option('tests')
  subdir('tests')
endif

# Documentation
if get_option('documents')
  subdir('docs')
endif

# Pkgconfig files
pkg = import('pkgconfig')

# org.freedesktop.portal.IBus.service
if dbus_dep.found()
  dbus_session_services_dir = dbus_dep.get_pkgconfig_variable('session_bus_services_dir')

  # Process the .service.in file
  configure_file(
    input : 'org.freedesktop.portal.IBus.service.in',
    output : 'org.freedesktop.portal.IBus.service',
    configuration : {
      'SCIM_LIBEXECDIR': scim_libexecdir,
      'SCIM_BINARY_VERSION': scim_binary_version
    },
    install : true,
    install_dir : dbus_session_services_dir
  )
endif

# spec file
configure_file(
  input : 'scim.spec.in',
  output : 'scim.spec',
  configuration : {
    'SCIM_VERSION': scim_version,
  }
)
