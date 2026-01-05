# docs/meson.build

# Man pages
man_pages = [
  'scim.1',
  'scim-config-agent.1',
  'scim-im-agent.1',
  'scim-setup.1'
]
install_man(man_pages)

# Doxygen documentation
if get_option('documents')
  doxygen = find_program('doxygen', required : false)
  dot = find_program('dot', required : false)
  perl = find_program('perl', required : false)

  if doxygen.found()
    cdata = configuration_data()
    cdata.set('PACKAGE', 'scim')
    cdata.set('VERSION', scim_version)
    cdata.set('top_srcdir', meson.project_source_root())
    if perl.found()
        cdata.set('PERL', perl.path())
    else
        cdata.set('PERL', '')
    endif

    if dot.found()
      cdata.set('HAVE_GRAPHVIZ', 'YES')
      cdata.set('DOT', dot.path())
    else
      cdata.set('HAVE_GRAPHVIZ', 'NO')
      cdata.set('DOT', '')
    endif

    scim_cfg = configure_file(
      input : 'scim.cfg.in',
      output : 'scim.cfg',
      configuration : cdata
    )

    custom_target('doxygen-docs',
      input : scim_cfg,
      output : 'html',
      command : [doxygen, scim_cfg],
      install : true,
      install_dir : join_paths(datadir, 'doc', 'scim', 'html')
    )
  endif

  # Manual generation
  xsltproc = find_program('xsltproc', required : false)

  if xsltproc.found()
      docbook_xsl = ''
      # Mimic configure.ac path checking
      check_paths = [
        '/usr/share/sgml/docbook/xsl-stylesheets/html/tldp-html.xsl',
        '/usr/share/sgml/docbook/xsl-stylesheets/html/docbook.xsl',
        '/usr/share/xml/docbook/stylesheet/nwalsh/current/html/docbook.xsl'
      ]

      fs = import('fs')
      foreach p : check_paths
        if fs.exists(p)
           docbook_xsl = p
           break
        endif
      endforeach

      if docbook_xsl != ''
         subdir('manual')
      else
         message('DocBook XSL stylesheets not found, skipping manual generation.')
      endif
  endif
endif
