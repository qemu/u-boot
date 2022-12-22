/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT7629 SoC
 *
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#ifndef __MT7622_H
#define __MT7622_H

/* Uboot definition */
#define CFG_SYS_UBOOT_BASE                   CONFIG_TEXT_BASE

/* SPL -> Uboot */
#define CFG_SYS_UBOOT_START		CONFIG_TEXT_BASE
/* DRAM */
#define CFG_SYS_SDRAM_BASE		0x40000000

/* Ethernet */

#endif
