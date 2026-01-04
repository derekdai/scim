imdkit_sources = [
  'FrameMgr.c',
  'i18nAttr.c',
  'i18nClbk.c',
  'i18nIc.c',
  'i18nIMProto.c',
  'i18nMethod.c',
  'i18nOffsetCache.c',
  'i18nPtHdr.c',
  'i18nUtil.c',
  'i18nX.c',
  'IMConn.c',
  'IMMethod.c',
  'IMValues.c'
]

libimdkit = static_library('imdkit',
  imdkit_sources,
  dependencies: x11_dep,
  include_directories: ['.']
)

imdkit_dep = declare_dependency(
  link_with: libimdkit,
  include_directories: include_directories('.')
)
