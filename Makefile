APP =		stm32f746g-disco
ARCH =		arm

CC =		${CROSS_COMPILE}gcc
LD =		${CROSS_COMPILE}ld
OBJCOPY =	${CROSS_COMPILE}objcopy

LDSCRIPT =	${.OBJDIR}/ldscript
LDSCRIPT_TPL =	${.CURDIR}/ldscript.tpl

OBJECTS =	gpio.o						\
		main.o						\
		osfive/sys/arm/stm/stm32l4_usart.o		\
		osfive/sys/arm/stm/stm32f4_flash.o		\
		osfive/sys/arm/stm/stm32f4_gpio.o		\
		osfive/sys/arm/stm/stm32f4_ltdc.o		\
		osfive/sys/arm/stm/stm32f4_pwr.o		\
		osfive/sys/arm/stm/stm32f4_rcc.o		\
		osfive/sys/arm/stm/stm32f4_fmc.o		\
		osfive/sys/arm/stm/stm32f4_timer.o		\
		osfive/sys/arm/stm/stm32f7_syscfg.o		\
		osfive/sys/arm/stm/stm32f7_eth.o		\
		osfive/sys/arm/arm/nvic.o			\
		osfive/sys/arm/arm/trap.o			\
		osfive/sys/arm/arm/exception.o			\
		osfive/sys/net/if.o				\
		osfive/sys/kern/kern_malloc_fl.o		\
		osfive/sys/kern/kern_mbuf.o			\
		osfive/sys/kern/subr_prf.o			\
		osfive/sys/kern/subr_console.o			\
		start.o

.include "osfive/lib/libfont/Makefile.inc"
.include "osfive/lib/libc/Makefile.inc"
# .include "osfive/lib/libaeabi/Makefile.inc"
# .include "osfive/lib/libFLAC/Makefile.inc"

CFLAGS =	-mthumb -mcpu=cortex-m7						\
		-O -nostdlib -fno-pic -fno-builtin-printf			\
		-pipe -g -nostdinc -fno-omit-frame-pointer			\
		-fno-optimize-sibling-calls -ffreestanding -fwrapv		\
		-fdiagnostics-show-option -fms-extensions 			\
		-Wall -Wredundant-decls -Wnested-externs -Wstrict-prototypes	\
		-Wmissing-prototypes -Wpointer-arith -Winline -Wcast-qual	\
		-Wundef -Wno-pointer-sign -Wno-format -Wmissing-include-dirs	\
		-Wno-unknown-pragmas -Werror

all: compile link binary

${LDSCRIPT}: ${LDSCRIPT_TPL}
	cp ${LDSCRIPT_TPL} ${LDSCRIPT}

clean:
	rm -f ${OBJECTS:M*} ${LDSCRIPT} ${APP}.elf ${APP}.srec

.include "osfive/mk/user.mk"
.include "osfive/mk/compile.mk"
.include "osfive/mk/link.mk"
.include "osfive/mk/binutils.mk"
