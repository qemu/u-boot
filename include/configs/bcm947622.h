/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2022 Broadcom Ltd.
 */

#ifndef __BCM947622_H
#define __BCM947622_H
#include <linux/sizes.h>

#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BOOTM_LEN		(32 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x00000000

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE + SZ_16M)

#define COUNTER_FREQUENCY		50000000
#endif
