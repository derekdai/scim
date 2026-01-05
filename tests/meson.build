test_progs = [
  'testsocketserver', 'testsocketclient', 'testiconvert',
  'testpanel', 'testlang'
]

# We need libscim dependency which should be available from src/meson.build or root
# In root meson.build: subdir('src') comes before subdir('tests')

foreach t : test_progs
  executable(t, t + '.cpp',
    include_directories: [inc, src_inc],
    dependencies: [libscim_dep],
    install: false
  )
endforeach

# Helper module for tests
helper_mod_dir = join_paths(scim_libdir, 'scim' + scim_epoch, scim_binary_version, 'Helper')

# Only build test-helper if we have GTK
if scim_has_gtk2 or scim_has_gtk3 or scim_has_gtk4
    test_gtk_dep = dependency('', required: false)
    if scim_has_gtk3
        test_gtk_dep = gtk3_dep
    elif scim_has_gtk2
        test_gtk_dep = gtk2_dep
    elif scim_has_gtk4
        test_gtk_dep = gtk4_dep
    endif

    library('test-helper',
        ['scim_test_helper.cpp'],
        include_directories: [inc, src_inc],
        dependencies: [libscim_dep, test_gtk_dep],
        install: true,
        install_dir: helper_mod_dir,
        name_prefix: '',
        override_options: ['b_lundef=false']
    )
endif

# IMEngine module for tests
imengine_mod_dir = join_paths(scim_libdir, 'scim' + scim_epoch, scim_binary_version, 'IMEngine')
library('test-imengine',
    ['scim_test_imengine.cpp'],
    include_directories: [inc, src_inc],
    dependencies: [libscim_dep],
    install: true,
    install_dir: imengine_mod_dir,
    name_prefix: '',
    override_options: ['b_lundef=false']
)
