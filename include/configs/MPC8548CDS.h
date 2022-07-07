/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2004, 2007, 2010-2011 Freescale Semiconductor.
 * Copyright 2020 NXP
 */

/*
 * mpc8548cds board configuration file
 *
 * Please refer to doc/README.mpc85xxcds for more info.
 *
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_SRIO
#define CONFIG_SRIO1			/* SRIO port 1 */

#define CONFIG_INTERRUPTS		/* enable pci, srio, ddr interrupts */

#ifndef __ASSEMBLY__
#include <linux/stringify.h>
#endif

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */

/*
 * Only possible on E500 Version 2 or newer cores.
 */

#define CONFIG_SYS_CCSRBAR		0xe0000000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/* DDR Setup */
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup*/

#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000	/* DDR is system memory*/
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

/* I2C addresses of SPD EEPROMs */
#define SPD_EEPROM_ADDRESS	0x51	/* CTLR 0 DIMM 0 */

/* Make sure required options are set */
#ifndef CONFIG_SPD_EEPROM
#error ("CONFIG_SPD_EEPROM is required")
#endif

/*
 * Physical Address Map
 *
 * 32bit:
 * 0x0000_0000	0x7fff_ffff	DDR			2G	cacheable
 * 0x8000_0000	0x9fff_ffff	PCI1 MEM		512M	cacheable
 * 0xa000_0000	0xbfff_ffff	PCIe MEM		512M	cacheable
 * 0xc000_0000	0xdfff_ffff	RapidIO			512M	cacheable
 * 0xe000_0000	0xe00f_ffff	CCSR			1M	non-cacheable
 * 0xe200_0000	0xe20f_ffff	PCI1 IO			1M	non-cacheable
 * 0xe300_0000	0xe30f_ffff	PCIe IO			1M	non-cacheable
 * 0xf000_0000	0xf3ff_ffff	SDRAM			64M	cacheable
 * 0xf800_0000	0xf80f_ffff	NVRAM/CADMUS		1M	non-cacheable
 * 0xff00_0000	0xff7f_ffff	FLASH (2nd bank)	8M	non-cacheable
 * 0xff80_0000	0xffff_ffff	FLASH (boot bank)	8M	non-cacheable
 *
 * 36bit:
 * 0x00000_0000	0x07fff_ffff	DDR			2G	cacheable
 * 0xc0000_0000	0xc1fff_ffff	PCI1 MEM		512M	cacheable
 * 0xc2000_0000	0xc3fff_ffff	PCIe MEM		512M	cacheable
 * 0xc4000_0000	0xc5fff_ffff	RapidIO			512M	cacheable
 * 0xfe000_0000	0xfe00f_ffff	CCSR			1M	non-cacheable
 * 0xfe200_0000	0xfe20f_ffff	PCI1 IO			1M	non-cacheable
 * 0xfe300_0000	0xfe30f_ffff	PCIe IO			1M	non-cacheable
 * 0xff000_0000	0xff3ff_ffff	SDRAM			64M	cacheable
 * 0xff800_0000	0xff80f_ffff	NVRAM/CADMUS		1M	non-cacheable
 * 0xfff00_0000	0xfff7f_ffff	FLASH (2nd bank)	8M	non-cacheable
 * 0xfff80_0000	0xfffff_ffff	FLASH (boot bank)	8M	non-cacheable
 *
 */

/*
 * Local Bus Definitions
 */

/*
 * FLASH on the Local Bus
 * Two banks, 8M each, using the CFI driver.
 * Boot from BR0/OR0 bank at 0xff00_0000
 * Alternate BR1/OR1 bank at 0xff80_0000
 *
 * BR0, BR1:
 *    Base address 0 = 0xff00_0000 = BR0[0:16] = 1111 1111 0000 0000 0
 *    Base address 1 = 0xff80_0000 = BR1[0:16] = 1111 1111 1000 0000 0
 *    Port Size = 16 bits = BRx[19:20] = 10
 *    Use GPCM = BRx[24:26] = 000
 *    Valid = BRx[31] = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 1111 1000 0000 0001 0000 0000 0001 = ff801001	 BR0
 * 1111 1111 0000 0000 0001 0000 0000 0001 = ff001001	 BR1
 *
 * OR0, OR1:
 *    Addr Mask = 8M = ORx[0:16] = 1111 1111 1000 0000 0
 *    Reserved ORx[17:18] = 11, confusion here?
 *    CSNT = ORx[20] = 1
 *    ACS = half cycle delay = ORx[21:22] = 11
 *    SCY = 6 = ORx[24:27] = 0110
 *    TRLX = use relaxed timing = ORx[29] = 1
 *    EAD = use external address latch delay = OR[31] = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 1111 1000 0000 0110 1110 0110 0101 = ff806e65	 ORx
 */

#define CONFIG_SYS_FLASH_BASE		0xff000000	/* start of FLASH 16M */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_FLASH_BASE_PHYS	0xfff000000ull
#else
#define CONFIG_SYS_FLASH_BASE_PHYS	CONFIG_SYS_FLASH_BASE
#endif

#define CONFIG_SYS_FLASH_BANKS_LIST \
	{CONFIG_SYS_FLASH_BASE_PHYS + 0x800000, CONFIG_SYS_FLASH_BASE_PHYS}
#define CONFIG_SYS_MAX_FLASH_SECT	128		/* sectors per device */
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_FLASH_EMPTY_INFO

#define CONFIG_HWCONFIG			/* enable hwconfig */

/*
 * SDRAM on the Local Bus
 */
#define CONFIG_SYS_LBC_SDRAM_BASE	0xf0000000	/* Localbus SDRAM */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_LBC_SDRAM_BASE_PHYS	0xff0000000ull
#else
#define CONFIG_SYS_LBC_SDRAM_BASE_PHYS	CONFIG_SYS_LBC_SDRAM_BASE
#endif
#define CONFIG_SYS_LBC_SDRAM_SIZE	64		/* LBC SDRAM is 64MB */

/*
 * Base Register 2 and Option Register 2 configure SDRAM.
 * The SDRAM base address, CONFIG_SYS_LBC_SDRAM_BASE, is 0xf0000000.
 *
 * For BR2, need:
 *    Base address of 0xf0000000 = BR[0:16] = 1111 0000 0000 0000 0
 *    port-size = 32-bits = BR2[19:20] = 11
 *    no parity checking = BR2[21:22] = 00
 *    SDRAM for MSEL = BR2[24:26] = 011
 *    Valid = BR[31] = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 0000 0000 0000 0001 1000 0110 0001 = f0001861
 *
 * FIXME: CONFIG_SYS_LBC_SDRAM_BASE should be masked and OR'ed into
 * FIXME: the top 17 bits of BR2.
 */

/*
 * The SDRAM size in MB, CONFIG_SYS_LBC_SDRAM_SIZE, is 64.
 *
 * For OR2, need:
 *    64MB mask for AM, OR2[0:7] = 1111 1100
 *		   XAM, OR2[17:18] = 11
 *    9 columns OR2[19-21] = 010
 *    13 rows	OR2[23-25] = 100
 *    EAD set for extra time OR[31] = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 1100 0000 0000 0110 1001 0000 0001 = fc006901
 */

#define CONFIG_SYS_LBC_LCRR		0x00030004	/* LB clock ratio reg */
#define CONFIG_SYS_LBC_LBCR		0x00000000	/* LB config reg */
#define CONFIG_SYS_LBC_LSRT		0x20000000	/* LB sdram refresh timer */
#define CONFIG_SYS_LBC_MRTPR		0x00000000	/* LB refresh timer prescal*/

/*
 * Common settings for all Local Bus SDRAM commands.
 * At run time, either BSMA1516 (for CPU 1.1)
 *		    or BSMA1617 (for CPU 1.0) (old)
 * is OR'ed in too.
 */
#define CONFIG_SYS_LBC_LSDMR_COMMON	( LSDMR_RFCR16		\
				| LSDMR_PRETOACT7	\
				| LSDMR_ACTTORW7	\
				| LSDMR_BL8		\
				| LSDMR_WRC4		\
				| LSDMR_CL3		\
				| LSDMR_RFEN		\
				)

/*
 * The CADMUS registers are connected to CS3 on CDS.
 * The new memory map places CADMUS at 0xf8000000.
 *
 * For BR3, need:
 *    Base address of 0xf8000000 = BR[0:16] = 1111 1000 0000 0000 0
 *    port-size = 8-bits  = BR[19:20] = 01
 *    no parity checking  = BR[21:22] = 00
 *    GPMC for MSEL	  = BR[24:26] = 000
 *    Valid		  = BR[31]    = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 1000 0000 0000 0000 1000 0000 0001 = f8000801
 *
 * For OR3, need:
 *    1 MB mask for AM,	  OR[0:16]  = 1111 1111 1111 0000 0
 *    disable buffer ctrl OR[19]    = 0
 *    CSNT		  OR[20]    = 1
 *    ACS		  OR[21:22] = 11
 *    XACS		  OR[23]    = 1
 *    SCY 15 wait states  OR[24:27] = 1111	max is suboptimal but safe
 *    SETA		  OR[28]    = 0
 *    TRLX		  OR[29]    = 1
 *    EHTR		  OR[30]    = 1
 *    EAD extra time	  OR[31]    = 1
 *
 * 0	4    8	  12   16   20	 24   28
 * 1111 1111 1111 0000 0000 1111 1111 0111 = fff00ff7
 */

#define CONFIG_FSL_CADMUS

#define CADMUS_BASE_ADDR 0xf8000000
#ifdef CONFIG_PHYS_64BIT
#define CADMUS_BASE_ADDR_PHYS	0xff8000000ull
#else
#define CADMUS_BASE_ADDR_PHYS	CADMUS_BASE_ADDR
#endif

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xe4010000	/* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000		/* Size of used area in RAM */

#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)

/* Serial Port */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/*
 * I2C
 */
#if !CONFIG_IS_ENABLED(DM_I2C)
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x69} }
#endif

/* EEPROM */
#define CONFIG_SYS_I2C_EEPROM_CCID

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
#define CONFIG_SYS_PCI1_MEM_VIRT	0x80000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCI1_MEM_BUS		0xe0000000
#define CONFIG_SYS_PCI1_MEM_PHYS	0xc00000000ull
#else
#define CONFIG_SYS_PCI1_MEM_BUS	0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	0x80000000
#endif
#define CONFIG_SYS_PCI1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCI1_IO_VIRT	0xe2000000
#define CONFIG_SYS_PCI1_IO_BUS	0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCI1_IO_PHYS 0xfe2000000ull
#else
#define CONFIG_SYS_PCI1_IO_PHYS	0xe2000000
#endif
#define CONFIG_SYS_PCI1_IO_SIZE	0x00100000	/* 1M */

#ifdef CONFIG_PCIE1
#define CONFIG_SYS_PCIE1_MEM_VIRT	0xa0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc20000000ull
#else
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xa0000000
#endif
#define CONFIG_SYS_PCIE1_IO_VIRT	0xe3000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_IO_PHYS        0xfe3000000ull
#else
#define CONFIG_SYS_PCIE1_IO_PHYS	0xe3000000
#endif
#endif

/*
 * RapidIO MMU
 */
#define CONFIG_SYS_SRIO1_MEM_VIRT	0xc0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_SRIO1_MEM_PHYS	0xc40000000ull
#else
#define CONFIG_SYS_SRIO1_MEM_PHYS	0xc0000000
#endif
#define CONFIG_SYS_SRIO1_MEM_SIZE	0x20000000	/* 512M */

#if defined(CONFIG_TSEC_ENET)

#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"eTSEC0"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"eTSEC1"
#define CONFIG_TSEC3	1
#define CONFIG_TSEC3_NAME	"eTSEC2"
#define CONFIG_TSEC4
#define CONFIG_TSEC4_NAME	"eTSEC3"
#undef CONFIG_MPC85XX_FEC

#define TSEC1_PHY_ADDR		0
#define TSEC2_PHY_ADDR		1
#define TSEC3_PHY_ADDR		2
#define TSEC4_PHY_ADDR		3

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC3_PHYIDX		0
#define TSEC4_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC2_FLAGS		TSEC_GIGABIT
#define TSEC3_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC4_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#endif	/* CONFIG_TSEC_ENET */

/*
 * Environment
 */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * Miscellaneous configurable options
 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial Memory map for Linux*/

/*
 * Environment Configuration
 */

#define CONFIG_IPADDR	 192.168.1.253

#define CONFIG_HOSTNAME	 "unknown"
#define CONFIG_ROOTPATH	 "/nfsroot"
#define CONFIG_UBOOTPATH	8548cds/u-boot.bin	/* TFTP server */

#define CONFIG_SERVERIP	 192.168.1.1
#define CONFIG_GATEWAYIP 192.168.1.1
#define CONFIG_NETMASK	 255.255.255.0

#define	CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:ecc=off\0"		\
	"netdev=eth0\0"				\
	"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"	\
	"tftpflash=tftpboot $loadaddr $uboot; "	\
		"protect off " __stringify(CONFIG_SYS_TEXT_BASE)	\
			" +$filesize; "	\
		"erase " __stringify(CONFIG_SYS_TEXT_BASE)		\
			" +$filesize; "	\
		"cp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE)	\
			" $filesize; "	\
		"protect on " __stringify(CONFIG_SYS_TEXT_BASE)		\
			" +$filesize; "	\
		"cmp.b $loadaddr " __stringify(CONFIG_SYS_TEXT_BASE)	\
			" $filesize\0"	\
	"consoledev=ttyS1\0"			\
	"ramdiskaddr=2000000\0"			\
	"ramdiskfile=ramdisk.uboot\0"		\
	"fdtaddr=1e00000\0"			\
	"fdtfile=mpc8548cds.dtb\0"

#endif	/* __CONFIG_H */
