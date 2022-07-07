/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 */

#ifndef __CONFIG_RK3128_COMMON_H
#define __CONFIG_RK3128_COMMON_H

#include "rockchip-common.h"

#define CONFIG_SYS_HZ_CLOCK		24000000

#define CONFIG_IRAM_BASE		0x10080000

/* RAW SD card / eMMC locations. */

#define CONFIG_SYS_SDRAM_BASE		0x60000000
#define SDRAM_MAX_SIZE			0x80000000

/* usb mass storage */

#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x60500000\0" \
	"pxefile_addr_r=0x60600000\0" \
	"fdt_addr_r=0x61f00000\0" \
	"kernel_addr_r=0x62000000\0" \
	"ramdisk_addr_r=0x64000000\0"

#include <config_distro_bootcmd.h>
#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_MEM_LAYOUT_SETTINGS \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"partitions=" PARTS_DEFAULT \
	BOOTENV

#endif
