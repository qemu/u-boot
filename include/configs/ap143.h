/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_MHZ                  325
#define CONFIG_SYS_MIPS_TIMER_FREQ      (CONFIG_SYS_MHZ * 1000000)

#define CONFIG_SYS_SDRAM_BASE           0x80000000

#define CONFIG_SYS_INIT_RAM_ADDR        0xbd000000
#define CONFIG_SYS_INIT_RAM_SIZE        0x2000

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_CLK          25000000

/* Miscellaneous configurable options */

/*
 * Diagnostics
 */

#endif  /* __CONFIG_H */
