/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_DB_88F6820_AMC_H
#define _CONFIG_DB_88F6820_AMC_H

/*
 * High Level Configuration Options (easy to change)
 */

/* Environment in SPI NOR flash */

#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/* PCIe support */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_PCI_SCAN_SHOW
#endif

/* NAND */

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

/* SPL */
/*
 * Select the boot device here
 *
 * Currently supported are:
 * SPL_BOOT_SPI_NOR_FLASH	- Booting via SPI NOR flash
 *
 * MMC is not populated on this board.
 * NAND support may be added in the future.
 */
#define SPL_BOOT_SPI_NOR_FLASH		1
#define CONFIG_SPL_BOOT_DEVICE		SPL_BOOT_SPI_NOR_FLASH

/* Defines for SPL */
#define CONFIG_SPL_SIZE			(140 << 10)

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

#endif /* _CONFIG_DB_88F6820_AMC_H */
