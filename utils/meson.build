# GTK Utils
if get_option('panel-gtk').enabled() or (get_option('panel-gtk').auto() and scim_has_gtk3) or get_option('setup-ui')
    gtk_utils_src = ['scimkeyselection.cpp']
    gtk_utils_headers = ['scimkeyselection.h']
    
    if enable_tray_icon
        gtk_utils_src += ['scimtrayicon.c']
        gtk_utils_headers += ['scimtrayicon.h']
    endif
    
    if scim_build_setup_ui or (enable_panel_gtk)
       libscim_gtkutils = library('scim-gtkutils-' + scim_epoch,
         gtk_utils_src,
         dependencies: [setup_gtk_dep, libscim_dep],
         install: true,
         version: '8.2.3',
         soversion: '8'
       )
       
       libscim_gtkutils_dep = declare_dependency(
         link_with: libscim_gtkutils,
         include_directories: include_directories('.')
       )
       
       install_headers(gtk_utils_headers, subdir: 'scim')

       pkg.generate(libscim_gtkutils,
         name: 'SCIM GTK Utils',
         description: 'SCIM GTK Utility Library',
         version: scim_version,
         requires: ['scim', 'gtk+-3.0'] # Should match actual gtk used
       )
    endif
endif

# X11 Utils
if get_option('frontend-x11')
    libscim_x11utils = library('scim-x11utils-' + scim_epoch,
        ['scim_x11_utils.cpp'],
        dependencies: [x11_dep, libscim_dep],
        install: true,
        version: '8.2.3',
        soversion: '8'
    )
    
    libscim_x11utils_dep = declare_dependency(
        link_with: libscim_x11utils,
        include_directories: include_directories('.')
    )
    
    install_headers('scim_x11_utils.h', subdir: 'scim')

    pkg.generate(libscim_x11utils,
         name: 'SCIM X11 Utils',
         description: 'SCIM X11 Utility Library',
         version: scim_version,
         requires: ['scim', 'x11']
    )
endif
