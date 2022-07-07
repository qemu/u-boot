/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2021 NXP
 */

#ifndef __L1028A_COMMON_H
#define __L1028A_COMMON_H

#include <asm/arch/stream_id_lsch3.h>
#include <asm/arch/config.h>
#include <asm/arch/soc.h>

/* Link Definitions */

#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000UL
#define CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_DDR_BLOCK2_BASE	0x2080000000ULL
#define CONFIG_SYS_FSL_DDR_MAIN_NUM_CTRLS	1

/*
 * SMP Definitinos
 */
#define CPU_RELEASE_ADDR		secondary_boot_addr

/* GPIO */

/* I2C */

/* Serial Port */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE     1
#define CONFIG_SYS_NS16550_CLK          (get_bus_freq(0) / 2)

/* Miscellaneous configurable options */

/* Physical Memory Map */

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		128

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

#define XSPI_NOR_BOOTCOMMAND	\
	"run xspi_hdploadcmd; run distro_bootcmd; run xspi_bootcmd; " \
	"env exists secureboot && esbc_halt;;"
#define SD_BOOTCOMMAND	\
	"run sd_hdploadcmd; run distro_bootcmd;run sd_bootcmd; " \
	"env exists secureboot && esbc_halt;"
#define SD2_BOOTCOMMAND	\
	"run emmc_hdploadcmd; run distro_bootcmd;run emmc_bootcmd; " \
	"env exists secureboot && esbc_halt;"

#define OCRAM_NONSECURE_SIZE		0x00010000
#define CONFIG_SYS_FSL_QSPI_BASE	0x20000000

/* I2C bus multiplexer */
#define I2C_MUX_PCA_ADDR_PRI            0x77 /* Primary Mux*/
#define I2C_MUX_CH_DEFAULT              0x8

/* EEPROM */
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_EEPROM_BUS_NUM		0

/* DisplayPort */
#define DP_PWD_EN_DEFAULT_MASK          0x8

#ifdef CONFIG_NXP_ESBC
#include <asm/fsl_secure_boot.h>
#endif

#endif /* __L1028A_COMMON_H */
