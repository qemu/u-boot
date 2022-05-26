/*
 * (C) Copyright 2015
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * DDR Setup
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000 /* DDR is system memory */
/* TODO: Check: Can this be unified with CONFIG_SYS_SDRAM_BASE? */
#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_SDRAM_BASE

/*
 * Memory test
 * TODO: Migrate!
 */

/*
 * The reserved memory
 */

#define CONFIG_SYS_MONITOR_LEN	(512 * 1024) /* Reserve 512 kB for Mon */

/*
 * Initial RAM Base Address Setup
 */
#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000 /* Size of used area in RAM */

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_BASE		0xFE000000 /* FLASH base address */
#define CONFIG_SYS_FLASH_SIZE		8 /* FLASH size is up to 8M */

#define CONFIG_SYS_MAX_FLASH_SECT	135

#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*
 * Environment
 */

#define CONFIG_LOADS_ECHO		/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change */

/*
 * Miscellaneous configurable options
 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20) /* Initial Memory map for Linux */

/*
 * Environment Configuration
 */

/* TODO: Turn into string option and migrate to Kconfig */
#define CONFIG_HOSTNAME		"gazerbeam"
#define CONFIG_ROOTPATH		"/opt/nfsroot"

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"consoledev=ttyS1\0"						\
	"u-boot=u-boot.bin\0"						\
	"kernel_addr=1000000\0"					\
	"fdt_addr=C00000\0"						\
	"fdtfile=hrcon.dtb\0"				\
	"load=tftp ${loadaddr} ${u-boot}\0"				\
	"update=protect off " __stringify(CONFIG_SYS_MONITOR_BASE)	\
		" +${filesize};era " __stringify(CONFIG_SYS_MONITOR_BASE)\
		" +${filesize};cp.b ${fileaddr} "			\
		__stringify(CONFIG_SYS_MONITOR_BASE) " ${filesize}\0"	\
	"upd=run load update\0"						\

#endif	/* __CONFIG_H */
