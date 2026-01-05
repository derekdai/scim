project('ltdl', 'c', version: '2.4.6')

ltdl_inc = include_directories('.')

libltdl = library('ltdl',
  'ltdl.c',
  include_directories: ltdl_inc,
  version: '7.3.1',
  install: false
)

ltdl_dep = declare_dependency(
  link_with: libltdl,
  include_directories: ltdl_inc
)
