/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board configuration file for Samsung S9(SM-G9600)(starqltechn)
 *
 * (C) Copyright 2017 Jorge Ramirez-Ortiz <jorge.ramirez-ortiz@linaro.org>
 */

#ifndef __CONFIGS_STARQLTECHN_H
#define __CONFIGS_STARQLTECHN_H

#include <configs/sdm845.h>

#define CONFIG_LNX_KRNL_IMG_TEXT_OFFSET_BASE    CONFIG_SYS_LOAD_ADDR
#define CONFIG_LINUX_KERNEL_IMAGE_HEADER

#endif
