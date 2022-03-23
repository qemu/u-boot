/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 * Ilko Iliev <www.ronetix.at>
 *
 * Configuration settings for the RONETIX PM9263 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>

/* ARM asynchronous clock */

#define MASTER_PLL_DIV		6
#define MASTER_PLL_MUL		65
#define MAIN_PLL_DIV		2	/* 2 or 4 */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */

/* clocks */
#define CONFIG_SYS_MOR_VAL						\
		(AT91_PMC_MOR_MOSCEN |					\
		 (255 << 8))		/* Main Oscillator Start-up Time */
#define CONFIG_SYS_PLLAR_VAL						\
		(AT91_PMC_PLLAR_29 | /* Bit 29 must be 1 when prog */ \
		 AT91_PMC_PLLXR_OUT(3) |				\
		 AT91_PMC_PLLXR_PLLCOUNT(0x3f) |	/* PLL Counter */\
		 (2 << 28) |		/* PLL Clock Frequency Range */	\
		 ((MASTER_PLL_MUL - 1) << 16) | (MASTER_PLL_DIV))

#if (MAIN_PLL_DIV == 2)
/* PCK/2 = MCK Master Clock from PLLA */
#define	CONFIG_SYS_MCKR1_VAL		\
		(AT91_PMC_MCKR_CSS_SLOW |	\
		 AT91_PMC_MCKR_PRES_1 |	\
		 AT91_PMC_MCKR_MDIV_2)
/* PCK/2 = MCK Master Clock from PLLA */
#define	CONFIG_SYS_MCKR2_VAL		\
		(AT91_PMC_MCKR_CSS_PLLA |	\
		 AT91_PMC_MCKR_PRES_1 |	\
		 AT91_PMC_MCKR_MDIV_2)
#else
/* PCK/4 = MCK Master Clock from PLLA */
#define	CONFIG_SYS_MCKR1_VAL			\
		(AT91_PMC_MCKR_CSS_SLOW |		\
		 AT91_PMC_MCKR_PRES_1 |		\
		 AT91_PMC_MCKR_MDIV_4)
/* PCK/4 = MCK Master Clock from PLLA */
#define	CONFIG_SYS_MCKR2_VAL			\
		(AT91_PMC_MCKR_CSS_PLLA |		\
		 AT91_PMC_MCKR_PRES_1 |		\
		 AT91_PMC_MCKR_MDIV_4)
#endif
/* define PDC[31:16] as DATA[31:16] */
#define CONFIG_SYS_PIOD_PDR_VAL1	0xFFFF0000
/* no pull-up for D[31:16] */
#define CONFIG_SYS_PIOD_PPUDR_VAL	0xFFFF0000
/* EBI0_CSA, CS1 SDRAM, CS3 NAND Flash, 3.3V memories */
#define CONFIG_SYS_MATRIX_EBI0CSA_VAL					\
	(AT91_MATRIX_CSA_DBPUC | AT91_MATRIX_CSA_VDDIOMSEL_3_3V |	\
	 AT91_MATRIX_CSA_EBI_CS1A)

/* SDRAM */
/* SDRAMC_MR Mode register */
#define CONFIG_SYS_SDRC_MR_VAL1		0
/* SDRAMC_TR - Refresh Timer register */
#define CONFIG_SYS_SDRC_TR_VAL1		0x3AA
/* SDRAMC_CR - Configuration register*/
#define CONFIG_SYS_SDRC_CR_VAL							\
		(AT91_SDRAMC_NC_9 |						\
		 AT91_SDRAMC_NR_13 |						\
		 AT91_SDRAMC_NB_4 |						\
		 AT91_SDRAMC_CAS_2 |						\
		 AT91_SDRAMC_DBW_32 |						\
		 (2 <<  8) |	/* tWR -  Write Recovery Delay */		\
		 (7 << 12) |	/* tRC -  Row Cycle Delay */			\
		 (2 << 16) |	/* tRP -  Row Precharge Delay */		\
		 (2 << 20) |	/* tRCD - Row to Column Delay */		\
		 (5 << 24) |	/* tRAS - Active to Precharge Delay */		\
		 (8 << 28))	/* tXSR - Exit Self Refresh to Active Delay */

/* Memory Device Register -> SDRAM */
#define CONFIG_SYS_SDRC_MDR_VAL		AT91_SDRAMC_MD_SDRAM
#define CONFIG_SYS_SDRC_MR_VAL2		AT91_SDRAMC_MODE_PRECHARGE
#define CONFIG_SYS_SDRAM_VAL1		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRC_MR_VAL3		AT91_SDRAMC_MODE_REFRESH
#define CONFIG_SYS_SDRAM_VAL2		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL3		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL4		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL5		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL6		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL7		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL8		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRAM_VAL9		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRC_MR_VAL4		AT91_SDRAMC_MODE_LMR
#define CONFIG_SYS_SDRAM_VAL10		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRC_MR_VAL5		AT91_SDRAMC_MODE_NORMAL
#define CONFIG_SYS_SDRAM_VAL11		0		/* SDRAM_BASE */
#define CONFIG_SYS_SDRC_TR_VAL2		1200		/* SDRAM_TR */
#define CONFIG_SYS_SDRAM_VAL12		0		/* SDRAM_BASE */

/* setup SMC0, CS0 (NOR Flash) - 16-bit, 15 WS */
#define CONFIG_SYS_SMC0_SETUP0_VAL					\
		(AT91_SMC_SETUP_NWE(10) | AT91_SMC_SETUP_NCS_WR(10) |	\
		 AT91_SMC_SETUP_NRD(10) | AT91_SMC_SETUP_NCS_RD(10))
#define CONFIG_SYS_SMC0_PULSE0_VAL					\
		(AT91_SMC_PULSE_NWE(11) | AT91_SMC_PULSE_NCS_WR(11) |	\
		 AT91_SMC_PULSE_NRD(11) | AT91_SMC_PULSE_NCS_RD(11))
#define CONFIG_SYS_SMC0_CYCLE0_VAL	\
		(AT91_SMC_CYCLE_NWE(22) | AT91_SMC_CYCLE_NRD(22))
#define CONFIG_SYS_SMC0_MODE0_VAL				\
		(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |	\
		 AT91_SMC_MODE_DBW_16 |				\
		 AT91_SMC_MODE_TDF |				\
		 AT91_SMC_MODE_TDF_CYCLE(6))

/* user reset enable */
#define CONFIG_SYS_RSTC_RMR_VAL			\
		(AT91_RSTC_KEY |		\
		AT91_RSTC_CR_PROCRST |		\
		AT91_RSTC_MR_ERSTL(1) |	\
		AT91_RSTC_MR_ERSTL(2))

/* Disable Watchdog */
#define CONFIG_SYS_WDTC_WDMR_VAL				\
		(AT91_WDT_MR_WDIDLEHLT | AT91_WDT_MR_WDDBGHLT |	\
		 AT91_WDT_MR_WDV(0xfff) |					\
		 AT91_WDT_MR_WDDIS |				\
		 AT91_WDT_MR_WDD(0xfff))

/*
 * Hardware drivers
 */
/* LCD */
#define LCD_BPP				LCD_COLOR8

/* SDRAM */
#define PHYS_SDRAM		0x20000000
#define PHYS_SDRAM_SIZE		0x04000000	/* 64 megs */

/* NOR flash, if populated */
#define PHYS_FLASH_1			0x10000000
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#define CONFIG_SYS_MAX_FLASH_SECT	256

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_DBW_8		1
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	GPIO_PIN_PD(15)
#define CONFIG_SYS_NAND_READY_PIN	GPIO_PIN_PB(30)

#endif

/* PSRAM */
#define	PHYS_PSRAM			0x70000000
#define	PHYS_PSRAM_SIZE			0x00400000	/* 4MB */
/* Slave EBI1, PSRAM connected */
#define CONFIG_PSRAM_SCFG		(AT91_MATRIX_SCFG_ARBT_FIXED_PRIORITY	| \
					 AT91_MATRIX_SCFG_FIXED_DEFMSTR(5)	| \
					 AT91_MATRIX_SCFG_DEFMSTR_TYPE_FIXED	| \
					 AT91_MATRIX_SCFG_SLOT_CYCLE(255))

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_PLLB
#define CONFIG_USB_OHCI_NEW			1
#define CONFIG_SYS_USB_OHCI_CPU_INIT		1
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00a00000	/* AT91SAM9263_UHP_BASE */
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9263"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0"				\
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0"			\
	"partition=nand0,0\0"					\
	"ramargs=setenv bootargs $(bootargs) $(mtdparts)\0"	\
	"nfsargs=setenv bootargs root=/dev/nfs rw "		\
		"fbcon=rotate:3 "				\
		"nfsroot=$(serverip):$(rootpath) $(mtdparts)\0"	\
	"addip=setenv bootargs $(bootargs) "			\
		"ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask)"\
		":$(hostname):eth0:off\0"			\
	"ramboot=tftpboot 0x22000000 vmImage;"			\
		"run ramargs;run addip;bootm 22000000\0"	\
	"nfsboot=tftpboot 0x22000000 vmImage;"			\
		"run nfsargs;run addip;bootm 22000000\0"	\
	"flashboot=run ramargs;run addip;bootm 0x10050000\0"	\
	""

#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 - \
				GENERATED_GBL_DATA_SIZE)

#endif
