/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Kontron Electronics GmbH
 *
 * Configuration settings for the Kontron i.MX6UL boards/SoMs.
 */
#ifndef __KONTRON_MX6UL_CONFIG_H
#define __KONTRON_MX6UL_CONFIG_H

/* DDR RAM */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM

#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* Board and environment settings */
#define CONFIG_MXC_UART_BASE		UART4_BASE
#define CONFIG_HOSTNAME			"kontron-mx6ul"

#define KONTRON_ENV_KERNEL_MTDPARTS	"mtdparts=spi1.0:128k(spl),832k(u-boot),64k(env);spi4.0:192m(rootfs),-(user)"

#define KONTRON_ENV_KERNEL_ADDR		"0x82000000"
#define KONTRON_ENV_FDT_ADDR		"0x83000000"
#define KONTRON_ENV_PXE_ADDR		"0x83100000"
#define KONTRON_ENV_RAMDISK_ADDR	"0x83200000"

/* Common options for Kontron Electronics boards */
#include "kontron_common.h"

/* Boot order for distro boot */
#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(UBIFS, ubifs, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#else
#define BOOTENV
#endif

/* MMC Configs */
#ifdef CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC1_BASE_ADDR
#define CONFIG_SYS_FSL_USDHC_NUM	2
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1
#endif

#endif /* __KONTRON_MX6UL_CONFIG_H */
