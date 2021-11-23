// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 */
#include <common.h>

int board_init(void)
{
	if (IS_ENABLED(CONFIG_LED))
		led_default_state();

	return 0;
}
