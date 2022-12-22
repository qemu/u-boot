/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010-2017 CS Systemes d'Information
 * Christophe Leroy <christophe.leroy@c-s.fr>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"sdram_type=SDRAM\0"						\
	"flash_type=AM29LV160DB\0"					\
	"loadaddr=0x400000\0"						\
	"filename=uImage.lzma\0"					\
	"nfsroot=/opt/ofs\0"						\
	"dhcp_ip=ip=:::::eth0:dhcp\0"					\
	"console_args=console=ttyCPM0,115200N8\0"			\
	"flashboot=setenv bootargs "					\
		"${console_args} "					\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"	\
		"mcr3k:eth0:off;"					\
		"${ofl_args}; "						\
		"bootm 0x04060000 - 0x04050000\0"			\
	"tftpboot=setenv bootargs "					\
		"${console_args} "					\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"	\
		"mcr3k:eth0:off "					\
		"${ofl_args}; "						\
		"tftp ${loadaddr} ${filename};"				\
		"tftp 0xf00000 mcr3000.dtb;"				\
		"bootm ${loadaddr} - 0xf00000\0"			\
	"netboot=dhcp ${loadaddr} ${filename};"				\
		"tftp 0xf00000 mcr3000.dtb;"				\
		"setenv bootargs "					\
		"root=/dev/nfs rw "					\
		"${console_args} "					\
		"${dhcp_ip};"						\
		"bootm ${loadaddr} - 0xf00000\0"			\
	"nfsboot=setenv bootargs "					\
		"root=/dev/nfs rw nfsroot=${serverip}:${nfsroot} "	\
		"${console_args} "					\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"	\
		"mcr3k:eth0:off;"					\
		"bootm 0x04060000 - 0x04050000\0"			\
	"dhcpboot=dhcp ${loadaddr} ${filename};"			\
		"tftp 0xf00000 mcr3000.dtb;"				\
		"setenv bootargs "					\
		"${console_args} "					\
		"${dhcp_ip} "						\
		"${ofl_args}; "						\
		"bootm ${loadaddr} - 0xf00000\0"

/* Miscellaneous configurable options */

/* Definitions for initial stack pointer and data area (in DPRAM) */
#define CFG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_IMMR + 0x2800)
#define	CFG_SYS_INIT_RAM_SIZE	(0x2e00 - 0x2800)

/* RAM configuration (note that CFG_SYS_SDRAM_BASE must be zero) */
#define	CFG_SYS_SDRAM_BASE		0x00000000

/* FLASH organization */
#define CFG_SYS_FLASH_BASE		CONFIG_TEXT_BASE

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CFG_SYS_BOOTMAPSZ		(8 << 20)

/* Environment Configuration */

/* environment is in FLASH */

/* Ethernet configuration part */

/* NAND configuration part */
#define CFG_SYS_NAND_BASE		0x0C000000

#endif /* __CONFIG_H */
