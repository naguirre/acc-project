project('chain-item', 'c', default_options : ['buildtype=minsize','b_lto=true'])

assert(meson.is_cross_build(), 'This projects can only be built in a cross build environment.')

exe = executable('chain-item', 'main.c', 'suart.c')

objcopy = find_program('avr-objcopy')

eep = custom_target('chain-item.eep',
    input : exe,
    output : 'chain-item.eep',
    command : [objcopy, '-O', 'ihex', '-j', '.eeprom',
          '--set-section-flags=.eeprom=alloc,load',
          '--no-change-warnings', '--change-section-lma',
          '.eeprom=0', '@INPUT@', '@OUTPUT@'
      ]
)

hex = custom_target('chain-item.hex',
    input : exe,
    output : 'chain-item.hex',
    command : [objcopy, '-O', 'ihex', '-R', '.eeprom',
        '@INPUT@', '@OUTPUT@',
    ],
    depends : eep,
)
