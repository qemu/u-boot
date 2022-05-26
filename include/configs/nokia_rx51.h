/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011-2012
 * Pali Rohár <pali@kernel.org>
 *
 * (C) Copyright 2010
 * Alistair Buxton <a.j.buxton@gmail.com>
 *
 * Derived from Beagle Board code:
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * Configuration settings for the Nokia RX-51 aka N900.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap.h>
#include <asm/arch/mem.h>
#include <linux/stringify.h>

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#define CONFIG_UBI_SIZE			(512 << 10)

/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK		48000000		/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * select serial console configuration
 */
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3

#define CONFIG_SYS_BAUDRATE_TABLE { 4800, 9600, 19200, 38400, 57600, 115200 }

/* USB device configuration */
#define CONFIG_USB_DEVICE
#define CONFIG_USB_TTY
#define CONFIG_USBD_VENDORID		0x0421
#define CONFIG_USBD_PRODUCTID_CDCACM	0x01c8
#define CONFIG_USBD_PRODUCTID_GSERIAL	0x01c8
#define CONFIG_USBD_MANUFACTURER	"Nokia"
#define CONFIG_USBD_PRODUCT_NAME	"N900 (U-Boot)"

#define GPIO_SLIDE			71

/*
 * Board ONENAND Info.
 */

#define CONFIG_SYS_ONENAND_BASE		ONENAND_MAP

/* Environment information */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"usbtty=cdc_acm\0" \
	"stdin=usbtty,serial,keyboard\0" \
	"stdout=usbtty,serial,vidconsole\0" \
	"stderr=usbtty,serial,vidconsole\0" \
	"slide=gpio input " __stringify(GPIO_SLIDE) "\0" \
	"switchmmc=mmc dev ${mmcnum}\0" \
	"kernaddr=0x82008000\0" \
	"initrdaddr=0x84008000\0" \
	"scriptaddr=0x86008000\0" \
	"fileload=${mmctype}load mmc ${mmcnum}:${mmcpart} " \
		"${loadaddr} ${mmcfile}\0" \
	"kernload=setenv loadaddr ${kernaddr};" \
		"setenv mmcfile ${mmckernfile};" \
		"run fileload\0" \
	"initrdload=setenv loadaddr ${initrdaddr};" \
		"setenv mmcfile ${mmcinitrdfile};" \
		"run fileload\0" \
	"scriptload=setenv loadaddr ${scriptaddr};" \
		"setenv mmcfile ${mmcscriptfile};" \
		"run fileload\0" \
	"scriptboot=echo Running ${mmcscriptfile} from mmc " \
		"${mmcnum}:${mmcpart} ...; source ${scriptaddr}\0" \
	"kernboot=echo Booting ${mmckernfile} from mmc " \
		"${mmcnum}:${mmcpart} ...; bootm ${kernaddr} || " \
			"bootz ${kernaddr}\0" \
	"kerninitrdboot=echo Booting ${mmckernfile} ${mmcinitrdfile} from mmc "\
		"${mmcnum}:${mmcpart} ...; bootm ${kernaddr} ${initrdaddr} || " \
			"bootz ${kernaddr} ${initrdaddr}\0" \
	"attachboot=echo Booting attached kernel image ...;" \
		"setenv setup_omap_atag 1;" \
		"bootm ${attkernaddr} || bootz ${attkernaddr};" \
		"setenv setup_omap_atag\0" \
	"trymmcscriptboot=run switchmmc && run scriptload && run scriptboot\0" \
	"trymmckernboot=run switchmmc && run kernload && run kernboot\0" \
	"trymmckerninitrdboot=run switchmmc && run initrdload && " \
		"run kernload && run kerninitrdboot\0" \
	"trymmcpartboot=setenv mmcscriptfile boot.scr; run trymmcscriptboot;" \
		"setenv mmckernfile uImage; run trymmckernboot;" \
		"setenv mmckernfile zImage; run trymmckernboot\0" \
	"trymmcallpartboot=setenv mmcpart 1; run trymmcpartboot;" \
		"setenv mmcpart 2; run trymmcpartboot;" \
		"setenv mmcpart 3; run trymmcpartboot;" \
		"setenv mmcpart 4; run trymmcpartboot\0" \
	"trymmcboot=if run switchmmc; then " \
			"setenv mmctype fat;" \
			"run trymmcallpartboot;" \
			"setenv mmctype ext4;" \
			"run trymmcallpartboot;" \
		"fi\0" \
	"emmcboot=setenv mmcnum 1; run trymmcboot\0" \
	"sdboot=setenv mmcnum 0; run trymmcboot\0" \
	"trymmcbootmenu=setenv mmctype fat && run trymmcscriptboot || " \
		"setenv mmctype ext4 && run trymmcscriptboot\0" \
	"preboot=setenv mmcpart 1; setenv mmcscriptfile bootmenu.scr;" \
		"setenv mmcnum 0 && run trymmcbootmenu || " \
		"setenv mmcnum 1 && run trymmcbootmenu;" \
		"if run slide; then true; else " \
			"setenv bootmenu_delay 0;" \
			"setenv bootdelay 0;" \
		"fi\0" \
	"menucmd=bootmenu\0" \
	"bootmenu_0=Attached kernel=run attachboot\0" \
	"bootmenu_1=Internal eMMC=run emmcboot\0" \
	"bootmenu_2=External SD card=run sdboot\0" \
	"bootmenu_3=U-Boot boot order=boot\0" \
	"bootmenu_delay=30\0" \
	""

#define CONFIG_POSTBOOTMENU \
	"echo;" \
	"echo Extra commands:;" \
	"echo run sdboot - Boot from SD card slot.;" \
	"echo run emmcboot - Boot internal eMMC memory.;" \
	"echo run attachboot - Boot attached kernel image.;" \
	"echo"

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)

/*
 * Physical Memory Map
 */
#define PHYS_SDRAM_1			OMAP34XX_SDRC_CS0

/*
 * FLASH and environment organization
 */

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800

/*
 * Attached kernel image
 */

#define SDRAM_SIZE			0x10000000	/* 256 MB */
#define SDRAM_END			(CONFIG_SYS_SDRAM_BASE + SDRAM_SIZE)

#define IMAGE_MAXSIZE			0x1FF800	/* 2 MB - 2 kB */
#define KERNEL_OFFSET			0x40000		/* 256 kB */
#define KERNEL_MAXSIZE			(IMAGE_MAXSIZE-KERNEL_OFFSET)
#define KERNEL_ADDRESS			(SDRAM_END-KERNEL_MAXSIZE)

/* Reserve protected RAM for attached kernel */
#define CONFIG_PRAM			((KERNEL_MAXSIZE >> 10)+1)

#endif /* __CONFIG_H */
