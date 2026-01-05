# extras/immodules/client-qt/qt3/meson.build
if qt3_dep.found()
  # Meson has a qt3 module but it might be deprecated or behave differently.
  # Assuming we use the qt3_dep found in root meson.build

  # MOC handling
  # We need to find moc, probably using find_program if not provided by dependency
  # configure.ac allows specifying it.
  # root meson.build uses pkg-config to find qt3 but doesn't explicitly look for moc.

  qt3_moc = find_program('moc-qt3', 'moc', required : false) # Adjust name as needed

  # Since Meson's qt support is mostly for qt4/5/6, manual moc might be needed or generator

  if qt3_moc.found()
      moc_gen = generator(qt3_moc,
        output  : 'moc_@BASENAME@.cpp',
        arguments : ['@INPUT@', '-o', '@OUTPUT@'])

      qt3_moc_sources = moc_gen.process('../scim-bridge-client-qt.h', '../im-scim-bridge-qt.h')
  else
      qt3_moc_sources = []
  endif

  im_scim_qt3_sources = [
    '../im-scim-bridge-qt.cpp',
    '../scim-bridge-client-qt.cpp',
    '../scim-bridge-client-imcontext-qt.cpp',
    '../scim-bridge-client-key-event-utility-qt.cpp'
  ]

  qt3_im_moduledir = get_option('libdir') / 'qt3' / 'plugins' / 'inputmethods' # Default guess

  shared_module('im-scim',
    im_scim_qt3_sources + qt3_moc_sources,
    dependencies : [
      libscim_bridge_common_dep,
      libscim_bridge_client_common_dep,
      qt3_dep
    ],
    include_directories : [
      include_directories('../../common'),
      include_directories('../../client-common')
    ],
    cpp_args : ['-DQT_IMMODULE', '-fPIC'],
    install : true,
    install_dir : qt3_im_moduledir
  )
endif
