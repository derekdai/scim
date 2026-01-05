if get_option('documents')
    doxygen = find_program('doxygen', required: false)
    if doxygen.found()
        configure_file(input: 'scim.cfg.in',
            output: 'scim.cfg',
            configuration: conf_data)

        # custom_target for docs?
        # Autotools does this with a rule:
        # (cd $(top_builddir)/docs; rm -fr html/*; $(DOXYGEN) scim.cfg; tar -czf scim-doc-$(SCIM_VERSION).tar.gz html)
        # We can replicate this if needed, but often in Meson we just let it be or install if generated.
        # But 'documents' usually means generating API docs.

        # For now, let's just note it's possible.
        # Given the instruction 1:1, maybe we should create a custom target.

        custom_target('scim-doc',
            input: 'scim.cfg.in',
            output: 'scim-doc-' + scim_version + '.tar.gz',
            command: [
                'sh', '-c',
                'cd @OUTDIR@ && rm -fr html/* && ' + doxygen.path() + ' ' + join_paths(meson.current_build_dir(), 'scim.cfg') + ' && tar -czf @OUTPUT@ html'
            ],
            depends: [], # generated scim.cfg?
            install: false # Autotools doesn't seem to install it automatically, just 'make html'
        )
    else
        warning('Doxygen not found, documentation will not be built.')
    endif
endif

# Autotools does NOT install man pages from docs/Makefile.am, so we don't either.
