// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020-2021 SiFive, Inc
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <init.h>
#include <spl.h>
#include <misc.h>
#include <log.h>
#include <fdtdec.h>
#include <dm/root.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/spl.h>
#include <asm/arch/eeprom.h>

#define GEM_PHY_RESET	SIFIVE_GENERIC_GPIO_NR(0, 12)

#define MODE_SELECT_REG		0x1000
#define MODE_SELECT_SD		0xb
#define MODE_SELECT_MASK	GENMASK(3, 0)

int spl_board_init_f(void)
{
	int ret;

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_MULTI_DTB_FIT)
	int rescan;

	ret = fdtdec_resetup(&rescan);
	if (!ret && rescan) {
		dm_uninit();
		dm_init_and_scan(true);
	}
#endif

	ret = spl_soc_init();
	if (ret) {
		debug("HiFive Unmatched FU740 SPL init failed: %d\n", ret);
		return ret;
	}

	/*
	 * GEMGXL init VSC8541 PHY reset sequence;
	 * leave pull-down active for 2ms
	 */
	udelay(2000);
	ret = gpio_request(GEM_PHY_RESET, "gem_phy_reset");
	if (ret) {
		debug("gem_phy_reset gpio request failed: %d\n", ret);
		return ret;
	}

	/* Set GPIO 12 (PHY NRESET) */
	ret = gpio_direction_output(GEM_PHY_RESET, 1);
	if (ret) {
		debug("gem_phy_reset gpio direction set failed: %d\n", ret);
		return ret;
	}

	udelay(1);

	/* Reset PHY again to enter unmanaged mode */
	gpio_set_value(GEM_PHY_RESET, 0);
	udelay(1);
	gpio_set_value(GEM_PHY_RESET, 1);
	mdelay(15);

	return 0;
}

u32 spl_boot_device(void)
{
	u32 mode_select = readl((void *)MODE_SELECT_REG);
	u32 boot_device = mode_select & MODE_SELECT_MASK;

	switch (boot_device) {
	case MODE_SELECT_SD:
		return BOOT_DEVICE_MMC1;
	default:
		debug("Unsupported boot device 0x%x but trying MMC1\n",
		      boot_device);
		return BOOT_DEVICE_MMC1;
	}
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/*
	 * Apply different DDR params on different board revision.
	 * Use PCB revision which is byte 0x7 in I2C platform EEPROM
	 * to distinguish that.
	 */
	if (get_pcb_revision_from_eeprom() == PCB_REVISION_REV3 &&
	    !strcmp(name, "hifive-unmatched-a00"))
		return 0;
	else if (get_pcb_revision_from_eeprom() != PCB_REVISION_REV3 &&
		 !strcmp(name, "hifive-unmatched-a00-rev1"))
		return 0;

	return -1;
}
#endif
