# stm32f746g-disco
Driving LTDC panel on STM32F746G-DISCO 

Connect UART pins as follows:

| STM32F469I-DISCO  | UART-to-USB adapter  |
| ----------------- | -------------------- |
| D0 (USART6_RX)    | TX                   |
| D1 (USART6_TX)    | RX                   |

UART baud rate: 115200

### Build under Linux

    $ export CROSS_COMPILE=arm-none-eabi-
    $ git clone --recursive https://github.com/osfive/stm32f746g-disco
    $ cd stm32f746g-disco
    $ bmake

### Build under FreeBSD

    $ setenv CROSS_COMPILE arm-none-eabi-
    $ git clone --recursive https://github.com/osfive/stm32f746g-disco
    $ cd stm32f746g-disco
    $ make

### Program font
    $ sudo openocd -s /path/to/openocd/tcl -f interface/stlink-v2-1.cfg -f target/stm32f7x.cfg -c 'program fonts/ter-x24n.pcf reset 0x080b0000 exit'

### Program osfive
    $ sudo openocd -s /path/to/openocd/tcl -f interface/stlink-v2-1.cfg -f target/stm32f7x.cfg -c 'program obj/stm32f746g-disco.bin reset 0x08000000 exit'
