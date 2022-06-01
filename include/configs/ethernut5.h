/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * egnite GmbH <info@egnite.de>
 *
 * Configuation settings for Ethernut 5 with AT91SAM9XE.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/hardware.h>

/* The first stage boot loader expects u-boot running at this address. */

/* The first stage boot loader takes care of low level initialization. */

/* CPU information */

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768	/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000 /* 18.432 MHz crystal */

/* 32kB internal SRAM */
#define CONFIG_SYS_INIT_RAM_ADDR	0x00300000 /*AT91SAM9XE_SRAM_BASE */
#define CONFIG_SYS_INIT_RAM_SIZE	(32 << 10)

/* 128MB SDRAM in 1 bank */
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		(128 << 20)

/* 512kB on-chip NOR flash */
# define CONFIG_SYS_FLASH_BASE		0x00200000 /* AT91SAM9XE_FLASH_BASE */
# define CONFIG_SYS_MAX_FLASH_SECT	32


/* bootstrap + u-boot + env + linux in dataflash on CS0 */

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_DBW_8
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	GPIO_PIN_PC(14)
#endif

/* JFFS2 */

/* Ethernet */
#define CONFIG_PHY_ID			0
#define CONFIG_MACB_SEARCH_PHY

/* MMC */
#ifdef CONFIG_CMD_MMC
#define CONFIG_SYS_MMC_CD_PIN		AT91_PIO_PORTC, 8
#endif

/* USB */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_PLLB
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE	0x00500000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"host"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#endif

/* RTC */
#if defined(CONFIG_CMD_DATE) || defined(CONFIG_CMD_SNTP)
#define CONFIG_SYS_I2C_RTC_ADDR		0x51
#endif

/* I2C */
#define CONFIG_SYS_MAX_I2C_BUS	1

#define I2C_SOFT_DECLARATIONS

#define GPIO_I2C_SCL		AT91_PIO_PORTA, 24
#define GPIO_I2C_SDA		AT91_PIO_PORTA, 23

#define I2C_INIT { \
	at91_set_pio_periph(AT91_PIO_PORTA, 23, 0); \
	at91_set_pio_multi_drive(AT91_PIO_PORTA, 23, 1); \
	at91_set_pio_periph(AT91_PIO_PORTA, 24, 0); \
	at91_set_pio_output(AT91_PIO_PORTA, 24, 0); \
	at91_set_pio_multi_drive(AT91_PIO_PORTA, 24, 1); \
}

#define I2C_ACTIVE	at91_set_pio_output(AT91_PIO_PORTA, 23, 0)
#define I2C_TRISTATE	at91_set_pio_input(AT91_PIO_PORTA, 23, 0)
#define I2C_SCL(bit)	at91_set_pio_value(AT91_PIO_PORTA, 24, bit)
#define I2C_SDA(bit)	at91_set_pio_value(AT91_PIO_PORTA, 23, bit)
#define I2C_DELAY	udelay(100)
#define I2C_READ	at91_get_pio_value(AT91_PIO_PORTA, 23)

/* DHCP/BOOTP options */
#ifdef CONFIG_CMD_DHCP
#define CONFIG_SYS_AUTOLOAD	"n"
#endif

/* File systems */

/* Boot command */

/* Misc. u-boot settings */

#endif
