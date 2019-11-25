// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2014, NVIDIA CORPORATION.  All rights reserved.
 */

#include <common.h>
#include <command.h>

#include "pxe_utils.h"

#ifdef CONFIG_CMD_NET
const char *pxe_default_paths[] = {
#ifdef CONFIG_SYS_SOC
#ifdef CONFIG_SYS_BOARD
	"default-" CONFIG_SYS_ARCH "-" CONFIG_SYS_SOC "-" CONFIG_SYS_BOARD,
#endif
	"default-" CONFIG_SYS_ARCH "-" CONFIG_SYS_SOC,
#endif
	"default-" CONFIG_SYS_ARCH,
	"default",
	NULL
};

static int do_get_tftp(cmd_tbl_t *cmdtp, const char *file_path, char *file_addr)
{
	char *tftp_argv[] = {"tftp", NULL, NULL, NULL};

	tftp_argv[1] = file_addr;
	tftp_argv[2] = (void *)file_path;

	if (do_tftpb(cmdtp, 0, 3, tftp_argv))
		return -ENOENT;

	return 1;
}

/*
 * Entry point for the 'pxe get' command.
 * This Follows pxelinux's rules to download a config file from a tftp server.
 * The file is stored at the location given by the pxefile_addr_r environment
 * variable, which must be set.
 *
 * UUID comes from pxeuuid env variable, if defined
 * MAC addr comes from ethaddr env variable, if defined
 * IP
 *
 * see http://syslinux.zytor.com/wiki/index.php/PXELINUX
 *
 * Returns 0 on success or 1 on error.
 */
static int
do_pxe_get(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *pxefile_addr_str;
	unsigned long pxefile_addr_r;
	int err, i = 0;

	do_getfile = do_get_tftp;

	if (argc != 1)
		return CMD_RET_USAGE;

	pxefile_addr_str = from_env("pxefile_addr_r");

	if (!pxefile_addr_str)
		return 1;

	err = strict_strtoul(pxefile_addr_str, 16,
			     (unsigned long *)&pxefile_addr_r);
	if (err < 0)
		return 1;

	/*
	 * Keep trying paths until we successfully get a file we're looking
	 * for.
	 */
	if (pxe_uuid_path(cmdtp, pxefile_addr_r) > 0 ||
	    pxe_mac_path(cmdtp, pxefile_addr_r) > 0 ||
	    pxe_ipaddr_paths(cmdtp, pxefile_addr_r) > 0) {
		printf("Config file found\n");

		return 0;
	}

	while (pxe_default_paths[i]) {
		if (get_pxelinux_path(cmdtp, pxe_default_paths[i],
				      pxefile_addr_r) > 0) {
			printf("Config file found\n");
			return 0;
		}
		i++;
	}

	printf("Config file not found\n");

	return 1;
}

/*
 * Boots a system using a pxe file
 *
 * Returns 0 on success, 1 on error.
 */
static int
do_pxe_boot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long pxefile_addr_r;
	struct pxe_menu *cfg;
	char *pxefile_addr_str;

	do_getfile = do_get_tftp;

	if (argc == 1) {
		pxefile_addr_str = from_env("pxefile_addr_r");
		if (!pxefile_addr_str)
			return 1;

	} else if (argc == 2) {
		pxefile_addr_str = argv[1];
	} else {
		return CMD_RET_USAGE;
	}

	if (strict_strtoul(pxefile_addr_str, 16, &pxefile_addr_r) < 0) {
		printf("Invalid pxefile address: %s\n", pxefile_addr_str);
		return 1;
	}

	cfg = parse_pxefile(cmdtp, pxefile_addr_r);

	if (!cfg) {
		printf("Error parsing config file\n");
		return 1;
	}

	handle_pxe_menu(cmdtp, cfg);

	destroy_pxe_menu(cfg);

	copy_filename(net_boot_file_name, "", sizeof(net_boot_file_name));

	return 0;
}

static cmd_tbl_t cmd_pxe_sub[] = {
	U_BOOT_CMD_MKENT(get, 1, 1, do_pxe_get, "", ""),
	U_BOOT_CMD_MKENT(boot, 2, 1, do_pxe_boot, "", "")
};

static int do_pxe(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	is_pxe = true;

	/* drop initial "pxe" arg */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], cmd_pxe_sub, ARRAY_SIZE(cmd_pxe_sub));

	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

U_BOOT_CMD(pxe, 3, 1, do_pxe,
	   "commands to get and boot from pxe files",
	   "get - try to retrieve a pxe file using tftp\n"
	   "pxe boot [pxefile_addr_r] - boot from the pxe file at pxefile_addr_r\n"
);
#endif
