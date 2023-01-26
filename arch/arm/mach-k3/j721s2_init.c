// SPDX-License-Identifier: GPL-2.0+
/*
 * J721E: SoC specific initialization
 *
 * Copyright (C) 2021 Texas Instruments Incorporated - http://www.ti.com/
 *	David Huang <d-huang@ti.com>
 */

#include <common.h>
#include <init.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/armv7_mpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sysfw-loader.h>
#include "common.h"
#include <asm/arch/sys_proto.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <dm/pinctrl.h>
#include <mmc.h>
#include <remoteproc.h>

static void ctrl_mmr_unlock(void)
{
	/* Unlock all WKUP_CTRL_MMR0 module registers */
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 0);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 1);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 2);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 3);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 4);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 6);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 7);

	/* Unlock all MCU_CTRL_MMR0 module registers */
	mmr_unlock(MCU_CTRL_MMR0_BASE, 0);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 1);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 2);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 3);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 4);

	/* Unlock all CTRL_MMR0 module registers */
	mmr_unlock(CTRL_MMR0_BASE, 0);
	mmr_unlock(CTRL_MMR0_BASE, 1);
	mmr_unlock(CTRL_MMR0_BASE, 2);
	mmr_unlock(CTRL_MMR0_BASE, 3);
	mmr_unlock(CTRL_MMR0_BASE, 5);
	mmr_unlock(CTRL_MMR0_BASE, 7);
}

void k3_mmc_stop_clock(void)
{
	if (IS_ENABLED(CONFIG_K3_LOAD_SYSFW)) {
		if (spl_boot_device() == BOOT_DEVICE_MMC1) {
			struct mmc *mmc = find_mmc_device(0);

			if (!mmc)
				return;

			mmc->saved_clock = mmc->clock;
			mmc_set_clock(mmc, 0, true);
		}
	}
}

void k3_mmc_restart_clock(void)
{
	if (IS_ENABLED(CONFIG_K3_LOAD_SYSFW)) {
		if (spl_boot_device() == BOOT_DEVICE_MMC1) {
			struct mmc *mmc = find_mmc_device(0);

			if (!mmc)
				return;

			mmc_set_clock(mmc, mmc->saved_clock, false);
		}
	}
}

/*
 * This uninitialized global variable would normal end up in the .bss section,
 * but the .bss is cleared between writing and reading this variable, so move
 * it to the .data section.
 */
u32 bootindex __attribute__((section(".data")));
static struct rom_extended_boot_data bootdata __section(".data");

static void store_boot_info_from_rom(void)
{
	bootindex = *(u32 *)(CONFIG_SYS_K3_BOOT_PARAM_TABLE_INDEX);
	memcpy(&bootdata, (uintptr_t *)ROM_EXTENDED_BOOT_DATA_INFO,
	       sizeof(struct rom_extended_boot_data));
}

void k3_spl_init(void)
{
	struct udevice *dev;
	int ret;
	/*
	 * Cannot delay this further as there is a chance that
	 * K3_BOOT_PARAM_TABLE_INDEX can be over written by SPL MALLOC section.
	 */
	store_boot_info_from_rom();

	/* Make all control module registers accessible */
	ctrl_mmr_unlock();

	if (IS_ENABLED(CONFIG_CPU_V7R)) {
		disable_linefill_optimization();
		setup_k3_mpu_regions();
	}

	/* Init DM early */
	spl_early_init();

	/* Prepare console output */
	preloader_console_init();

	if (IS_ENABLED(CONFIG_K3_LOAD_SYSFW)) {
		/*
		 * Process pinctrl for the serial0 a.k.a. WKUP_UART0 module and continue
		 * regardless of the result of pinctrl. Do this without probing the
		 * device, but instead by searching the device that would request the
		 * given sequence number if probed. The UART will be used by the system
		 * firmware (SYSFW) image for various purposes and SYSFW depends on us
		 * to initialize its pin settings.
		 */
		ret = uclass_find_device_by_seq(UCLASS_SERIAL, 0, &dev);
		if (!ret)
			pinctrl_select_state(dev, "default");

		/*
		 * Load, start up, and configure system controller firmware. Provide
		 * the U-Boot console init function to the SYSFW post-PM configuration
		 * callback hook, effectively switching on (or over) the console
		 * output.
		 */
		k3_sysfw_loader(is_rom_loaded_sysfw(&bootdata),
				k3_mmc_stop_clock, k3_mmc_restart_clock);

		if (IS_ENABLED(CONFIG_SPL_CLK_K3)) {
			/*
			 * Force probe of clk_k3 driver here to ensure basic default clock
			 * configuration is always done for enabling PM services.
			 */
			ret = uclass_get_device_by_driver(UCLASS_CLK,
							  DM_DRIVER_GET(ti_clk),
							  &dev);
			if (ret)
				panic("Failed to initialize clk-k3!\n");
		}
	}

	/* Output System Firmware version info */
	k3_sysfw_print_ver();
}

bool check_rom_loaded_sysfw(void)
{
	return is_rom_loaded_sysfw(&bootdata);
}

void k3_mem_init(void)
{
	struct udevice *dev;
	int ret;

	if (IS_ENABLED(CONFIG_TARGET_J721S2_R5_EVM)) {
		ret = uclass_get_device_by_name(UCLASS_MISC, "msmc", &dev);
		if (ret)
			panic("Probe of msmc failed: %d\n", ret);

		ret = uclass_get_device(UCLASS_RAM, 0, &dev);
		if (ret)
			panic("DRAM 0 init failed: %d\n", ret);

		ret = uclass_next_device_err(&dev);
		if (ret)
			panic("DRAM 1 init failed: %d\n", ret);
	}
	spl_enable_dcache();
}

u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		return MMCSD_MODE_EMMCBOOT;
	case BOOT_DEVICE_MMC2:
		return MMCSD_MODE_FS;
	default:
		return MMCSD_MODE_RAW;
	}
}

static u32 __get_backup_bootmedia(u32 main_devstat)
{
	u32 bkup_boot = (main_devstat & MAIN_DEVSTAT_BKUP_BOOTMODE_MASK) >>
			MAIN_DEVSTAT_BKUP_BOOTMODE_SHIFT;

	switch (bkup_boot) {
	case BACKUP_BOOT_DEVICE_USB:
		return BOOT_DEVICE_DFU;
	case BACKUP_BOOT_DEVICE_UART:
		return BOOT_DEVICE_UART;
	case BACKUP_BOOT_DEVICE_ETHERNET:
		return BOOT_DEVICE_ETHERNET;
	case BACKUP_BOOT_DEVICE_MMC2:
	{
		u32 port = (main_devstat & MAIN_DEVSTAT_BKUP_MMC_PORT_MASK) >>
			    MAIN_DEVSTAT_BKUP_MMC_PORT_SHIFT;
		if (port == 0x0)
			return BOOT_DEVICE_MMC1;
		return BOOT_DEVICE_MMC2;
	}
	case BACKUP_BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;
	case BACKUP_BOOT_DEVICE_I2C:
		return BOOT_DEVICE_I2C;
	}

	return BOOT_DEVICE_RAM;
}

static u32 __get_primary_bootmedia(u32 main_devstat, u32 wkup_devstat)
{
	u32 bootmode = (wkup_devstat & WKUP_DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
			WKUP_DEVSTAT_PRIMARY_BOOTMODE_SHIFT;

	bootmode |= (main_devstat & MAIN_DEVSTAT_BOOT_MODE_B_MASK) <<
			BOOT_MODE_B_SHIFT;

	if (bootmode == BOOT_DEVICE_OSPI || bootmode ==	BOOT_DEVICE_QSPI ||
	    bootmode == BOOT_DEVICE_XSPI)
		bootmode = BOOT_DEVICE_SPI;

	if (bootmode == BOOT_DEVICE_MMC2) {
		u32 port = (main_devstat &
			    MAIN_DEVSTAT_PRIM_BOOTMODE_MMC_PORT_MASK) >>
			   MAIN_DEVSTAT_PRIM_BOOTMODE_PORT_SHIFT;
		if (port == 0x0)
			bootmode = BOOT_DEVICE_MMC1;
	}

	return bootmode;
}

u32 spl_boot_device(void)
{
	u32 wkup_devstat = readl(CTRLMMR_WKUP_DEVSTAT);
	u32 main_devstat;

	if (wkup_devstat & WKUP_DEVSTAT_MCU_OMLY_MASK) {
		printf("ERROR: MCU only boot is not yet supported\n");
		return BOOT_DEVICE_RAM;
	}

	/* MAIN CTRL MMR can only be read if MCU ONLY is 0 */
	main_devstat = readl(CTRLMMR_MAIN_DEVSTAT);

	if (bootindex == K3_PRIMARY_BOOTMODE)
		return __get_primary_bootmedia(main_devstat, wkup_devstat);
	else
		return __get_backup_bootmedia(main_devstat);
}

#define J721S2_DEV_MCU_RTI0			295
#define J721S2_DEV_MCU_RTI1			296
#define J721S2_DEV_MCU_ARMSS0_CPU0		284
#define J721S2_DEV_MCU_ARMSS0_CPU1		285

void release_resources_for_core_shutdown(void)
{
	if (IS_ENABLED(CONFIG_SYS_K3_SPL_ATF)) {
		struct ti_sci_handle *ti_sci;
		struct ti_sci_dev_ops *dev_ops;
		struct ti_sci_proc_ops *proc_ops;
		int ret;
		u32 i;

		const u32 put_device_ids[] = {
			J721S2_DEV_MCU_RTI0,
			J721S2_DEV_MCU_RTI1,
		};

		ti_sci = get_ti_sci_handle();
		dev_ops = &ti_sci->ops.dev_ops;
		proc_ops = &ti_sci->ops.proc_ops;

		/* Iterate through list of devices to put (shutdown) */
		for (i = 0; i < ARRAY_SIZE(put_device_ids); i++) {
			u32 id = put_device_ids[i];

			ret = dev_ops->put_device(ti_sci, id);
			if (ret)
				panic("Failed to put device %u (%d)\n", id, ret);
		}

		const u32 put_core_ids[] = {
			J721S2_DEV_MCU_ARMSS0_CPU1,
			J721S2_DEV_MCU_ARMSS0_CPU0,	/* Handle CPU0 after CPU1 */
		};

		/* Iterate through list of cores to put (shutdown) */
		for (i = 0; i < ARRAY_SIZE(put_core_ids); i++) {
			u32 id = put_core_ids[i];

			/*
			 * Queue up the core shutdown request. Note that this call
			 * needs to be followed up by an actual invocation of an WFE
			 * or WFI CPU instruction.
			 */
			ret = proc_ops->proc_shutdown_no_wait(ti_sci, id);
			if (ret)
				panic("Failed sending core %u shutdown message (%d)\n",
				      id, ret);
		}
	}
}
