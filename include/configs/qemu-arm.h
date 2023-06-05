/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Tuomas Tynkkynen
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* Physical memory map */
#define CFG_SYS_SDRAM_BASE		0x40000000

/* For timer, QEMU emulates an ARMv7/ARMv8 architected timer */

#define GICD_BASE		0x8000000	/* 24MHz, FPGA runs at 5MHz */
#define GICR_BASE		0x80a0000	/* 24MHz, FPGA runs at 5MHz */

#endif /* __CONFIG_H */
