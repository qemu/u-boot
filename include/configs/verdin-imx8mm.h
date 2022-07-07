/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020-2021 Toradex
 */

#ifndef __VERDIN_IMX8MM_H
#define __VERDIN_IMX8MM_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define CONFIG_SYS_MONITOR_LEN		SZ_512K
#define CONFIG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CONFIG_MALLOC_F_ADDR		0x930000
/* For RAW image gives a error info not panic */
#endif

#define MEM_LAYOUT_ENV_SETTINGS \
	"fdt_addr_r=0x44000000\0" \
	"kernel_addr_r=0x42000000\0" \
	"ramdisk_addr_r=0x46400000\0" \
	"scriptaddr=0x46000000\0"

/* Enable Distro Boot */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	MEM_LAYOUT_ENV_SETTINGS \
	"bootcmd_mfg=fastboot 0\0" \
	"boot_file=Image\0" \
	"boot_script_dhcp=boot.scr\0" \
	"console=ttymxc0\0" \
	"fdt_addr=0x43000000\0" \
	"fdt_board=dev\0" \
	"initrd_addr=0x43800000\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"setup=setenv setupargs console=tty1 console=${console},${baudrate} " \
		"consoleblank=0 earlycon\0" \
	"update_uboot=askenv confirm Did you load flash.bin (y/N)?; " \
		"if test \"$confirm\" = \"y\"; then " \
		"setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt " \
		"${blkcnt} / 0x200; mmc dev 0 1; mmc write ${loadaddr} 0x2 " \
		"${blkcnt}; fi\0"

#define CONFIG_SYS_INIT_RAM_ADDR        0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE        SZ_2M

#if defined(CONFIG_ENV_IS_IN_MMC)
/* Environment in eMMC, before config block at the end of 1st "boot sector" */
#endif

#define CONFIG_SYS_SDRAM_BASE           0x40000000

/* SDRAM configuration */
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			SZ_2G /* 2GB DDR */

/* ENET */
#define CONFIG_FEC_MXC_PHYADDR          7

/* USB Configs */
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)

#endif /* __VERDIN_IMX8MM_H */
