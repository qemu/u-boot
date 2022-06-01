/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 samtec automotive software & electronics gmbh
 *
 * Configuration settings for the Samtec VIN|ING 2000 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

#ifdef CONFIG_SPL
#include "imx6_spl.h"
#endif

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* MMC Configuration */
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC4_BASE_ADDR

/* PMIC */
#define CONFIG_POWER_PFUZE100
#define CONFIG_POWER_PFUZE100_I2C_ADDR	0x08

/* Network */
#define CONFIG_FEC_MXC_PHYADDR          0x0

#define CONFIG_MXC_USB_PORTSC  (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS   0
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2

#ifdef CONFIG_CMD_PCI
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCIE_IMX
#define CONFIG_PCIE_IMX_PERST_GPIO	IMX_GPIO_NR(4, 6)
#endif

#define CONFIG_IMX6_PWM_PER_CLK 66000000

#ifdef CONFIG_ENV_IS_IN_MMC
/* 0=user, 1=boot0, 2=boot1, * 4..7=general0..3. */
#endif

#ifdef CONFIG_SPL_BUILD
#define CONFIG_MXC_UART_BASE		UART1_BASE
#endif

#endif				/* __CONFIG_H */
