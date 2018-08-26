/*-
 * Copyright (c) 2018 Ruslan Bukin <br@bsdpad.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/console.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/lock.h>

#include <sys/mbuf.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#include <machine/frame.h>

#include <dev/display/panel.h>
#include <dev/display/dsi.h>

#include <arm/stm/stm32f7.h>
#include <arm/arm/nvic.h>

#include <libfont/libfont.h>

#include "gpio.h"

#define	NVIC_NINTRS	128
#define	FB_BASE		0xC0000000
#define	FONT_ADDR	0x080b0000
#define	DISPLAY_WIDTH	480
#define	DISPLAY_HEIGHT	272

extern uint32_t _sbss;
extern uint32_t _ebss;

static struct global_data {
	uint32_t ptr;
	struct font_info font;
} g_data;

struct stm32l4_usart_softc usart_sc;
struct stm32f4_gpio_softc gpio_sc;
struct stm32f4_fmc_softc fmc_sc;
struct stm32f4_flash_softc flash_sc;
struct stm32f4_pwr_softc pwr_sc;
struct stm32f4_rcc_softc rcc_sc;
struct stm32f7_eth_softc eth_sc;
struct stm32f7_syscfg_softc syscfg_sc;
struct stm32f4_ltdc_softc ltdc_sc;
struct stm32f4_timer_softc timer_sc;
struct arm_nvic_softc nvic_sc;

void app_main(void);

static const struct nvic_intr_entry nvic_intr_map[NVIC_NINTRS] = {
	[28] = { stm32f4_timer_intr, &timer_sc },
	[61] = { stm32f7_eth_intr, &eth_sc },
	[62] = { stm32f7_eth_wkup_intr, &eth_sc },
}; 

static const struct sdram sdram_entry = {
	.sdrtr = 0x603,
	.nrfs = 8,
	.bank1 = {
		.sdcr = { 0, 1, 2, 0, 2, 1, 1, 1, 0 },
		.sdtr = { 2, 2, 2, 7, 4, 7, 2 },
		.sdcmr = { 1 },
	},
};

static const struct layer_info layers[1] = {{
	.width = DISPLAY_WIDTH,
	.height = DISPLAY_HEIGHT,
	.hsync = 41,
	.hfp = 32,
	.hbp = 13,
	.vsync = 10,
	.vfp = 2,
	.vbp = 2,
	.bpp = 16,
	.base = FB_BASE,
}};

void
udelay(uint32_t usec)
{
	int i;

	/* TODO: implement me */

	for (i = 0; i < usec * 100; i++)
		;
}

void
usleep(uint32_t usec)
{

	stm32f4_timer_usleep(&timer_sc, usec);
}

static void
draw_pixel(void *arg, int x, int y, int pixel)
{
	uint16_t *addr;

	addr = (uint16_t *)(g_data.ptr + DISPLAY_WIDTH * y * 2 + x * 2);

	if (pixel)
		*addr = 0xffff;
	else
		*addr = 0;
}

static void
draw_text_utf8(uint8_t *s)
{
	struct char_info ci;
	uint8_t *newptr;
	uint8_t *buf;
	int c;

	g_data.ptr = FB_BASE;

	buf = (uint8_t *)s;

	for (;;) {
		c = utf8_to_ucs2(buf, &newptr);
		if (c == -1)
			return;
		buf = newptr;
		get_char_info(&g_data.font, c, &ci);
		draw_char(&g_data.font, c);
		g_data.ptr += ci.xsize * 2;
	}
}

static void
uart_putchar(int c, void *arg)
{
	struct stm32l4_usart_softc *sc;
 
	sc = arg;
 
	if (c == '\n')
		stm32l4_usart_putc(sc, '\r');

	stm32l4_usart_putc(sc, c);
}

static void
sdram_memtest(void)
{
	uint32_t *addr;
	int i;

	addr = (uint32_t *)FB_BASE;

	for (i = 0; i < (1024 * 1024); i++) {
		/* Test */
		*(volatile uint32_t *)(addr + i) = 0xaaaaaaaa;
		if (*(volatile uint32_t *)(addr + i) != 0xaaaaaaaa) {
			printf("sdram test failed %x\n",
			    *(volatile uint32_t *)(addr + i));
				while (1);
		}
	}

	printf("sdram test completed\n");
}

static void
display_clear(void)
{
	uint32_t p;
	uint8_t *b;

	b = (uint8_t *)FB_BASE;
	for (p = 0; p < (DISPLAY_WIDTH * DISPLAY_HEIGHT * 2); p++)
		*(b + p) = 0;
}

static void
eth_setup(void)
{
	struct in_addr in;

	/* Ethernet */

	if_init();
	stm32f7_eth_init(&eth_sc, ETH_BASE);
	stm32f4_rcc_eth_reset(&rcc_sc);
	stm32f7_syscfg_init(&syscfg_sc, SYSCFG_BASE);
	stm32f7_syscfg_eth_rmii(&syscfg_sc);
	udelay(10000);
	if (stm32f7_eth_setup(&eth_sc, NULL) != 0)
		return;

	/* Setup some IP address */
	in.s_addr = htonl(0x0a020002);
	in_aifaddr(eth_sc.ifp, in, 0xffffff00);

	arm_nvic_enable_intr(&nvic_sc, 61);
	arm_nvic_enable_intr(&nvic_sc, 62);
}

static void
app_init(void)
{
	uint32_t reg;

	stm32f4_flash_init(&flash_sc, FLASH_BASE);
	stm32f4_pwr_init(&pwr_sc, PWR_BASE);

	stm32f4_rcc_init(&rcc_sc, RCC_BASE);
	stm32f4_rcc_pll_configure(&rcc_sc, 25, 432, 9,
	    0, 1, (CFGR_PPRE2_2 | CFGR_PPRE1_4));

	reg = (GPIOAEN | GPIOBEN | GPIOCEN);
	reg |= (GPIODEN | GPIOEEN | GPIOFEN);
	reg |= (GPIOGEN | GPIOHEN | GPIOIEN);
	reg |= (GPIOJEN | GPIOKEN);
	reg |= (ETHMACRXEN | ETHMACTXEN | ETHMACEN);

	stm32f4_flash_setup(&flash_sc);
	stm32f4_rcc_setup(&rcc_sc, reg, 0, FMCEN,
	    (PWREN | TIM2EN), (USART6EN | SYSCFGEN | LTDCEN));
	stm32f4_rcc_pllsai(&rcc_sc, 192, 4, 5);

	stm32f4_gpio_init(&gpio_sc, GPIO_BASE);
	gpio_config(&gpio_sc);

	stm32f4_fmc_init(&fmc_sc, FMC_BASE);
	stm32f4_fmc_setup(&fmc_sc, &sdram_entry);

	arm_nvic_init(&nvic_sc, NVIC_BASE);
	arm_nvic_install_intr_map(&nvic_sc, nvic_intr_map);
	arm_nvic_enable_intr(&nvic_sc, 28);

	stm32l4_usart_init(&usart_sc, USART6_BASE, 108000000, 115200);
	console_register(uart_putchar, (void *)&usart_sc);

	/* (216/4) * 2 = 108MHz / 1 PSC = 108 */
	stm32f4_timer_init(&timer_sc, TIM2_BASE, 108000000);

	stm32f4_ltdc_init(&ltdc_sc, LTDC_BASE);
	stm32f4_ltdc_setup(&ltdc_sc, layers, 1);

	g_data.font.draw_pixel = draw_pixel;

	fl_init();
	fl_add_region(0x20010000, 240*1024);

	eth_setup();
}

static void
clear_bss(void)
{
	uint32_t *sbss;
	uint32_t *ebss;

	sbss = (uint32_t *)&_sbss;
	ebss = (uint32_t *)&_ebss;
	while (sbss < ebss)
		*sbss++ = 0;
}

void
app_main(void)
{
	uint8_t text[64];
	int err;
	int i;

	clear_bss();

	app_init();

	sdram_memtest();
	display_clear();

	printf("Hello World!\n");

	err = font_init(&g_data.font, (uint8_t *)FONT_ADDR);
	if (err != 0) {
		while (1) {
			printf("Error: font not found\n");
			usleep(1000000);
		}
	}

	i = 0;

	while (1) {
		sprintf(text, "привет %d", i);
		draw_text_utf8(text);
		i += 1;
		usleep(100000);
	}
}

void *
malloc(size_t size)
{
	void *ret;

	spinlock_enter();
	ret = fl_malloc(size);
	spinlock_exit();

	return (ret);
}

void
free(void *ptr)
{

	spinlock_enter();
	fl_free(ptr);
	spinlock_exit();
}

void *
calloc(size_t number, size_t size)
{

	return (fl_calloc(number, size));
}

void *
realloc(void *ptr, size_t size)
{

	return (fl_realloc(ptr, size));
}
