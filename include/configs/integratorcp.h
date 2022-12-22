/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Compact Integrator board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "integrator-common.h"

/* Integrator CP-specific configuration */
#define CFG_SYS_HZ_CLOCK		1000000	/* Timer 1 is clocked at 1Mhz */

/*
 * Miscellaneous configurable options
 */
#define PHYS_FLASH_SIZE			0x01000000	/* 16MB */

#endif /* __CONFIG_H */
