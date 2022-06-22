/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2022 ARM Limited
 * (C) Copyright 2022 Linaro
 * Rui Miguel Silva <rui.silva@linaro.org>
 * Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 *
 * Configuration for Corstone1000. Parts were derived from other ARM
 * configurations.
 */

#ifndef __CORSTONE1000_H
#define __CORSTONE1000_H

#include <linux/sizes.h>

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x03f00000)

#define V2M_BASE		0x80000000

#define CONFIG_PL011_CLOCK	50000000

/* Physical Memory Map */
#define PHYS_SDRAM_1		(V2M_BASE)
#define PHYS_SDRAM_1_SIZE	0x80000000

#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1

#endif
