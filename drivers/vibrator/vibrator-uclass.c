// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Samuel Dionne-Riel <samuel@dionne-riel.com>
 * Copyright (c) 2015 Google, Inc
 * Largely derived from `drivers/led/led-uclass.c`
 * Original written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_VIBRATOR

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <vibrator.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/uclass-internal.h>

int vibrator_get_by_label(const char *label, struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_VIBRATOR, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		struct vibrator_uc_plat *uc_plat = dev_get_uclass_plat(dev);

		if (uc_plat->label && strcmp(label, uc_plat->label) == 0)
			return uclass_get_device_tail(dev, 0, devp);
	}

	return -ENODEV;
}

int vibrator_set_state(struct udevice *dev, enum vibrator_state_t state)
{
	struct vibrator_ops *ops = vibrator_get_ops(dev);

	if (!ops->set_state)
		return -ENOSYS;

	return ops->set_state(dev, state);
}

enum vibrator_state_t vibrator_get_state(struct udevice *dev)
{
	struct vibrator_ops *ops = vibrator_get_ops(dev);

	if (!ops->get_state)
		return -ENOSYS;

	return ops->get_state(dev);
}

UCLASS_DRIVER(vibrator) = {
	.id		= UCLASS_VIBRATOR,
	.name		= "vibrator",
	.per_device_plat_auto	= sizeof(struct vibrator_uc_plat),
};
