/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#ifndef __IMX8MM_DATA_MODUL_EDM_SBC_H
#define __IMX8MM_DATA_MODUL_EDM_SBC_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_SYS_BOOTM_LEN		SZ_128M

#define CONFIG_SPL_MAX_SIZE		(148 * 1024)
#define CONFIG_SYS_MONITOR_LEN		SZ_1M

#define CONFIG_SPL_STACK		0x920000
#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_BSS_START_ADDR	0x910000
#define CONFIG_SPL_BSS_MAX_SIZE		SZ_8K	/* 8 kiB */
#define CONFIG_SYS_SPL_MALLOC_START	0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_16M	/* 16 MiB */

#define CONFIG_MALLOC_F_ADDR		0x930000

/* For RAW image gives a error info not panic */
#define CONFIG_SPL_ABORT_ON_RAW_IMAGE

#endif

/* Link Definitions */
#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x200000
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0x40000000 /* Minimum 1 GiB DDR */

#define CONFIG_MXC_UART_BASE		UART3_BASE_ADDR

#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* PHY needs a longer autonegotiation timeout after reset */
#define PHY_ANEG_TIMEOUT		20000

/* USDHC */
#define CONFIG_SYS_FSL_USDHC_NUM	2
#define CONFIG_SYS_FSL_ESDHC_ADDR	0

#if !defined(CONFIG_SPL_BUILD)

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"altbootcmd=setenv devpart 2 && run bootcmd ; reset\0"		\
	"bootlimit=3\0"							\
	"devtype=mmc\0"							\
	"devpart=1\0"							\
	/* Give slow devices beyond USB HUB chance to come up. */	\
	"usb_pgood_delay=2000\0"					\
	"dfu_alt_info="							\
		/* RAM block at DRAM offset 256..768 MiB */		\
		"ram ram0=ram ram 0x50000000 0x20000000&"		\
		/* 16 MiB SPI NOR */					\
		"mtd nor0=sf raw 0x0 0x1000000\0"			\
	"dmo_preboot="							\
		"sf probe ; " /* Scan for SPI NOR, needed by DFU */	\
		"run dmo_usb_start_hub ; "				\
		/* Attempt to start USB and Network console */		\
		"run dmo_usb_cdc_acm_start ; "				\
		"run dmo_netconsole_start\0"				\
	"dmo_update_env="						\
		"setenv dmo_update_env true ; saveenv ; saveenv\0"	\
	"dmo_usb_cdc_acm_start="					\
		"if test \"${dmo_usb_cdc_acm_enabled}\" = \"true\" ; then "\
			/* Ungate IMX8MM_CLK_USB1_CTRL_ROOT */		\
			"mw 0x303844d0 3 ; "				\
			/* Read USBNC_n_PHY_STATUS BIT(4) VBUS_VLD */	\
			"setexpr.l usbnc_n_phy_status *0x32e4023c \\\\& 0x8 ; "	\
			/* If USB OTG has valid VBUS, enable CDC ACM */	\
			"if test \"${usbnc_n_phy_status}\" -eq 8 ; then "\
				"usb start && "				\
				"setenv stderr ${stderr},usbacm && "	\
				"setenv stdout ${stdout},usbacm && "	\
				"setenv stdin ${stdin},usbacm ; "	\
			"fi ; "						\
		"fi\0"							\
	"dmo_usb_start_hub="						\
		"i2c dev 1 ; "						\
		/* Reset the USB USB */					\
		"gpio clear GPIO5_2 ; sleep 0.01 ; " /* t1 > 1us */	\
		"gpio set GPIO5_2 ; sleep 0.01 ; " /* t5 > 3us */	\
		/* Write chunks of descriptor into the USB HUB */	\
		"mw.l 0x7e1000 0x14042417 ; mw.l 0x7e1004 0x9b0bb325 ; "\
		"mw.l 0x7e1008 0x00000220 ; mw.l 0x7e100c 0x01320100 ; "\
		"mw.l 0x7e1010 0x00003232 ; mw.l 0x7e1014 0x4d000909 ; "\
		"i2c write 0x7e1000 0x2c 0x00 0x18 -s ; "		\
		"mw.l 0x7e1000 0x6300690f ; mw.l 0x7e1004 0x6f007200 ; "\
		"mw.l 0x7e1008 0x68006300 ; mw.l 0x7e100c 0x70006900 ; "\
		"i2c write 0x7e1000 0x2c 0x18 0x10 -s ; "		\
		"mw.l 0x7e1000 0x53005511 ; mw.l 0x7e1004 0x32004200 ; "\
		"mw.l 0x7e1008 0x31003500 ; mw.l 0x7e100c 0x42003400 ; "\
		"mw.l 0x7e1010 0x00006900 ; "				\
		"i2c write 0x7e1000 0x2c 0x54 0x12 -s ; "		\
		"mw.l 0x7e1000 0x00000101 ; "				\
		"i2c write 0x7e1000 0x2c 0xff 0x2 -s\0"			\
	"dmo_netconsole_start="						\
		"if test \"${dmo_netconsole_enabled}\" = \"true\" ; then "\
			"setenv autoload false && "			\
			"dhcp && "					\
			"setenv autoload && "				\
			"setenv ncip ${serverip} && "			\
			"setenv stderr ${stderr},nc && "		\
			"setenv stdout ${stdout},nc && "		\
			"setenv stdin ${stdin},nc ; "			\
		"fi"

#endif

#endif
