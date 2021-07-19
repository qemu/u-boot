// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Kontron Electronics GmbH
 *
 * Configuration settings for the Kontron SL/BL i.MX8M-Mini boards and modules (N81xx).
 */
#ifndef __KONTRON_MX8MM_CONFIG_H
#define __KONTRON_MX8MM_CONFIG_H

#ifdef CONFIG_SPL_BUILD
#include <config.h>
#endif

/* DDR RAM */
#define PHYS_SDRAM			DDR_CSD1_BASE_ADDR
#define PHYS_SDRAM_SIZE			(SZ_4G)
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM

#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x200000

/* Board and environment settings */
#define CONFIG_MXC_UART_BASE		UART3_BASE_ADDR
#define CONFIG_HOSTNAME			"kontron-n8000"

#define KONTRON_ENV_KERNEL_MTDPARTS	"mtdparts=spi1.0:128k(spl),832k(u-boot),64k(env)"

#define KONTRON_ENV_KERNEL_ADDR		"0x43000000"
#define KONTRON_ENV_FDT_ADDR		"0x42000000"
#define KONTRON_ENV_PXE_ADDR		"0x42100000"
#define KONTRON_ENV_RAMDISK_ADDR	"0x42200000"

/* Common options for Kontron Electronics boards */
#include "kontron_common.h"

#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
        func(MMC, mmc, 1) \
        func(MMC, mmc, 0) \
        func(PXE, pxe, na)
#include <config_distro_bootcmd.h>
/* Do not try to probe USB net adapters for net boot */
#undef BOOTENV_RUN_NET_USB_START
#define BOOTENV_RUN_NET_USB_START
#else
#define BOOTENV
#endif

#define CONFIG_LOADADDR			0x40480000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_BOOTM_LEN		SZ_64M
#define CONFIG_SPL_MAX_SIZE		(148 * SZ_1K)
#define CONFIG_FSL_USDHC

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_STACK		0x91fff0
#define CONFIG_SPL_BSS_START_ADDR	0x910000
#define CONFIG_SPL_BSS_MAX_SIZE		SZ_8K
#define CONFIG_SYS_SPL_MALLOC_START	0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_512K
#define CONFIG_MALLOC_F_ADDR		0x930000 /* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#endif

#define FEC_QUIRK_ENET_MAC

#endif /* __KONTRON_MX8MM_CONFIG_H */
