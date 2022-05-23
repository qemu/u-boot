/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT7629 SoC
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#ifndef __MT7629_H
#define __MT7629_H

#include <linux/sizes.h>

/* Miscellaneous configurable options */

#define CONFIG_SYS_BOOTM_LEN		SZ_64M

#define CONFIG_SYS_NONCACHED_MEMORY	SZ_1M

/* Environment */

/* Defines for SPL */
#define CONFIG_SPL_STACK		0x106000
#define CONFIG_SPL_MAX_FOOTPRINT	SZ_64K

#define CONFIG_SPI_ADDR			0x30000000
#define CONFIG_SYS_UBOOT_BASE		(CONFIG_SPI_ADDR + CONFIG_SPL_PAD_TO)

/* SPL -> Uboot */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE + SZ_2M - \
					 GENERATED_GBL_DATA_SIZE)

/* UBoot -> Kernel */
#define CONFIG_SYS_SPL_ARGS_ADDR	0x40000000

/* DRAM */
#define CONFIG_SYS_SDRAM_BASE		0x40000000

/* Ethernet */
#define CONFIG_IPADDR			192.168.1.1
#define CONFIG_SERVERIP			192.168.1.2

#endif
