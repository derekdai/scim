project('ltdl', 'c')

ltdl_inc = include_directories('.')
ltdl_lib = static_library('ltdl', 'ltdl.c')

ltdl_dep = declare_dependency(
  link_with : ltdl_lib,
  include_directories : ltdl_inc
)
