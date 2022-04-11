// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <spl.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/ddr.h>
#include <asm/mach-imx/boot_mode.h>

#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

#include <power/pmic.h>
#include <power/bd71837.h>

#include "lpddr4_timing.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)
#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE)

static const iomux_v3_cfg_t uart_pads[] = {
	IMX8MM_PAD_UART3_RXD_UART3_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MM_PAD_UART3_TXD_UART3_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static const iomux_v3_cfg_t wdog_pads[] = {
	IMX8MM_PAD_GPIO1_IO02_WDOG1_WDOG_B  | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

static void data_modul_imx8mm_edm_sbc_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

static int data_modul_imx8mm_edm_sbc_board_power_init(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("pmic@4b", &dev);
	if (ret == -ENODEV) {
		puts("Failed to get PMIC\n");
		return 0;
	}
	if (ret != 0)
		return ret;

	/* Unlock the PMIC regs */
	pmic_reg_write(dev, BD718XX_REGLOCK, 0x1);

	/* Increase VDD_SOC to typical value 0.85V before first DRAM access */
	pmic_reg_write(dev, BD718XX_BUCK1_VOLT_RUN, 0x0f);

	/* Increase VDD_DRAM to 0.975V for 3GHz DDR */
	pmic_reg_write(dev, BD718XX_1ST_NODVS_BUCK_VOLT, 0x83);

	/* Lock the PMIC regs */
	pmic_reg_write(dev, BD718XX_REGLOCK, 0x11);

	return 0;
}

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	if (boot_dev_spl == MMC3_BOOT)
		return BOOT_DEVICE_MMC2;	/* eMMC */
	else
		return BOOT_DEVICE_MMC1;	/* SD */
}

void board_boot_order(u32 *spl_boot_list)
{
	int boot_device = spl_boot_device();

	spl_boot_list[0] = boot_device;		/* 1:SD 2:eMMC */

	if (boot_device == BOOT_DEVICE_MMC1)
		spl_boot_list[1] = BOOT_DEVICE_MMC2;	/* eMMC */
	else
		spl_boot_list[1] = BOOT_DEVICE_MMC1;	/* SD */

	spl_boot_list[2] = BOOT_DEVICE_UART;	/* YModem */
	spl_boot_list[3] = BOOT_DEVICE_NONE;
}

static struct dram_timing_info *dram_timing_info[8] = {
	&dmo_imx8mm_sbc_dram_timing_32_32,	/* 32 Gbit x32 */
	NULL,					/* 32 Gbit x16 */
	&dmo_imx8mm_sbc_dram_timing_16_32,	/* 16 Gbit x32 */
	NULL,					/* 16 Gbit x16 */
	NULL,					/* 8 Gbit x32 */
	NULL,					/* 8 Gbit x16 */
	NULL,					/* INVALID */
	NULL,					/* INVALID */
};

static void spl_dram_init(void)
{
	u8 memcfg = dmo_get_memcfg();
	int i;

	printf("DDR:   %d GiB x%d [0x%x]\n",
	       /* 0..4 GiB, 1..2 GiB, 0..1 GiB */
	       4 >> ((memcfg >> 1) & 0x3),
	       /* 0..x32, 1..x16 */
	       32 >> (memcfg & BIT(0)),
	       memcfg);

	if (!dram_timing_info[memcfg]) {
		printf("Unsupported DRAM strapping, trying lowest supported. MEMCFG=0x%x\n",
		       memcfg);
		for (i = ARRAY_SIZE(dram_timing_info) - 1; i >= 0; i--)
			if (dram_timing_info[i])	/* Configuration found */
				break;
	}

	ddr_init(dram_timing_info[memcfg]);
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	icache_enable();

	arch_cpu_init();

	init_uart_clk(2);

	data_modul_imx8mm_edm_sbc_early_init_f();

	preloader_console_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device_by_name(UCLASS_CLK,
					"clock-controller@30380000",
					&dev);
	if (ret < 0) {
		printf("Failed to find clock node. Check device tree\n");
		hang();
	}

	enable_tzc380();

	data_modul_imx8mm_edm_sbc_board_power_init();

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}
