Contains the code for the chain-item hardware.
software uart code is largely inspired by https://www.avrfreaks.net/projects/software-uart-fifo?module=Freaks%20Academy&func=viewItem&item_id=1895&item_type=project
changed to use TMR0 instead of TMR1 and change FIFO size to fit into attiny10
This code is intented to run on attiny-10 microcontroller

*** NOT TESTED YET ***

To build, you need meson and run :
meson --cross-file attiny10_cross.txt build
