/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT8512 SoC
 *
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Mingming Lee <mingming.lee@mediatek.com>
 */

#ifndef __MT8512_H
#define __MT8512_H

#include <linux/sizes.h>

#define CONFIG_SYS_NONCACHED_MEMORY		SZ_1M

/* Uboot definition */
#define CONFIG_SYS_UBOOT_START			CONFIG_SYS_TEXT_BASE

#define ENV_BOOT_READ_IMAGE \
	"boot_rd_img=mmc dev 0" \
	";mmc read ${loadaddr} 0x27000 0x8000" \
	";iminfo ${loadaddr}\0"

/* Console configuration */
#define ENV_DEVICE_SETTINGS \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"stderr=serial\0"

#define ENV_BOOT_CMD \
	"mtk_boot=run boot_rd_img;bootm;\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_high=0x6c000000\0" \
	ENV_DEVICE_SETTINGS \
	ENV_BOOT_READ_IMAGE \
	ENV_BOOT_CMD \
	"bootcmd=run mtk_boot;\0" \

#endif
