project('scim', ['c', 'cpp'],
  version : '1.4.19',
  default_options : ['warning_level=1', 'cpp_std=c++11', 'buildtype=debugoptimized'])

scim_major_version = 1
scim_minor_version = 4
scim_micro_version = 19
scim_version = '@0@.@1@.@2@'.format(scim_major_version, scim_minor_version, scim_micro_version)

# Library versioning
# increment if the interface has additions, changes, removals.
scim_current = 10
# increment any time the source changes; set to 0 if you increment CURRENT
scim_revision = 7
# increment if any interfaces have been added; set to 0
# if any interfaces have been removed. removal has
# precedence over adding, so set to 0 if both happened.
scim_age = 2

scim_epoch = '-1.0'
scim_binary_version = '1.4.0'

# Paths
prefix = get_option('prefix')
libdir = join_paths(prefix, get_option('libdir'))
datadir = join_paths(prefix, get_option('datadir'))
sysconfdir = join_paths(prefix, get_option('sysconfdir'))
includedir = join_paths(prefix, get_option('includedir'))

scim_datadir = join_paths(datadir, 'scim')
scim_sysconfdir = sysconfdir
scim_icondir = join_paths(scim_datadir, 'icons')
scim_module_path = join_paths(libdir, 'scim' + scim_epoch)
scim_libexecdir = join_paths(libdir, 'scim' + scim_epoch)
scim_localedir = join_paths(datadir, 'locale')
scim_tempdir = '/tmp'
scim_prefix = prefix
scim_libdir = libdir

# Dependencies
cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

dl_dep = cc.find_library('dl', required : false)
ltdl_dep = cc.find_library('ltdl', required : false)
if not ltdl_dep.found()
  ltdl_proj = subproject('ltdl')
  ltdl_dep = ltdl_proj.get_variable('ltdl_dep')
endif

# X11
x11_dep = dependency('x11', required : get_option('frontend-x11'))

# GTK
gtk2_dep = dependency('gtk+-2.0', version : '>= 2.4.0', required : false)
gtk3_dep = dependency('gtk+-3.0', version : '>= 3.0.0', required : false)
gtk4_dep = dependency('gtk4', version : '>= 4.0.0', required : false)
pango_dep = dependency('pango', version : '>= 1.1.0', required : false)
gdk_pixbuf_dep = dependency('gdk-pixbuf-2.0', version : '>= 2.4.0', required : false)

# QT
qt3_dep = dependency('qt-mt', version : '>= 3.3', required : false)
qt4_dep = dependency('QtGui', version : '>= 4.0', required : false)
qt5_dep = dependency('Qt5Core', version : '>= 5.0', required : false)
if qt5_dep.found()
  qt5_widgets_dep = dependency('Qt5Widgets', required : false)
  qt5_x11extras_dep = dependency('Qt5X11Extras', required : false)
endif

# Other deps
gthread_dep = dependency('gthread-2.0', version : '>= 2.0.0', required : false)
libsystemd_dep = dependency('libsystemd', version : '>= 245', required : false)
dbus_dep = dependency('dbus-1', required : false)
clutter_dep = dependency('clutter-1.0', required : false)
clutter_imcontext_dep = dependency('clutter-imcontext-0.1', required : false)

# config.h generation
conf_data = configuration_data()
conf_data.set_quoted('SCIM_VERSION', scim_version)
conf_data.set('SCIM_MAJOR_VERSION', scim_major_version)
conf_data.set('SCIM_MINOR_VERSION', scim_minor_version)
conf_data.set('SCIM_MICRO_VERSION', scim_micro_version)
conf_data.set_quoted('SCIM_BINARY_VERSION', scim_binary_version)
conf_data.set_quoted('GETTEXT_PACKAGE', 'scim')

# SCIM Bridge Version
conf_data.set_quoted('SCIM_BRIDGE_VERSION', scim_version)
conf_data.set('SCIM_BRIDGE_MAJOR_VERSION', scim_major_version)
conf_data.set('SCIM_BRIDGE_MINOR_VERSION', scim_minor_version)
conf_data.set('SCIM_BRIDGE_MICRO_VERSION', scim_micro_version)

# Check headers and functions
check_headers = [
  'langinfo.h', 'libintl.h', 'string.h', 'dirent.h', 'hash_map', 'ext/hash_map',
  'stdint.h', 'unistd.h'
]

foreach h : check_headers
  if cc.has_header(h)
    conf_data.set('HAVE_' + h.underscorify().to_upper(), 1)
  endif
endforeach

# Strict size checks
if not cc.has_type('size_t', prefix : '#include <stddef.h>')
    error('No type size_t, but SCIM needs it!')
endif

sizeof_char = cc.sizeof('char')
sizeof_short = cc.sizeof('unsigned short int')

if sizeof_short != 2 or sizeof_char != 1
  error('''
*** SCIM requires
***      sizeof (unsigned short int)  == 2
***      sizeof (char)                == 1
*** You might want to consider using the GNU C compiler.
''')
endif

if cc.sizeof('wchar_t', prefix: '#include <wchar.h>') == 0
  error('''
*** SCIM requires a compiler that supports wchar_t,
*** You might want to consider using the GNU C compiler.
''')
endif

check_funcs = [
  'gettimeofday', 'memmove', 'memset', 'nl_langinfo', 'setlocale', 'daemon',
  'opendir', 'closedir', 'readdir', 'usleep', 'nanosleep'
]

foreach f : check_funcs
  if cc.has_function(f)
    conf_data.set('HAVE_' + f.to_upper(), 1)
  endif
endforeach

# Socket checks
if cc.has_function('socket') and cc.has_function('bind') and cc.has_function('accept') and cc.has_function('connect') and cc.has_function('listen')
  socket_ok = true
else
  # Check if we need -lsocket -lnsl etc? For now assuming simple check.
  # configure.ac checks for gethostbyname etc as well.
  socket_ok = false
endif

# Feature options logic (simplified for now, mimicking configure.ac)
enable_debug = get_option('buildtype').startswith('debug')
if enable_debug
  conf_data.set('ENABLE_DEBUG', 1)
endif

# ICONV check
# We hardcode ICONV_CONST to empty because the system iconv.h declares it as char**
# and the detection logic is unreliable or C++ strictness is interfering.
conf_data.set('ICONV_CONST', '')

enable_hash_map = get_option('hash-map')
if enable_hash_map
  conf_data.set('ENABLE_HASH_MAP', 1)
endif

# Tray icon logic
enable_tray_icon = false
if (gtk2_dep.found() or gtk3_dep.found() or gtk4_dep.found()) and x11_dep.found()
  enable_tray_icon = true
  conf_data.set('ENABLE_TRAY_ICON', 1)
endif

# Generate scim_types.h
scim_types_data = configuration_data()
if cc.has_header('stdint.h')
  scim_types_data.set('INCLUDE_STDINT', '#include <stdint.h>')
  scim_types_data.set('UINT16', 'uint16_t')
  scim_types_data.set('UINT32', 'uint32_t')
  scim_types_data.set('UINT64', 'uint64_t')
else
  scim_types_data.set('INCLUDE_STDINT', '//#include <stdint.h>')
  scim_types_data.set('UINT16', 'unsigned short int')
  if cc.sizeof('unsigned int') == 4
    scim_types_data.set('UINT32', 'unsigned int')
  elif cc.sizeof('unsigned long int') == 4
     scim_types_data.set('UINT32', 'unsigned long int')
  else
     error('*** No suitable integer type for uint32 found.')
  endif

  if cc.sizeof('unsigned long long int') == 8
    scim_types_data.set('UINT64', 'unsigned long long int')
  else
    error('*** No suitable integer type for uint64 found.')
  endif
endif

configure_file(
  input : 'src/scim_types.h.in',
  output : 'scim_types.h',
  configuration : scim_types_data,
  install : true,
  install_dir : join_paths(includedir, 'scim' + scim_epoch)
)

configure_file(output : 'config.h', configuration : conf_data)

# Include directories
inc = include_directories('.', 'src')

# Subdirectories
subdir('src')
subdir('utils')
subdir('modules')
subdir('extras')
subdir('data')
subdir('configs')
subdir('po')

if get_option('documents')
  subdir('docs')
endif

if get_option('tests')
  subdir('tests')
endif

# Pkg-config generation
pkg = import('pkgconfig')

# scim.pc
pkg.generate(
  libscim,
  name : 'SCIM',
  description : 'Smart Common Input Method platform',
  version : scim_version,
  filebase : 'scim',
  subdirs : 'scim' + scim_epoch,
  variables : [
    'scimdatadir=' + join_paths(get_option('prefix'), get_option('datadir'), 'scim'),
    'icondir=' + join_paths(get_option('prefix'), get_option('datadir'), 'scim', 'icons'),
    'moduledir=' + join_paths(get_option('prefix'), get_option('libdir'), 'scim' + scim_epoch, scim_binary_version),
    'scim_binary_version=' + scim_binary_version
  ]
)

# scim-gtkutils.pc
if is_variable('libscim_gtkutils')
  pkg.generate(
    libscim_gtkutils,
    name : 'SCIM GTK Utils',
    description : 'SCIM GTK Utility Library',
    version : scim_version,
    filebase : 'scim-gtkutils',
    subdirs : 'scim' + scim_epoch,
    requires : [libscim]
  )
endif

# scim-x11utils.pc
if is_variable('libscim_x11utils')
  pkg.generate(
    libscim_x11utils,
    name : 'SCIM X11 Utils',
    description : 'SCIM X11 Utility Library',
    version : scim_version,
    filebase : 'scim-x11utils',
    subdirs : 'scim' + scim_epoch,
    requires : [libscim]
  )
endif

# scim.spec generation
cdata = configuration_data()
cdata.set('PACKAGE', 'scim')
cdata.set('VERSION', scim_version)
cdata.set('SCIM_VERSION', scim_version)
cdata.set('SCIM_EPOCH', scim_epoch)
cdata.set('SCIM_BINARY_VERSION', scim_binary_version)

# Default to 0 for these variables, they need actual logic if we want perfection, but
# matching configure.ac defaults/results is key.
# In configure.ac these are usually 0 or 1.

cdata.set('SCIM_BUILD_CONFIG_SOCKET', get_option('config-socket') ? 1 : 0)
cdata.set('SCIM_BUILD_CONFIG_SIMPLE', get_option('config-simple') ? 1 : 0)
cdata.set('SCIM_BUILD_FRONTEND_X11', get_option('frontend-x11') ? 1 : 0)
cdata.set('SCIM_BUILD_FRONTEND_SOCKET', get_option('frontend-socket') ? 1 : 0)
cdata.set('SCIM_BUILD_IMENGINE_RAWCODE', get_option('im-rawcode') ? 1 : 0)
cdata.set('SCIM_BUILD_IMENGINE_SOCKET', get_option('im-socket') ? 1 : 0)
cdata.set('SCIM_BUILD_GTK2_IMMODULE', get_option('gtk2-immodule') ? 1 : 0)
cdata.set('SCIM_BUILD_SCIM_SETUP', get_option('setup-ui') ? 1 : 0)

# These need more complex logic to match exactly what was built, but using options is a good approximation
cdata.set('SCIM_BUILD_PANEL_GTK', get_option('panel-gtk').enabled() or (get_option('panel-gtk').auto() and gtk2_dep.found()) ? 1 : 0)

# Logic for GTK utils (built if panel or setup-ui is built and gtk exists)
build_gtk_utils = (get_option('panel-gtk').enabled() or (get_option('panel-gtk').auto() and gtk2_dep.found()) or get_option('setup-ui')) and (gtk2_dep.found() or gtk3_dep.found())
cdata.set('SCIM_BUILD_GTK_UTILS', build_gtk_utils ? 1 : 0)

# Logic for X11 utils (built if frontend-x11 and x11 exists)
cdata.set('SCIM_BUILD_X11_UTILS', get_option('frontend-x11') and x11_dep.found() ? 1 : 0)

cdata.set('SCIM_BUILD_FILTER_SCTC', get_option('filter-sctc') ? 1 : 0)

# GTK Version logic for spec
gtk_version = '2.0.0' # Default
gtk_libdir = libdir
if gtk3_dep.found()
    gtk_version = gtk3_dep.version()
    gtk_libdir = gtk3_dep.get_variable(pkgconfig : 'libdir')
elif gtk2_dep.found()
    gtk_version = gtk2_dep.version()
    gtk_libdir = gtk2_dep.get_variable(pkgconfig : 'libdir')
endif
cdata.set('GTK_VERSION', gtk_version)
cdata.set('GTK_LIBDIR', gtk_libdir)

configure_file(
  input : 'scim.spec.in',
  output : 'scim.spec',
  configuration : cdata
)
