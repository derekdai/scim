subdir('icons')
subdir('pixmaps')

build_frontend_ibus = get_option('frontend-ibus') and scim_has_sd and scim_has_dbus

if build_frontend_ibus
  configure_file(
    input: '../org.freedesktop.portal.IBus.service.in',
    output: 'org.freedesktop.portal.IBus.service',
    configuration: {'SCIM_LIBEXECDIR': scim_libexecdir},
    install: true,
    install_dir: join_paths(datadir, 'dbus-1', 'services')
  )
endif
