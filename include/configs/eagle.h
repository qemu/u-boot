/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/eagle.h
 *     This file is Eagle board configuration.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 */

#ifndef __EAGLE_H
#define __EAGLE_H

#include "rcar-gen3-common.h"

/* Environment compatibility */

/* Board Clock */
/* XTAL_CLK : 33.33MHz */

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

#endif /* __EAGLE_H */
