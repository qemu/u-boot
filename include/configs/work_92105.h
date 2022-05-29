/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * WORK Microwave work_92105 board configuration file
 *
 * (C) Copyright 2014  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 */

#ifndef __CONFIG_WORK_92105_H__
#define __CONFIG_WORK_92105_H__

/* SoC and board defines */
#include <linux/sizes.h>
#include <asm/arch/cpu.h>

/*
 * Memory configurations
 */
#define CONFIG_SYS_SDRAM_BASE		EMC_DYCS0_BASE
#define CONFIG_SYS_SDRAM_SIZE		SZ_128M

#define CONFIG_RTC_DS1374

/*
 * U-Boot General Configurations
 */

/*
 * NAND chip timings for FIXME: which one?
 */

#define CONFIG_LPC32XX_NAND_MLC_TCEA_DELAY  333333333
#define CONFIG_LPC32XX_NAND_MLC_BUSY_DELAY   10000000
#define CONFIG_LPC32XX_NAND_MLC_NAND_TA      18181818
#define CONFIG_LPC32XX_NAND_MLC_RD_HIGH      31250000
#define CONFIG_LPC32XX_NAND_MLC_RD_LOW       45454545
#define CONFIG_LPC32XX_NAND_MLC_WR_HIGH      40000000
#define CONFIG_LPC32XX_NAND_MLC_WR_LOW       83333333

/*
 * NAND
 */

/* driver configuration */
#define CONFIG_SYS_MAX_NAND_DEVICE 1
#define CONFIG_SYS_MAX_NAND_CHIPS 1
#define CONFIG_SYS_NAND_BASE MLC_NAND_BASE

/*
 * GPIO
 */

/*
 * Environment
 */

/*
 * SPL
 */

/* SPL will be executed at offset 0 */
/* SPL will use SRAM as stack */
/* Use the framework and generic lib */
/* SPL will use serial */
/* SPL will load U-Boot from NAND offset 0x40000 */
/* U-Boot will be 0x40000 bytes, loaded and run at CONFIG_SYS_TEXT_BASE */
#define CONFIG_SYS_MONITOR_LEN 0x40000 /* actually, MAX size */
#define CONFIG_SYS_NAND_U_BOOT_START CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_DST   CONFIG_SYS_TEXT_BASE

/*
 * Include SoC specific configuration
 */
#include <asm/arch/config.h>

#endif  /* __CONFIG_WORK_92105_H__*/
