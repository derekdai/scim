# extras/immodules/doc/meson.build
if get_option('documents')
  doxygen = find_program('doxygen', required : false)
  if doxygen.found()
    # It runs doxygen on doxygen.conf from the parent directory context?
    # "cd ..; doxygen doc/doxygen.conf"
    # This implies the input paths in doxygen.conf are relative to ..

    # We can use custom_target
    custom_target('immodules-api-docs',
      input : 'doxygen.conf',
      output : 'api',
      command : [doxygen, '@INPUT@'], # We might need to handle the working directory
      install : false # Makefile.am doesn't seem to install it, just builds it
    )
  endif
endif
