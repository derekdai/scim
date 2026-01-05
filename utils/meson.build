# utils/meson.build

# scim-x11utils
if get_option('frontend-x11') and x11_dep.found()
  libscim_x11utils_sources = ['scim_x11_utils.cpp']

  libscim_x11utils = shared_library('scim-x11utils' + scim_epoch,
    libscim_x11utils_sources,
    dependencies : [libscim_dep, x11_dep],
    version : '@0@.@1@.@2@'.format(scim_current, scim_revision, scim_age),
    soversion : scim_current,
    install : true
  )

  libscim_x11utils_dep = declare_dependency(
    link_with : libscim_x11utils,
    dependencies : [libscim_dep, x11_dep],
    include_directories : [inc, include_directories('.')]
  )

  install_headers('scim_x11_utils.h', subdir : 'scim' + scim_epoch + '/x11')
endif

# scim-gtkutils
if get_option('panel-gtk').enabled() or (get_option('panel-gtk').auto() and gtk2_dep.found()) or get_option('setup-ui')
  # Assuming GTK2 or 3 is available, otherwise this logic needs refinement
  # configure.ac uses SCIM_BUILD_GTK_UTILS if SCIM_HAS_GTK is yes.

  gtk_dep_for_utils = dependency('', required : false)
  if gtk3_dep.found()
     gtk_dep_for_utils = gtk3_dep
  elif gtk2_dep.found()
     gtk_dep_for_utils = gtk2_dep
  endif

  if gtk_dep_for_utils.found()
    libscim_gtkutils_sources = ['scimstringview.c', 'scimkeyselection.cpp']

    # Tray icon
    if enable_tray_icon
        libscim_gtkutils_sources += ['scimtrayicon.c']
    endif

    # Add libscim_x11utils to dependencies if it exists, as scimkeyselection uses it
    gtkutils_deps = [libscim_dep, gtk_dep_for_utils, x11_dep]
    if is_variable('libscim_x11utils')
        gtkutils_deps += libscim_x11utils_dep
    endif

    libscim_gtkutils = shared_library('scim-gtkutils' + scim_epoch,
      libscim_gtkutils_sources,
      dependencies : gtkutils_deps,
      version : '@0@.@1@.@2@'.format(scim_current, scim_revision, scim_age),
      soversion : scim_current,
      install : true
    )

    libscim_gtkutils_dep = declare_dependency(
        link_with : libscim_gtkutils,
        dependencies : [libscim_dep, gtk_dep_for_utils],
        include_directories : [inc, include_directories('.')]
    )

    install_headers(['scimstringview.h', 'scimkeyselection.h'], subdir : 'scim' + scim_epoch + '/gtk')
    if enable_tray_icon
       install_headers('scimtrayicon.h', subdir : 'scim' + scim_epoch + '/gtk')
    endif
  endif
endif
