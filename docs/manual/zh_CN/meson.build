# docs/manual/zh_CN/meson.build
subdir('figures')

custom_target('user-manual-zh_CN',
  input : 'user-manual.xml',
  output : 'user-manual.html',
  command : [xsltproc, '-o', '@OUTPUT@', docbook_xsl, '@INPUT@'],
  install : true,
  install_dir : join_paths(datadir, 'doc', 'scim', 'manual', 'zh_CN')
)
