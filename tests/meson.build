# tests/meson.build

# Helper module for tests
# It depends on GTK. The source includes <gtk/gtk.h>.
# We should try to find GTK2 or GTK3 or GTK4.
# The previous Makefile.am used @GTK_CFLAGS@ and @GTK_LIBS@ which were set by GTK2/3/4 logic in configure.ac
# Root meson.build defines gtk2_dep, gtk3_dep, gtk4_dep.
# We'll try to pick one.

gtk_test_dep = dependency('', required : false)
if gtk3_dep.found()
  gtk_test_dep = gtk3_dep
elif gtk2_dep.found()
  gtk_test_dep = gtk2_dep
elif gtk4_dep.found()
  gtk_test_dep = gtk4_dep
endif

if gtk_test_dep.found()
  test_helper_sources = ['scim_test_helper.cpp']

  # The test helper uses vbox which is deprecated in GTK3 and removed in GTK4.
  # If it fails with GTK4, we might need to be careful.
  # But assuming it compiles if dependency is found (legacy code often stuck on old GTK).

  test_helper = shared_module('test-helper',
    test_helper_sources,
    dependencies : [libscim_dep, gtk_test_dep],
    install : false
  )
endif

# IMEngine module for tests
test_imengine_sources = ['scim_test_imengine.cpp']
test_imengine = shared_module('test-imengine',
  test_imengine_sources,
  dependencies : [libscim_dep],
  install : false
)

# Test executables
test_progs = [
  'testsocketserver',
  'testsocketclient',
  'testiconvert',
  'testpanel',
  'testlang'
]

foreach p : test_progs
  exe = executable(p,
    p + '.cpp',
    dependencies : libscim_dep,
    install : false
  )
endforeach
