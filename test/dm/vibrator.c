// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Samuel Dionne-Riel <samuel@dionne-riel.com>
 */

#include <common.h>
#include <dm.h>
#include <vibrator.h>
#include <asm/gpio.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Base test of the vibrator uclass */
static int dm_test_vibrator_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_VIBRATOR, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_VIBRATOR, 1, &dev));
	ut_asserteq(-ENODEV, uclass_get_device(UCLASS_VIBRATOR, 2, &dev));

	return 0;
}
DM_TEST(dm_test_vibrator_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test of the vibrator uclass using the vibrator_gpio driver */
static int dm_test_vibrator_gpio(struct unit_test_state *uts)
{
	const int offset = 1;
	struct udevice *dev, *gpio;

	/*
	 * Check that we can manipulate a vibration motor. Vibrator 1 is connected to GPIO
	 * bank gpio_a, offset 1.
	 */
	ut_assertok(uclass_get_device(UCLASS_VIBRATOR, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	ut_assertok(vibrator_set_state(dev, VIBRATOR_STATE_ON));
	ut_asserteq(1, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(VIBRATOR_STATE_ON, vibrator_get_state(dev));

	ut_assertok(vibrator_set_state(dev, VIBRATOR_STATE_OFF));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(VIBRATOR_STATE_OFF, vibrator_get_state(dev));

	return 0;
}
DM_TEST(dm_test_vibrator_gpio, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that we can toggle vibration motors */
static int dm_test_vibrator_toggle(struct unit_test_state *uts)
{
	const int offset = 1;
	struct udevice *dev, *gpio;

	/*
	 * Check that we can manipulate a vibration motor. Vibrator 1 is connected to GPIO
	 * bank gpio_a, offset 1.
	 */
	ut_assertok(uclass_get_device(UCLASS_VIBRATOR, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	ut_assertok(vibrator_set_state(dev, VIBRATOR_STATE_TOGGLE));
	ut_asserteq(1, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(VIBRATOR_STATE_ON, vibrator_get_state(dev));

	ut_assertok(vibrator_set_state(dev, VIBRATOR_STATE_TOGGLE));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(VIBRATOR_STATE_OFF, vibrator_get_state(dev));

	return 0;
}
DM_TEST(dm_test_vibrator_toggle, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test obtaining a vibration motor by label */
static int dm_test_vibrator_label(struct unit_test_state *uts)
{
	struct udevice *dev, *cmp;

	ut_assertok(vibrator_get_by_label("vibrator_left", &dev));
	ut_asserteq(1, device_active(dev));
	ut_assertok(uclass_get_device(UCLASS_VIBRATOR, 0, &cmp));
	ut_asserteq_ptr(dev, cmp);

	ut_assertok(vibrator_get_by_label("vibrator_right", &dev));
	ut_asserteq(1, device_active(dev));
	ut_assertok(uclass_get_device(UCLASS_VIBRATOR, 1, &cmp));
	ut_asserteq_ptr(dev, cmp);

	ut_asserteq(-ENODEV, vibrator_get_by_label("doesnotexist", &dev));

	return 0;
}
DM_TEST(dm_test_vibrator_label, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

