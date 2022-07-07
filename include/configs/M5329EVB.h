/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the Freescale MCF5329 FireEngine board.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef _M5329EVB_H
#define _M5329EVB_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_SYS_UART_PORT		(0)

#define CONFIG_WATCHDOG_TIMEOUT	5000	/* timeout in milliseconds, max timeout is 6.71sec */

/* I2C */

#ifdef CONFIG_MCFFEC
#	define CONFIG_IPADDR	192.162.1.2
#	define CONFIG_NETMASK	255.255.255.0
#	define CONFIG_SERVERIP	192.162.1.1
#	define CONFIG_GATEWAYIP	192.162.1.1
#endif				/* FEC_ENET */

#define CONFIG_HOSTNAME		"M5329EVB"
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"			\
	"loadaddr=40010000\0"	\
	"u-boot=u-boot.bin\0"	\
	"load=tftp ${loadaddr) ${u-boot}\0"	\
	"upd=run load; run prog\0"	\
	"prog=prot off 0 3ffff;"	\
	"era 0 3ffff;"	\
	"cp.b ${loadaddr} 0 ${filesize};"	\
	"save\0"	\
	""

#define CONFIG_PRAM		512	/* 512 KB */

#define CONFIG_SYS_CLK			80000000
#define CONFIG_SYS_CPU_CLK		CONFIG_SYS_CLK * 3

#define CONFIG_SYS_MBAR		0xFC000000

#define CONFIG_SYS_LATCH_ADDR		(CONFIG_SYS_CS1_BASE + 0x80000)

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x80000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x8000	/* Size of used area in internal SRAM */
#define CONFIG_SYS_INIT_RAM_CTRL	0x221

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_SDRAM_SIZE		32	/* SDRAM size in MB */
#define CONFIG_SYS_SDRAM_CFG1		0x53722730
#define CONFIG_SYS_SDRAM_CFG2		0x56670000
#define CONFIG_SYS_SDRAM_CTRL		0xE1092000
#define CONFIG_SYS_SDRAM_EMOD		0x40010000
#define CONFIG_SYS_SDRAM_MODE		0x018D0000

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
#define CONFIG_SYS_BOOTMAPSZ		(CONFIG_SYS_SDRAM_BASE + (CONFIG_SYS_SDRAM_SIZE << 20))

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#ifdef CONFIG_SYS_FLASH_CFI
#	define CONFIG_SYS_FLASH_SIZE		0x800000	/* Max size that the board might have */
#	define CONFIG_SYS_MAX_FLASH_SECT	137	/* max number of sectors on one chip */
#endif

#ifdef CONFIG_CMD_NAND
#	define CONFIG_SYS_MAX_NAND_DEVICE	1
#	define CONFIG_SYS_NAND_BASE		CONFIG_SYS_CS2_BASE
#	define CONFIG_SYS_NAND_SIZE		1
#	define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE }
#	define NAND_ALLOW_ERASE_ALL	1
#endif

#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_CS0_BASE

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */

#define LDS_BOARD_TEXT \
	. = DEFINED(env_offset) ? env_offset : .; \
	env/embedded.o(.text*);

/*-----------------------------------------------------------------------
 * Cache Configuration
 */

#define ICACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 4)
#define CONFIG_SYS_ICACHE_INV		(CF_CACR_CINVA)
#define CONFIG_SYS_CACHE_ACR0		(CONFIG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CONFIG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CONFIG_SYS_CACHE_ICACR		(CF_CACR_EC | CF_CACR_CINVA | \
					 CF_CACR_DCM_P)

/*-----------------------------------------------------------------------
 * Chipselect bank definitions
 */
/*
 * CS0 - NOR Flash 1, 2, 4, or 8MB
 * CS1 - CompactFlash and registers
 * CS2 - NAND Flash 16, 32, or 64MB
 * CS3 - Available
 * CS4 - Available
 * CS5 - Available
 */
#define CONFIG_SYS_CS0_BASE		0
#define CONFIG_SYS_CS0_MASK		0x007f0001
#define CONFIG_SYS_CS0_CTRL		0x00001fa0

#define CONFIG_SYS_CS1_BASE		0x10000000
#define CONFIG_SYS_CS1_MASK		0x001f0001
#define CONFIG_SYS_CS1_CTRL		0x002A3780

#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_CS2_BASE		0x20000000
#define CONFIG_SYS_CS2_MASK		(16 << 20)
#define CONFIG_SYS_CS2_CTRL		0x00001f60
#endif

#endif				/* _M5329EVB_H */
