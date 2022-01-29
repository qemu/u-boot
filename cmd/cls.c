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
#include <lcd.h>
#include <video.h>

#define ESC "\x1b"

static int do_video_clear(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	/*  Send clear screen and home */
	printf(ESC "[2J" ESC "[1;1H");
#if !defined(CONFIG_DM_VIDEO) && !defined(CONFIG_CFB_CONSOLE_ANSI)
	if (CONFIG_IS_ENABLED(CFB_CONSOLE))
		video_clear();
	else if (CONFIG_IS_ENABLED(LCD)
		lcd_clear();
#endif
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(cls,	1, 1, do_video_clear, "clear screen", "");
