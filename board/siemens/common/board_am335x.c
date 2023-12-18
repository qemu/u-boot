// SPDX-License-Identifier: GPL-2.0+
/*
 * Common board functions for siemens AM335X based boards
 * (C) Copyright Siemens AG 2013
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * U-Boot file:/board/ti/am335x/board.c
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <init.h>
#include <serial.h>
#include <asm/arch/clock.h>
#include "board_am335x.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPL_BUILD
void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	/* Initalize the board header */
	enable_i2c0_pin_mux();

	/* enable early the console */
	gd->baudrate = CONFIG_BAUDRATE;
	serial_init();
	gd->have_console = 1;

	siemens_ee_setup();
	if (siemens_read_eeprom() < 0)
		puts("Could not get board ID.\n");

	enable_board_pin_mux();
}

void sdram_init(void)
{
	spl_siemens_board_init();
	siemens_init_ddr();

	return;
}
#endif /* #ifdef CONFIG_SPL_BUILD */

#define OSC	(V_OSCK/1000000)
const struct dpll_params dpll_ddr = {
		DDR_PLL_FREQ, OSC-1, 1, -1, -1, -1, -1};

const struct dpll_params *get_dpll_ddr_params(void)
{
	return &dpll_ddr;
}
