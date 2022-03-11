/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 * Copyright 2018 Emcraft Systems
 * Copyright 2019 Purism
 *
 */

#ifndef __LIBREM5_H
#define __LIBREM5_H

/* #define DEBUG */

#include <version.h>
#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#ifdef CONFIG_SECURE_BOOT
#define CONFIG_CSF_SIZE			0x2000 /* 8K region */
#endif

#define CONFIG_SPL_MAX_SIZE		(148 * 1024)
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_LDSCRIPT		"arch/arm/cpu/armv8/u-boot-spl.lds"
#define CONFIG_SPL_STACK		0x187FF0
#define CONFIG_SPL_BSS_START_ADDR      0x00180000
#define CONFIG_SPL_BSS_MAX_SIZE        0x2000	/* 8 KB */
#define CONFIG_SPL_STACK_R_ADDR		0x42300000
#define CONFIG_SPL_STACK_R
#define CONFIG_SYS_SPL_MALLOC_START    0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE     0x80000	/* 512 KB */

#define CONFIG_SYS_MALLOC_F_LEN		0x2000
#define CONFIG_MALLOC_F_ADDR		0x182000 /* malloc f used before GD_FLG_FULL_MALLOC_INIT */

#define CONFIG_SPL_ABORT_ON_RAW_IMAGE /* For RAW image gives a error info not panic */

#undef CONFIG_DEVRES
#undef CONFIG_DM_USB
#undef CONFIG_USB_DWC3_GENERIC

#define CONFIG_POWER_BD71837
#define CONFIG_POWER_BD71837_I2C_BUS	0
#define CONFIG_POWER_BD71837_I2C_ADDR	0x4B

#endif /* CONFIG_SPL_BUILD*/

#define CONFIG_SYS_FSL_USDHC_NUM	2

#define CONFIG_USB_MAX_CONTROLLER_COUNT 1

#define CONFIG_USBD_HS

#define CONFIG_USB_GADGET_VBUS_DRAW	2

#define CONSOLE_ON_UART1
#define CONFIG_BAUDRATE			115200

#ifdef CONSOLE_ON_UART1
#define CONFIG_MXC_UART_BASE		UART1_BASE_ADDR
#define CONSOLE_UART_CLK		0
#define CONSOLE		"ttymxc0"
#elif defined(CONSOLE_ON_UART2)
#define CONFIG_MXC_UART_BASE		UART2_BASE_ADDR
#define CONSOLE_UART_CLK		1
#define CONSOLE		"ttymxc1"
#elif defined(CONSOLE_ON_UART3)
#define CONFIG_MXC_UART_BASE		UART3_BASE_ADDR
#define CONSOLE_UART_CLK		2
#define CONSOLE		"ttymxc2"
#elif defined(CONSOLE_ON_UART4)
#define CONFIG_MXC_UART_BASE		UART4_BASE_ADDR
#define CONSOLE_UART_CLK		3
#define CONSOLE		"ttymxc3"
#else
#define CONFIG_MXC_UART_BASE		UART1_BASE_ADDR
#define CONSOLE_UART_CLK		0
#define CONSOLE		"ttymxc0"
#endif

#define CONFIG_REMAKE_ELF

#define CONFIG_BOARD_POSTCLK_INIT

/* Flat Device Tree Definitions */
#define CONFIG_OF_BOARD_SETUP

#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#else
#define BOOTENV
#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"scriptaddr=0x80000000\0" \
	"pxefile_addr_r=0x80100000\0" \
	"kernel_addr_r=0x80800000\0" \
	"fdt_addr_r=0x83000000\0" \
	"ramdisk_addr_r=0x83800000\0" \
	"console=" CONSOLE ",115200\0" \
	"bootargs=u_boot_version=" PLAIN_VERSION "\0" \
	"stdin=usbacm,serial\0" \
	"stdout=usbacm,serial\0" \
	"stderr=usbacm,serial\0" \
	BOOTENV

/* Link Definitions */
#define CONFIG_SYS_TEXT_BASE		0x40200000

#define CONFIG_SYS_INIT_RAM_ADDR        0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE        0x80000
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_ENV_OVERWRITE
#define CONFIG_MMCROOT			"/dev/mmcblk0p2"  /* USDHC1 */

#define CONFIG_SYS_SDRAM_BASE           0x40000000
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			0xc0000000 /* 3GB LPDDR4 one Rank */

#define CONFIG_SYS_ALT_MEMTEST

/* Monitor Command Prompt */
#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		"u-boot=> "
#define CONFIG_SYS_PROMPT_HUSH_PS2     "> "
#define CONFIG_SYS_CBSIZE              1024
#define CONFIG_SYS_MAXARGS             64
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_FSL_ESDHC_ADDR       0

#ifdef CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_BUS	0
#define CONFIG_SF_DEFAULT_CS	0
#endif
#endif
