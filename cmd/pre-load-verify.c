// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Philippe Reynes <philippe.reynes@softathome.com>
 */

#include <common.h>
#include <env.h>
#include <image.h>
#include <mapmem.h>

static ulong verify_get_addr(int argc, char *const argv[])
{
	ulong addr;

	if (argc > 0)
		addr = hextoul(argv[0], NULL);
	else
		addr = image_load_addr;

	return addr;
}

static int do_verify(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	ulong addr = verify_get_addr(argc, argv);
	int ret = 0;

	argc--; argv++;

	addr = verify_get_addr(argc, argv);

	if (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD)) {
		ret = image_pre_load(addr);

		if (ret) {
			ret = CMD_RET_FAILURE;
			goto out;
		}

		env_set_hex("loadaddr_verified", addr + image_load_offset);
	}

 out:
	return ret;
}

U_BOOT_CMD(pre_load_verify, 2, 1, do_verify,
	   "verify the global signature provided in the pre-load header,\n",
	   "\tif the check succeed, the u-boot env variable loadaddr_verified\n"
	   "\tis set to the address of the image (without the header)"
	   "<image addr>"
);
