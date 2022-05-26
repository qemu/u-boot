/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Bluewater Systems Snapper 9260 and 9G20 modules
 *
 * (C) Copyright 2011 Bluewater Systems
 *   Author: Andre Renaud <andre@bluewatersys.com>
 *   Author: Ryan Mallon <ryan@bluewatersys.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* SoC type is defined in boards.cfg */
#include <asm/hardware.h>
#include <linux/sizes.h>

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000 /* External Crystal, in Hz */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768

/* CPU */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CONFIG_SYS_SDRAM_SIZE		(64 * 1024 * 1024) /* 64MB */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_INIT_RAM_ADDR	ATMEL_BASE_SRAM1

/* Mem test settings */

/* NAND Flash */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21) /* AD21 */
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22) /* AD22 */
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIN_PC13

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_PLLB
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE	ATMEL_UHP_BASE
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"at91sam9260"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2

/* GPIOs and IO expander */
#define CONFIG_PCA953X
#define CONFIG_SYS_I2C_PCA953X_ADDR	0x28
#define CONFIG_SYS_I2C_PCA953X_WIDTH	{ {0x28, 16} }

/* UARTs/Serial console */
#ifndef CONFIG_DM_SERIAL
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define CONFIG_USART_ID			ATMEL_ID_SYS
#endif

/* I2C - Bit-bashed */
#define CONFIG_SOFT_I2C_READ_REPEATED_START
#define I2C_INIT do {							\
		at91_set_gpio_output(AT91_PIN_PA23, 1);			\
		at91_set_gpio_output(AT91_PIN_PA24, 1);			\
		at91_set_pio_multi_drive(AT91_PIO_PORTA, 23, 1);	\
		at91_set_pio_multi_drive(AT91_PIO_PORTA, 24, 1);	\
	} while (0)
#define I2C_SOFT_DECLARATIONS
#define I2C_ACTIVE
#define I2C_TRISTATE	at91_set_gpio_input(AT91_PIN_PA23, 1);
#define I2C_READ	at91_get_gpio_value(AT91_PIN_PA23);
#define I2C_SDA(bit) do {						\
		if (bit) {						\
			at91_set_gpio_input(AT91_PIN_PA23, 1);		\
		} else {						\
			at91_set_gpio_output(AT91_PIN_PA23, 1);		\
			at91_set_gpio_value(AT91_PIN_PA23, bit);	\
		}							\
	} while (0)
#define I2C_SCL(bit)	at91_set_pio_value(AT91_PIO_PORTA, 24, bit)
#define I2C_DELAY	udelay(2)

/* Boot options */

/* Environment settings */

/* Console settings */

#endif /* __CONFIG_H */
