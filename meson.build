project('scim', 'cpp', 'c',
  version: '1.4.19',
  default_options: ['cpp_std=c++98', 'warning_level=1']
)

# Global definition of HAVE_CONFIG_H to ensure config.h is included
add_project_arguments('-DHAVE_CONFIG_H', language: ['c', 'cpp'])

scim_version = meson.project_version()
version_arr = scim_version.split('.')
scim_major_version = version_arr[0]
scim_minor_version = version_arr[1]
scim_micro_version = version_arr[2]

scim_epoch = '-1.0'
scim_binary_version = '1.4.0'

# Interface versions
scim_current = 10
scim_revision = 7
scim_age = 2

cc = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')

conf_data = configuration_data()

conf_data.set_quoted('SCIM_VERSION', scim_version)
conf_data.set('SCIM_MAJOR_VERSION', scim_major_version)
conf_data.set('SCIM_MINOR_VERSION', scim_minor_version)
conf_data.set('SCIM_MICRO_VERSION', scim_micro_version)
conf_data.set_quoted('SCIM_BINARY_VERSION', scim_binary_version)
conf_data.set_quoted('SCIM_EPOCH', scim_epoch)

# SCIM Bridge versions
conf_data.set_quoted('SCIM_BRIDGE_VERSION', scim_version)
conf_data.set('SCIM_BRIDGE_MAJOR_VERSION', scim_major_version)
conf_data.set('SCIM_BRIDGE_MINOR_VERSION', scim_minor_version)
conf_data.set('SCIM_BRIDGE_MICRO_VERSION', scim_micro_version)

conf_data.set_quoted('GETTEXT_PACKAGE', 'scim')
conf_data.set_quoted('PACKAGE', 'scim')
conf_data.set_quoted('VERSION', scim_version)
conf_data.set_quoted('PACKAGE_BUGREPORT', 'https://github.com/scim-im/scim')
conf_data.set_quoted('PACKAGE_NAME', 'scim')
conf_data.set_quoted('PACKAGE_STRING', 'scim ' + scim_version)
conf_data.set_quoted('PACKAGE_TARNAME', 'scim')
conf_data.set_quoted('PACKAGE_URL', 'https://github.com/scim-im/scim')
conf_data.set_quoted('PACKAGE_VERSION', scim_version)

# Check headers
headers = [
  'langinfo.h', 'libintl.h', 'string.h', 'dirent.h',
  'hash_map', 'ext/hash_map', 'stdint.h', 'unistd.h',
  'sys/time.h', 'sys/types.h'
]

foreach h : headers
  if cc.has_header(h) or cpp.has_header(h)
    conf_data.set('HAVE_' + h.underscorify().to_upper(), 1)
  endif
endforeach

# Sizes
conf_data.set('SIZEOF_CHAR', cc.sizeof('char'))
conf_data.set('SIZEOF_UNSIGNED_SHORT_INT', cc.sizeof('unsigned short int'))
conf_data.set('SIZEOF_UNSIGNED_INT', cc.sizeof('unsigned int'))
conf_data.set('SIZEOF_UNSIGNED_LONG_INT', cc.sizeof('unsigned long int'))
conf_data.set('SIZEOF_UNSIGNED_LONG_LONG_INT', cc.sizeof('unsigned long long int'))
conf_data.set('SIZEOF_WCHAR_T', cc.sizeof('wchar_t'))

# Types
if cc.has_header('stdint.h')
  conf_data.set('INCLUDE_STDINT', '#include <stdint.h>')
  conf_data.set('UINT16', 'uint16_t')
  conf_data.set('UINT32', 'uint32_t')
  conf_data.set('UINT64', 'uint64_t')
else
  conf_data.set('INCLUDE_STDINT', '//#include <stdint.h>')
  conf_data.set('UINT16', 'unsigned short int')
  if cc.sizeof('unsigned int') == 4
    conf_data.set('UINT32', 'unsigned int')
  elif cc.sizeof('unsigned long int') == 4
    conf_data.set('UINT32', 'unsigned long int')
  endif
  if cc.sizeof('unsigned long long int') == 8
    conf_data.set('UINT64', 'unsigned long long int')
  endif
endif

# Functions
functions = [
  'gettimeofday', 'memmove', 'memset', 'nl_langinfo', 'setlocale', 'daemon',
  'opendir', 'closedir', 'readdir', 'usleep', 'nanosleep'
]

foreach f : functions
  if cc.has_function(f)
    conf_data.set('HAVE_' + f.to_upper(), 1)
  endif
endforeach

# Socket functions
socket_funcs = ['gethostbyname', 'gethostbyname_r', 'socket', 'bind', 'accept', 'connect', 'listen']
socket_ok = true
foreach f : socket_funcs
  if not cc.has_function(f)
    # Check if libsocket is needed (Solaris usually) or other libs
    # simplified for now as linux usually has them in libc
    # If strictly strictly needed we can try checking libraries
    socket_ok = false
  endif
endforeach

if socket_ok
  # No define needed for socket_ok in config.h in original, but needed for build logic
endif

# Iconv
libiconv = cc.find_library('iconv', required: false)
if cc.has_function('iconv_open', dependencies: libiconv)
  conf_data.set('HAVE_ICONV', 1)
endif

# Check if iconv() second argument needs const
# Force empty for Linux/glibc as detection was tricky/incorrect
conf_data.set('ICONV_CONST', '')

# Dependencies
libdl = cc.find_library('dl', required : false)
libltdl = cc.find_library('ltdl', required : true)

# X11
x11_dep = dependency('x11', required: get_option('frontend-x11'))
if x11_dep.found()
  conf_data.set('HAVE_X11', 1) # Generally implied
endif

# GTK
gtk2_dep = dependency('gtk+-2.0', version: '>= 2.4.0', required: get_option('gtk2-immodule'))
gtk3_dep = dependency('gtk+-3.0', version: '>= 3.0.0', required: get_option('gtk3-immodule'))
gtk4_dep = dependency('gtk4', version: '>= 4.0.0', required: get_option('gtk4-immodule'))

if gtk2_dep.found()
    # Check for GDK_MULTIHEAD_SAFE
    if dependency('gtk+-2.0', version: '>= 2.2', required: false).found()
        conf_data.set('GDK_MULTIHEAD_SAFE', 1)
    endif
endif

# QT
qt3_dep = dependency('qt-mt', version: '>= 3.3', required: get_option('qt3-immodule'))
qt4_dep = dependency('QtGui', version: '>= 4.0', required: get_option('qt4-immodule'))
qt5_dep = dependency('Qt5Core', required: get_option('qt5-immodule'))
if qt5_dep.found()
  qt5_widgets = dependency('Qt5Widgets', required: true)
  qt5_x11 = dependency('Qt5X11Extras', required: true)
  # Check private headers if possible? Skiping for now.
endif

# Clutter
clutter_dep = dependency('clutter-1.0', required: get_option('clutter-immodule'))
clutter_im_dep = dependency('clutter-imcontext-0.1', required: get_option('clutter-immodule'))

# Systemd / DBus
libsystemd_dep = dependency('libsystemd', version: '>= 245', required: false)
dbus_dep = dependency('dbus-1', required: false)

# GThread
gthread_dep = dependency('gthread-2.0', version: '>= 2.0.0', required: false)

# Logic for SCIM_HAS_*
scim_has_gtk2 = gtk2_dep.found()
scim_has_gtk3 = gtk3_dep.found()
scim_has_gtk4 = gtk4_dep.found()
scim_has_qt3 = qt3_dep.found()
scim_has_qt4 = qt4_dep.found()
scim_has_qt5 = qt5_dep.found()
scim_has_clutter = clutter_dep.found() and clutter_im_dep.found()
scim_has_sd = libsystemd_dep.found()
scim_has_dbus = dbus_dep.found()
scim_has_gthread2 = gthread_dep.found()

# Configuration switches
if get_option('enable_debug')
  conf_data.set('ENABLE_DEBUG', 1)
  add_project_arguments('-g', language: ['c', 'cpp'])
endif

if get_option('hash-map')
  conf_data.set('ENABLE_HASH_MAP', 1)
endif

# Setup UI logic
scim_build_setup_ui = false
if get_option('setup-ui')
    # Need to select GTK version
    want_gtk_version = get_option('gtk-version')
    use_gtk_version = ''
    
    if want_gtk_version == 'auto'
        if scim_has_gtk3
            use_gtk_version = '3'
        elif scim_has_gtk2
            use_gtk_version = '2'
        elif scim_has_gtk4
            use_gtk_version = '4'
        endif
    else
        use_gtk_version = want_gtk_version
    endif

    has_gtk_for_setup = false
    setup_gtk_dep = dependency('', required: false)

    if use_gtk_version == '4' and scim_has_gtk4
        has_gtk_for_setup = true
        setup_gtk_dep = gtk4_dep
    elif use_gtk_version == '3' and scim_has_gtk3
        has_gtk_for_setup = true
        setup_gtk_dep = gtk3_dep
    elif use_gtk_version == '2' and scim_has_gtk2
        has_gtk_for_setup = true
        setup_gtk_dep = gtk2_dep
    endif

    if has_gtk_for_setup
        scim_build_setup_ui = true
        conf_data.set('HAVE_SCIM_SETUP', 1)
    endif
endif

# Tray Icon
enable_tray_icon = false
if (scim_has_gtk4 or scim_has_gtk3 or scim_has_gtk2) and x11_dep.found()
    enable_tray_icon = true
    conf_data.set('ENABLE_TRAY_ICON', 1)
endif

# Panel GTK
enable_panel_gtk = false
if get_option('panel-gtk').enabled()
  enable_panel_gtk = true
elif get_option('panel-gtk').auto()
  if scim_has_gthread2 and (scim_has_gtk2 or scim_has_gtk3 or scim_has_gtk4)
      enable_panel_gtk = true
  endif
endif

# Directories
prefix = get_option('prefix')
libdir = join_paths(prefix, get_option('libdir'))
datadir = join_paths(prefix, get_option('datadir'))
sysconfdir = join_paths(prefix, get_option('sysconfdir'))

scim_datadir = join_paths(datadir, 'scim')
scim_sysconfdir = sysconfdir
scim_icondir = join_paths(datadir, 'scim/icons')
scim_module_path = join_paths(libdir, 'scim' + scim_epoch)
scim_libexecdir = join_paths(libdir, 'scim' + scim_epoch)
scim_localedir = join_paths(datadir, 'locale')
scim_tempdir = '/tmp'
scim_prefix = prefix
scim_libdir = libdir

conf_data.set_quoted('SCIM_DATADIR', scim_datadir)
conf_data.set_quoted('SCIM_SYSCONFDIR', scim_sysconfdir)
conf_data.set_quoted('SCIM_ICONDIR', scim_icondir)
conf_data.set_quoted('SCIM_MODULE_PATH', scim_module_path)
conf_data.set_quoted('SCIM_LIBEXECDIR', scim_libexecdir)
conf_data.set_quoted('SCIM_LOCALEDIR', scim_localedir)
conf_data.set_quoted('SCIM_TEMPDIR', scim_tempdir)
conf_data.set_quoted('SCIM_PREFIX', scim_prefix)
conf_data.set_quoted('SCIM_LIBDIR', scim_libdir)

configure_file(output: 'config.h', configuration: conf_data)

# Include directories
inc = include_directories('.')
src_inc = include_directories('src')

# Pkgconfig
pkg = import('pkgconfig')

# I18n
i18n = import('i18n')

subdir('src')
subdir('utils')
subdir('modules')
subdir('configs')
subdir('extras')
subdir('data')

if get_option('tests')
  subdir('tests')
endif

if get_option('documents')
  subdir('docs')
endif

# Po
subdir('po')

pkg.generate(libscim,
  name: 'SCIM',
  description: 'Smart Common Input Method platform',
  version: scim_version,
  filebase: 'scim',
  variables: [
      'scim_binary_version=' + scim_binary_version,
      'scim_epoch=' + scim_epoch,
      'moduledir=${libdir}/scim' + scim_epoch,
      'icondir=${datadir}/scim/icons'
  ]
)
