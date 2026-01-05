if get_option('clutter-immodule')
    if scim_has_clutter
        clutter_im_mod_dir = clutter_dep.get_pkgconfig_variable('immodules_dir')

        opt_clutter_dir = get_option('clutter-im-module-dir')
        if opt_clutter_dir != ''
            clutter_im_mod_dir = opt_clutter_dir
        else
             # Logic from configure.ac
             clutter_libdir = clutter_im_dep.get_pkgconfig_variable('libdir')
             clutter_im_mod_dir = join_paths(clutter_libdir, 'clutter-imcontext/immodules')
        endif

        library('im-scim',
            ['im-scim-bridge-clutter.c',
             'scim-bridge-client-clutter.c',
             'scim-bridge-client-imcontext-clutter.c',
             'scim-bridge-client-key-event-utility-clutter.c'],
            include_directories: [
                inc,
                include_directories('../common'),
                include_directories('../client-common')
            ],
            dependencies: [clutter_dep, clutter_im_dep, scim_bridge_common_dep, scim_bridge_client_common_dep],
            install: true,
            install_dir: clutter_im_mod_dir,
            name_prefix: '',
            override_options: ['b_lundef=false']
        )
    endif
endif
