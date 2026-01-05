# extras/immodules/client-clutter/meson.build
if clutter_dep.found() and clutter_imcontext_dep.found()
  if get_option('clutter-im-module-dir') != ''
      clutter_im_moduledir = get_option('clutter-im-module-dir')
  else
      clutter_im_moduledir = clutter_imcontext_dep.get_variable(pkgconfig : 'immodulesdir', default_value : get_option('libdir') / 'clutter-imcontext' / 'immodules')
  endif

  shared_module('im-scim-bridge-clutter',
    'scim-bridge-client-immodule-clutter.c',
    dependencies : [libscim_bridge_client_common_dep, clutter_dep, clutter_imcontext_dep],
    install : true,
    install_dir : clutter_im_moduledir
  )
endif
