/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016, Imagination Technologies Ltd.
 *
 * Zubair Lutfullah Kakakhel <Zubair.Kakakhel@imgtec.com>
 *
 * Imagination Technologies Ltd. MIPSfpga
 */

#ifndef __XILFPGA_CONFIG_H
#define __XILFPGA_CONFIG_H

/* BootROM + MIG is pretty smart. DDR and Cache initialized */

/*--------------------------------------------
 * CPU configuration
 */
/* CPU Timer rate */
#define CONFIG_SYS_MIPS_TIMER_FREQ	50000000

/*----------------------------------------------------------------------
 * Memory Layout
 */

/* SDRAM Configuration (for final code, data, stack, heap) */
#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_SDRAM_SIZE		0x08000000	/* 128 Mbytes */

/*----------------------------------------------------------------------
 * Commands
 */

/*------------------------------------------------------------
 * Console Configuration
 */

/* -------------------------------------------------
 * Environment
 */

/* ---------------------------------------------------------------------
 * Board boot configuration
 */

#endif	/* __XILFPGA_CONFIG_H */
