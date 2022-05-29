/* SPDX-License-Identifier: GPL-2.0 */
/*
 * include/configs/rcar-gen2-common.h
 *
 * Copyright (C) 2013,2014 Renesas Electronics Corporation
 */

#ifndef __RCAR_GEN2_COMMON_H
#define __RCAR_GEN2_COMMON_H

#include <asm/arch/rmobile.h>

#ifndef CONFIG_PINCTRL_PFC
#define CONFIG_SH_GPIO_PFC
#endif

/* console */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 38400, 115200 }

#define CONFIG_SYS_SDRAM_BASE		(RCAR_GEN2_SDRAM_BASE)
#define CONFIG_SYS_SDRAM_SIZE		(RCAR_GEN2_UBOOT_SDRAM_SIZE)

#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)

/* Timer */
#define CONFIG_TMU_TIMER
#define CONFIG_SYS_TIMER_COUNTS_DOWN
#define CONFIG_SYS_TIMER_COUNTER	(TMU_BASE + 0xc)	/* TCNT0 */
#define CONFIG_SYS_TIMER_RATE		(get_board_sys_clk() / 8)

#endif	/* __RCAR_GEN2_COMMON_H */
