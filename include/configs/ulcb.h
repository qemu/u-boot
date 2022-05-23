/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/ulcb.h
 *     This file is ULCB board configuration.
 *
 * Copyright (C) 2017 Renesas Electronics Corporation
 */

#ifndef __ULCB_H
#define __ULCB_H

#include "rcar-gen3-common.h"

/* Environment in eMMC, at the end of 2nd "boot sector" */

#define CONFIG_FLASH_SHOW_PROGRESS	45
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_SYS_FLASH_BANKS_LIST	{ 0x08000000 }
#define CONFIG_SYS_MAX_FLASH_SECT	256
#define CONFIG_SYS_WRITE_SWAPPED_DATA

#endif /* __ULCB_H */
