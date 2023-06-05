/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  (C) Copyright 2010,2012
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2022
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#include "tegra30-common.h"
#include "transformer-common.h"

#ifdef CONFIG_TRANSFORMER_SPI_BOOT
#define TRANSFORMER_BUTTON_ACTIONS \
	"then echo Starting SPI flash update ...;" \
		"run update_spi; fi;"
#else
#define TRANSFORMER_BUTTON_ACTIONS \
	"then bootmenu; fi;"
#endif

/* Board-specific serial config */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#include "tegra-common-post.h"

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND \
	"if test " __stringify(CONFIG_TRANSFORMER_HALL_GPIO) " -ne 0;" \
	"then setenv gpio_button " \
	__stringify(CONFIG_TRANSFORMER_HALL_GPIO) ";" \
	"if run check_button; then poweroff; fi; fi;" \
	"setenv gpio_button " \
	__stringify(CONFIG_TRANSFORMER_ACTION_GPIO) ";" \
	"if run check_button;" \
	TRANSFORMER_BUTTON_ACTIONS \
	"run bootcmd_mmc1;" \
	"run bootcmd_mmc0;" \
	"poweroff;"

#endif /* __CONFIG_H */
