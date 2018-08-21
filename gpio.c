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

#include <arm/stm/stm32f4_gpio.h>

#include "gpio.h"

static const struct gpio_pin pins_uart[] = {
	{ PORT_C,  6, MODE_ALT, 8, FLOAT }, /* USART6_TX, D1 */
	{ PORT_C,  7, MODE_ALT, 8, FLOAT }, /* USART6_RX, D0 */

	PINS_END
};

static const struct gpio_pin pins_dram[] = {
	{ PORT_F,  0, MODE_ALT, 12, PULLUP }, /* A0 */
	{ PORT_F,  1, MODE_ALT, 12, PULLUP }, /* A1 */
	{ PORT_F,  2, MODE_ALT, 12, PULLUP }, /* A2 */
	{ PORT_F,  3, MODE_ALT, 12, PULLUP }, /* A3 */
	{ PORT_F,  4, MODE_ALT, 12, PULLUP }, /* A4 */
	{ PORT_F,  5, MODE_ALT, 12, PULLUP }, /* A5 */
	{ PORT_F, 12, MODE_ALT, 12, PULLUP }, /* A6 */
	{ PORT_F, 13, MODE_ALT, 12, PULLUP }, /* A7 */
	{ PORT_F, 14, MODE_ALT, 12, PULLUP }, /* A8 */
	{ PORT_F, 15, MODE_ALT, 12, PULLUP }, /* A9 */
	{ PORT_G,  0, MODE_ALT, 12, PULLUP }, /* A10 */
	{ PORT_G,  1, MODE_ALT, 12, PULLUP }, /* A11 */

	{ PORT_D, 14, MODE_ALT, 12, PULLUP }, /* FMC_D0 */
	{ PORT_D, 15, MODE_ALT, 12, PULLUP }, /* FMC_D1 */
	{ PORT_D,  0, MODE_ALT, 12, PULLUP }, /* FMC_D2 */
	{ PORT_D,  1, MODE_ALT, 12, PULLUP }, /* FMC_D3 */
	{ PORT_E,  7, MODE_ALT, 12, PULLUP }, /* FMC_D4 */
	{ PORT_E,  8, MODE_ALT, 12, PULLUP }, /* FMC_D5 */
	{ PORT_E,  9, MODE_ALT, 12, PULLUP }, /* FMC_D6 */
	{ PORT_E, 10, MODE_ALT, 12, PULLUP }, /* FMC_D7 */
	{ PORT_E, 11, MODE_ALT, 12, PULLUP }, /* FMC_D8 */
	{ PORT_E, 12, MODE_ALT, 12, PULLUP }, /* FMC_D9 */
	{ PORT_E, 13, MODE_ALT, 12, PULLUP }, /* FMC_D10 */
	{ PORT_E, 14, MODE_ALT, 12, PULLUP }, /* FMC_D11 */
	{ PORT_E, 15, MODE_ALT, 12, PULLUP }, /* FMC_D12 */
	{ PORT_D,  8, MODE_ALT, 12, PULLUP }, /* FMC_D13 */
	{ PORT_D,  9, MODE_ALT, 12, PULLUP }, /* FMC_D14 */
	{ PORT_D, 10, MODE_ALT, 12, PULLUP }, /* FMC_D15 */

	{ PORT_H,  5, MODE_ALT, 12, PULLUP }, /* FMC_SDNWE */
	{ PORT_F, 11, MODE_ALT, 12, PULLUP }, /* FMC_SDNRAS */
	{ PORT_G, 15, MODE_ALT, 12, PULLUP }, /* FMC_SDNCAS */
	{ PORT_G,  8, MODE_ALT, 12, PULLUP }, /* FMC_SDCLK */
	{ PORT_G,  4, MODE_ALT, 12, PULLUP }, /* FMC_BA0 */
	{ PORT_G,  5, MODE_ALT, 12, PULLUP }, /* FMC_BA1 */
	{ PORT_H,  3, MODE_ALT, 12, PULLUP }, /* FMC_SDNE0 */
	{ PORT_C,  3, MODE_ALT, 12, PULLUP }, /* FMC_SDCKE0 */
	{ PORT_E,  0, MODE_ALT, 12, PULLUP }, /* FMC_NBL0 */
	{ PORT_E,  1, MODE_ALT, 12, PULLUP }, /* FMC_NBL1 */

	PINS_END
};

static const struct gpio_pin pins_ltdc[] = {
	{ PORT_I, 15, MODE_ALT, 14, PULLUP }, /* LCD_R0 */
	{ PORT_J,  0, MODE_ALT, 14, PULLUP }, /* LCD_R1 */
	{ PORT_J,  1, MODE_ALT, 14, PULLUP }, /* LCD_R2 */
	{ PORT_J,  2, MODE_ALT, 14, PULLUP }, /* LCD_R3 */
	{ PORT_J,  3, MODE_ALT, 14, PULLUP }, /* LCD_R4 */
	{ PORT_J,  4, MODE_ALT, 14, PULLUP }, /* LCD_R5 */
	{ PORT_J,  5, MODE_ALT, 14, PULLUP }, /* LCD_R6 */
	{ PORT_J,  6, MODE_ALT, 14, PULLUP }, /* LCD_R7 */

	{ PORT_J,  7, MODE_ALT, 14, PULLUP }, /* LCD_G0 */
	{ PORT_J,  8, MODE_ALT, 14, PULLUP }, /* LCD_G1 */
	{ PORT_J,  9, MODE_ALT, 14, PULLUP }, /* LCD_G2 */
	{ PORT_J, 10, MODE_ALT, 14, PULLUP }, /* LCD_G3 */
	{ PORT_J, 11, MODE_ALT, 14, PULLUP }, /* LCD_G4 */
	{ PORT_K,  0, MODE_ALT, 14, PULLUP }, /* LCD_G5 */
	{ PORT_K,  1, MODE_ALT, 14, PULLUP }, /* LCD_G6 */
	{ PORT_K,  2, MODE_ALT, 14, PULLUP }, /* LCD_G7 */

	{ PORT_E,  4, MODE_ALT, 14, PULLUP }, /* LCD_B0 */
	{ PORT_J, 13, MODE_ALT, 14, PULLUP }, /* LCD_B1 */
	{ PORT_J, 14, MODE_ALT, 14, PULLUP }, /* LCD_B2 */
	{ PORT_J, 15, MODE_ALT, 14, PULLUP }, /* LCD_B3 */
	{ PORT_G, 12, MODE_ALT,  9, PULLUP }, /* LCD_B4 */
	{ PORT_K,  4, MODE_ALT, 14, PULLUP }, /* LCD_B5 */
	{ PORT_K,  5, MODE_ALT, 14, PULLUP }, /* LCD_B6 */
	{ PORT_K,  6, MODE_ALT, 14, PULLUP }, /* LCD_B7 */

	{ PORT_I,  9, MODE_ALT, 14, PULLUP }, /* LCD_VSYNC */
	{ PORT_I, 10, MODE_ALT, 14, PULLUP }, /* LCD_HSYNC */
	{ PORT_K,  7, MODE_ALT, 14, PULLUP }, /* LCD_ENABLE */
	{ PORT_I, 14, MODE_ALT, 14, PULLUP }, /* LCD_DOTCLK */

	{ PORT_I, 12, MODE_OUT, 0, FLOAT }, /* LCD_DISP */
	{ PORT_K,  3, MODE_OUT, 0, FLOAT }, /* LCD_BL_CTRL */

	PINS_END
};

static const struct gpio_pin pins_eth[] = {
	{ PORT_G,  11, MODE_ALT, 11, FLOAT }, /* RMII_TX_EN */
	{ PORT_G,  13, MODE_ALT, 11, FLOAT }, /* RMII_TXD0 */
	{ PORT_G,  14, MODE_ALT, 11, FLOAT }, /* RMII_TXD1 */
	{ PORT_G,   2, MODE_ALT, 11, FLOAT }, /* RMII_RXER */
	{ PORT_C,   1, MODE_ALT, 11, FLOAT }, /* RMII_MDC */
	{ PORT_C,   4, MODE_ALT, 11, FLOAT }, /* RMII_RXD0 */
	{ PORT_C,   5, MODE_ALT, 11, FLOAT }, /* RMII_RXD1 */
	{ PORT_A,   7, MODE_ALT, 11, FLOAT }, /* RMII_CRS_DV */
	{ PORT_A,   2, MODE_ALT, 11, FLOAT }, /* RMII_MDIO */
	{ PORT_A,   1, MODE_ALT, 11, FLOAT }, /* RMII_REF_CLK */

	PINS_END
};

void
gpio_config(struct stm32f4_gpio_softc *sc)
{

	pin_configure(sc, pins_uart);
	pin_configure(sc, pins_dram);
	pin_configure(sc, pins_ltdc);
	pin_configure(sc, pins_eth);

	pin_set(sc, PORT_I, 12, 1);	/* LCD_DISP */
	pin_set(sc, PORT_K,  3, 1);	/* LCD_BL_CTRL */
}
