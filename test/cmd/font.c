// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for font command
 *
 * Copyright 2022 Google LLC
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <video_console.h>
#include <test/suites.h>
#include <test/ut.h>

/* Declare a new fdt test */
#define FONT_TEST(_name, _flags)	UNIT_TEST(_name, _flags, font_test)

/* Test 'fdt addr' resizing an fdt */
static int font_test_base(struct unit_test_state *uts)
{
	struct udevice *dev;
	int max_metrics;
	uint size;
	int ret;

	ut_assertok(uclass_first_device_err(UCLASS_VIDEO, &dev));
	ut_assertok(uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev));

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("font list", 0));
	ut_assert_nextline("nimbus_sans_l_regular");
	ut_assert_nextline("cantoraone_regular");
	ut_assertok(ut_check_console_end(uts));

	ut_asserteq_str("nimbus_sans_l_regular",
			vidconsole_get_font_size(dev, &size));
	ut_asserteq(18, size);

	max_metrics = 1;
	if (IS_ENABLED(CONFIG_CONSOLE_TRUETYPE))
		max_metrics = IF_ENABLED_INT(CONFIG_CONSOLE_TRUETYPE,
				     CONFIG_CONSOLE_TRUETYPE_MAX_METRICS);

	ret = run_command("font select cantoraone_regular 40", 0);
	if (max_metrics < 2) {
		ut_asserteq(1, ret);
		ut_assert_nextline("Failed (error -7)");
		ut_assertok(ut_check_console_end(uts));
		return 0;
	}

	ut_assertok(ret);
	ut_assertok(ut_check_console_end(uts));

	ut_asserteq_str("cantoraone_regular",
			vidconsole_get_font_size(dev, &size));
	ut_asserteq(40, size);

	ut_assertok(run_command("font size 30", 0));
	ut_assertok(ut_check_console_end(uts));

	ut_asserteq_str("cantoraone_regular",
			vidconsole_get_font_size(dev, &size));
	ut_asserteq(30, size);

	return 0;
}
FONT_TEST(font_test_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT |
	  UT_TESTF_CONSOLE_REC | UT_TESTF_DM);

int do_ut_font(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(font_Test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(font_test);

	return cmd_ut_category("font", "font_test_", tests, n_ents, argc, argv);
}
