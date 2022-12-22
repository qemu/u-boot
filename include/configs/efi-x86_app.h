/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=serial\0" \
					"stdout=vidconsole\0" \
					"stderr=vidconsole\0"

#endif
