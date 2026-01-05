# extras/immodules/client-qt/qt4/meson.build
if qt4_dep.found()
  qt4 = import('qt4')

  # Qt4 module usage for moc
  # But we are building a module, not a full app.
  # We can use qt4.preprocess

  qt4_moc_sources = qt4.preprocess(
      moc_headers : ['../scim-bridge-client-qt.h', '../im-scim-bridge-qt.h'],
      include_directories : [include_directories('../../common'), include_directories('../../client-common')]
  )

  im_scim_qt4_sources = [
    '../im-scim-bridge-qt.cpp',
    '../scim-bridge-client-qt.cpp',
    '../scim-bridge-client-imcontext-qt.cpp',
    '../scim-bridge-client-key-event-utility-qt.cpp'
  ]

  if get_option('qt4-im-module-dir') != ''
      qt4_im_moduledir = get_option('qt4-im-module-dir')
  else
      qt4_im_moduledir = get_option('libdir') / 'qt4' / 'plugins' / 'inputmethods'
  endif

  shared_module('im-scim',
    im_scim_qt4_sources + qt4_moc_sources,
    dependencies : [
      libscim_bridge_common_dep,
      libscim_bridge_client_common_dep,
      qt4_dep
    ],
    include_directories : [
      include_directories('../../common'),
      include_directories('../../client-common')
    ],
    cpp_args : ['-DQT_IMMODULE', '-fPIC', '-DQT_SHARED'],
    install : true,
    install_dir : qt4_im_moduledir
  )
endif
