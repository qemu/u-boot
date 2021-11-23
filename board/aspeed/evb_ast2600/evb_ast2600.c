// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Aspeed Technology Inc.
 */
#include <common.h>

int board_init(void)
{
	if (IS_ENABLED(CONFIG_LED))
		led_default_state();

	return 0;
}
