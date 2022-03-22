/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020 Engicam srl
 * Copyright (c) 2020 Amarula Solutions(India)
 */

#ifndef __IMX8MM_ICORE_MX8MM_H
#define __IMX8MM_ICORE_MX8MM_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_SPL_MAX_SIZE		(148 * 1024)
#define CONFIG_SYS_MONITOR_LEN		SZ_512K
#define CONFIG_SYS_UBOOT_BASE \
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
# define CONFIG_SPL_STACK		0x920000
# define CONFIG_SPL_BSS_START_ADDR	0x910000
# define CONFIG_SPL_BSS_MAX_SIZE	SZ_8K
# define CONFIG_SYS_SPL_MALLOC_START	0x42200000
# define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_512K

/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
# define CONFIG_MALLOC_F_ADDR		0x930000
/* For RAW image gives a error info not panic */
# define CONFIG_SPL_ABORT_ON_RAW_IMAGE
#endif /* CONFIG_SPL_BUILD */

#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 2) \
	func(MMC, mmc, 0)
#include <config_distro_bootcmd.h>
#else
#define BOOTENV
#endif

#define ENV_MEM_LAYOUT_SETTINGS \
	"fdt_addr_r=0x44000000\0" \
	"kernel_addr_r=0x42000000\0" \
	"ramdisk_addr_r=0x46400000\0" \
	"scriptaddr=0x46000000\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"console=ttymxc1,115200\0" \
	BOOTENV

/* Link Definitions */

#define CONFIG_SYS_INIT_RAM_ADDR        0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE        SZ_2M
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_SDRAM_BASE           0x40000000

/* SDRAM configuration */
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			SZ_2G /* 2GB DDR */
#define CONFIG_SYS_BOOTM_LEN		SZ_256M

/* UART */
#define CONFIG_MXC_UART_BASE		UART2_BASE_ADDR

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

/* USDHC */
#define CONFIG_SYS_FSL_USDHC_NUM	2
#define CONFIG_SYS_FSL_ESDHC_ADDR	0

#endif /* __IMX8MM_ICORE_MX8MM_H */
