// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * DENX Software Engineering, Anatolij Gustschin <agust@denx.de>
 *
 * cls - clear screen command
 */
#include <common.h>
#include <command.h>
#include <dm.h>
#include <video.h>

#define CSI "\x1b["

static int do_video_clear(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	__maybe_unused struct udevice *dev;

	/*  Send clear screen and home */
	printf(CSI "2J" CSI "1;1H");
	if (CONFIG_IS_ENABLED(DM_VIDEO) && !CONFIG_IS_ENABLED(VIDEO_ANSI)) {
		if (uclass_first_device_err(UCLASS_VIDEO, &dev))
			return CMD_RET_FAILURE;
		if (video_clear(dev))
			return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(cls,	1, 1, do_video_clear, "clear screen", "");
