inst_icons = [
  'pin-up.png', 'pin-down.png', 'full-letter.png', 'half-letter.png',
  'full-punct.png', 'half-punct.png', 'setup.png', 'help.png',
  'up.png', 'down.png', 'left.png', 'right.png', 'trademark.png',
  'rawcode.png', 'keyboard.png', 'menu.png'
]

sctc_filter_icons = ['sctc.png', 'sctc-sc-to-tc.png', 'sctc-tc-to-sc.png']

install_data(inst_icons, install_dir: scim_icondir)

if get_option('filter-sctc')
  install_data(sctc_filter_icons, install_dir: scim_icondir)
endif
