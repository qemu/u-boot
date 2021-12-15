/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#ifndef __CONFIG_ARBEL_H
#define __CONFIG_ARBEL_H

#define CONFIG_GICV2
#define GICD_BASE			(0xDFFF9000)
#define GICC_BASE			(0xDFFFA000)

#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS   1
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SYS_MAXARGS              32
#define CONFIG_SYS_CBSIZE               256
#define CONFIG_SYS_PBSIZE               (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_PROMPT_HUSH_PS2	    "> "
#define CONFIG_SYS_BOOTM_LEN            (20 << 20)
#define CONFIG_SYS_BOOTMAPSZ            (20 << 20)
#define CONFIG_SYS_SDRAM_BASE           0x0
#define CONFIG_SYS_INIT_SP_ADDR         (0x00008000 - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_MONITOR_LEN          (256 << 10)
#define CONFIG_SYS_MONITOR_BASE	        CONFIG_SYS_TEXT_BASE
#define CONFIG_BAUDRATE                 115200
#define CONFIG_SYS_HZ                   1000
#define CONFIG_BITBANGMII_MULTI

/* Default environemnt variables */
#define CONFIG_BOOTCOMMAND "run common_bootargs; run romboot"
#define CONFIG_EXTRA_ENV_SETTINGS   "uimage_flash_addr=80200000\0"   \
		"stdin=serial\0"   \
		"stdout=serial\0"   \
		"stderr=serial\0"    \
		"ethact=gmac1\0"   \
		"autostart=no\0"   \
		"ethaddr=00:00:F7:A0:00:FC\0"    \
		"eth1addr=00:00:F7:A0:00:FD\0"   \
		"eth2addr=00:00:F7:A0:00:FE\0"    \
		"eth3addr=00:00:F7:A0:00:FF\0"    \
		"gatewayip=192.168.0.17\0"    \
		"serverip=192.168.0.17\0"    \
		"ipaddr=192.168.0.15\0"    \
		"romboot=echo Booting Kernel from flash at 0x${uimage_flash_addr}; " \
		"echo Using bootargs: ${bootargs};bootm ${uimage_flash_addr}\0" \
		"earlycon=uart8250,mmio32,0xf0000000\0" \
		"console=ttyS0,115200n8\0" \
		"common_bootargs=setenv bootargs earlycon=${earlycon} root=/dev/ram " \
		"console=${console} ramdisk_size=48000\0" \
		"\0"

#endif
