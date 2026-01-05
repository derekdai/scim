if enable_panel_gtk
    executable('scim-panel-gtk',
        'scim_panel_gtk.cpp',
        include_directories: [inc, src_inc, include_directories('../../utils')],
        dependencies: [libscim_dep, setup_gtk_dep, gthread_dep, libscim_gtkutils_dep],
        install: true,
        install_dir: scim_libexecdir,
        cpp_args: [
            '-DSCIM_DATADIR="@0@"'.format(scim_datadir),
            '-DSCIM_LOCALEDIR="@0@"'.format(scim_localedir),
            '-DSCIM_SYSCONFDIR="@0@"'.format(scim_sysconfdir),
            '-DSCIM_LIBEXECDIR="@0@"'.format(scim_libexecdir),
            '-DSCIM_ICONDIR="@0@"'.format(scim_icondir),
            '-DSCIM_MODULE_PATH="@0@"'.format(scim_module_path),
            '-DSCIM_TEMPDIR="@0@"'.format(scim_tempdir),
            '-DSCIM_KEYBOARD_ICON_FILE="@0@"'.format(join_paths(scim_icondir, 'keyboard.png'))
        ]
    )
endif
