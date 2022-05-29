/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013 Atmel Corporation.
 * Josh Wu <josh.wu@atmel.com>
 *
 * Configuation settings for the AT91SAM9N12-EK boards.
 */

#ifndef __AT91SAM9N12_CONFIG_H_
#define __AT91SAM9N12_CONFIG_H_

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	16000000	/* main clock xtal */

/* Misc CPU related */

/* LCD */
#define LCD_BPP				LCD_COLOR16
#define LCD_OUTPUT_BPP			24

#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x08000000

/* DataFlash */

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	GPIO_PIN_PD(4)
#define CONFIG_SYS_NAND_READY_PIN	GPIO_PIN_PD(5)
#endif

#define CONFIG_EXTRA_ENV_SETTINGS                                       \
	"console=console=ttyS0,115200\0"                                \
	"mtdparts="CONFIG_MTDPARTS_DEFAULT"\0"					\
	"bootargs_nand=rootfstype=ubifs ubi.mtd=7 root=ubi0:rootfs rw\0"\
	"bootargs_mmc=root=/dev/mmcblk0p2 rw rootfstype=ext4 rootwait\0"

/* USB host */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_PLLB
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE	ATMEL_BASE_OHCI
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"at91sam9n12"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	1
#endif

/* SPL */

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#define CONFIG_SYS_MASTER_CLOCK		132096000
#define CONFIG_SYS_AT91_PLLA		0x20953f03
#define CONFIG_SYS_MCKR			0x1301
#define CONFIG_SYS_MCKR_CSS		0x1302

#endif
