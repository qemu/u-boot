/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the EXYNOS 78x0 based boards.
 *
 * Copyright (c) 2020 Dzmitry Sankouski (dsankouski@gmail.com)
 * based on include/exynos7420-common.h
 * Copyright (C) 2016 Samsung Electronics
 * Thomas Abraham <thomas.ab@samsung.com>
 */

#ifndef __CONFIG_EXYNOS78x0_COMMON_H
#define __CONFIG_EXYNOS78x0_COMMON_H

/* High Level Configuration Options */
#define CONFIG_SAMSUNG			/* in a SAMSUNG core */
#define CONFIG_S5P

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <linux/sizes.h>

/* Miscellaneous configurable options */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		1024	/* Print Buffer Size */

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* Timer input clock frequency */
#define COUNTER_FREQUENCY		26000000

#define CPU_RELEASE_ADDR		secondary_boot_addr

#define CONFIG_SYS_BAUDRATE_TABLE \
	{9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600}

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE + SZ_2M - GENERATED_GBL_DATA_SIZE)
/* DRAM Memory Banks */
#define SDRAM_BANK_SIZE		(256UL << 20UL)	/* 256 MB */
#define PHYS_SDRAM_1		CONFIG_SYS_SDRAM_BASE
#define PHYS_SDRAM_1_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_2		(CONFIG_SYS_SDRAM_BASE + SDRAM_BANK_SIZE)
#define PHYS_SDRAM_2_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_3		(CONFIG_SYS_SDRAM_BASE + (2 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_3_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_4		(CONFIG_SYS_SDRAM_BASE + (3 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_4_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_5		(CONFIG_SYS_SDRAM_BASE + (4 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_5_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_6		(CONFIG_SYS_SDRAM_BASE + (5 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_6_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_7		(CONFIG_SYS_SDRAM_BASE + (6 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_7_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_8		(CONFIG_SYS_SDRAM_BASE + (7 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_8_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_9		(CONFIG_SYS_SDRAM_BASE + (8 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_9_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_10		(CONFIG_SYS_SDRAM_BASE + (9 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_10_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_11		(CONFIG_SYS_SDRAM_BASE + (10 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_11_SIZE	SDRAM_BANK_SIZE
#define PHYS_SDRAM_12		(CONFIG_SYS_SDRAM_BASE + (11 * SDRAM_BANK_SIZE))
#define PHYS_SDRAM_12_SIZE	SDRAM_BANK_SIZE

#ifndef MEM_LAYOUT_ENV_SETTINGS
#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"bootm_low=0x40000000\0"
#endif

#ifndef EXYNOS_DEVICE_SETTINGS
#define EXYNOS_DEVICE_SETTINGS \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"stderr=serial\0"
#endif

#ifndef EXYNOS_FDTFILE_SETTING
#define EXYNOS_FDTFILE_SETTING
#endif

#define EXTRA_ENV_SETTINGS \
	EXYNOS_DEVICE_SETTINGS \
	EXYNOS_FDTFILE_SETTING \
	MEM_LAYOUT_ENV_SETTINGS

#define CONFIG_EXTRA_ENV_SETTINGS \
	EXTRA_ENV_SETTINGS

#endif	/* __CONFIG_EXYNOS78x0_COMMON_H */
