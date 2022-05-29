/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010-2012
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA_COMMON_H_
#define _TEGRA_COMMON_H_
#include <linux/sizes.h>
#include <linux/stringify.h>

/*
 * High Level Configuration Options
 */

#include <asm/arch/tegra.h>		/* get chip and board defs */

/* Use the Tegra US timer on ARMv7, but the architected timer on ARMv8. */
#ifndef CONFIG_ARM64
#define CONFIG_SYS_TIMER_RATE		1000000
#define CONFIG_SYS_TIMER_COUNTER	NV_PA_TMRUS_BASE
#endif

/* Environment */

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * Common HW configuration.
 * If this varies between SoCs later, move to tegraNN-common.h
 * Note: This is number of devices, not max device ID.
 */
#define CONFIG_SYS_MMC_MAX_DEVICE 4

#ifdef CONFIG_ARM64
#define FDTFILE "nvidia/" CONFIG_DEFAULT_DEVICE_TREE ".dtb"
#else
#define FDTFILE CONFIG_DEFAULT_DEVICE_TREE ".dtb"
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define PHYS_SDRAM_1		NV_PA_SDRC_CS0
#define PHYS_SDRAM_1_SIZE	0x20000000	/* 512M */

#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1

#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)	/* 256M */

#ifndef CONFIG_ARM64
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_STACKBASE
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_MALLOC_LEN

/* Defines for SPL */
#endif

#endif /* _TEGRA_COMMON_H_ */
