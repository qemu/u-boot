/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration file for the SAMA5D27 WLSOM1 EK Board.
 *
 * Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Nicolas Ferre <nicolas.ferre@microcihp.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "at91-sama5_common.h"

#undef CONFIG_SYS_AT91_MAIN_CLOCK
#define CONFIG_SYS_AT91_MAIN_CLOCK      24000000 /* from 24 MHz crystal */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x10000000

/* SPL */
#define CONFIG_SPL_BSS_START_ADDR	0x20000000
#define CONFIG_SYS_SPL_MALLOC_START	0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#endif
