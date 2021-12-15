// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/gcr.h>
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_NPCMX50;
	gd->bd->bi_boot_params = (CONFIG_SYS_SDRAM_BASE + 0x100UL);

	return 0;
}

int dram_init(void)
{
	struct npcm_gcr *gcr = (struct npcm_gcr *)NPCM_GCR_BA;

	/*
	 * get dram active size value from bootblock.
	 * Value sent using scrpad_02 register.
	 */
	gd->ram_size = readl(&gcr->scrpad_b);

	return 0;
}
