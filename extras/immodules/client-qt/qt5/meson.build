# extras/immodules/client-qt/qt5/meson.build
if qt5_dep.found()
  qt5 = import('qt5')

  qt5_moc_sources = qt5.preprocess(
      moc_headers : ['../scim-bridge-client-qt.h', '../im-scim-bridge-qt.h'],
      include_directories : [include_directories('../../common'), include_directories('../../client-common')]
  )

  im_scim_qt5_sources = [
    '../im-scim-bridge-qt.cpp',
    '../scim-bridge-client-qt.cpp',
    '../scim-bridge-client-imcontext-qt.cpp',
    '../scim-bridge-client-key-event-utility-qt.cpp'
  ]

  if get_option('qt5-im-module-dir') != ''
      qt5_im_moduledir = get_option('qt5-im-module-dir')
  else
      qt5_im_moduledir = get_option('libdir') / 'qt5' / 'plugins' / 'platforminputcontexts'
  endif

  shared_module('im-scim',
    im_scim_qt5_sources + qt5_moc_sources,
    dependencies : [
      libscim_bridge_common_dep,
      libscim_bridge_client_common_dep,
      qt5_dep,
      qt5_widgets_dep,
      qt5_x11extras_dep
    ],
    include_directories : [
      include_directories('../../common'),
      include_directories('../../client-common')
    ],
    cpp_args : ['-DQT_IMMODULE', '-fPIC'],
    install : true,
    install_dir : qt5_im_moduledir
  )
endif
