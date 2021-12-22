// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Samuel Dionne-Riel <samuel@dionne-riel.com>
 * Copyright (c) 2015 Google, Inc
 * Largely derived from `drivers/led/led_gpio.c`
 * Original written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <vibrator.h>
#include <log.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <dm/lists.h>

struct vibrator_gpio_priv {
	struct gpio_desc gpio;
};

static int gpio_vibrator_set_state(struct udevice *dev, enum vibrator_state_t state)
{
	struct vibrator_gpio_priv *priv = dev_get_priv(dev);
	int ret;

	if (!dm_gpio_is_valid(&priv->gpio))
		return -EREMOTEIO;
	switch (state) {
	case VIBRATOR_STATE_OFF:
	case VIBRATOR_STATE_ON:
		break;
	case VIBRATOR_STATE_TOGGLE:
		ret = dm_gpio_get_value(&priv->gpio);
		if (ret < 0)
			return ret;
		state = !ret;
		break;
	default:
		return -ENOSYS;
	}

	return dm_gpio_set_value(&priv->gpio, state);
}

static enum vibrator_state_t gpio_vibrator_get_state(struct udevice *dev)
{
	struct vibrator_gpio_priv *priv = dev_get_priv(dev);
	int ret;

	if (!dm_gpio_is_valid(&priv->gpio))
		return -EREMOTEIO;
	ret = dm_gpio_get_value(&priv->gpio);
	if (ret < 0)
		return ret;

	return ret ? VIBRATOR_STATE_ON : VIBRATOR_STATE_OFF;
}

static int vibrator_gpio_probe(struct udevice *dev)
{
	struct vibrator_gpio_priv *priv = dev_get_priv(dev);
	int ret;

	ret = gpio_request_by_name(dev, "enable-gpios", 0, &priv->gpio, GPIOD_IS_OUT);
	if (ret)
		return ret;

	return 0;
}

static int vibrator_gpio_remove(struct udevice *dev)
{
	/*
	 * The GPIO driver may have already been removed. We will need to
	 * address this more generally.
	 */
	if (!IS_ENABLED(CONFIG_SANDBOX)) {
		struct vibrator_gpio_priv *priv = dev_get_priv(dev);

		if (dm_gpio_is_valid(&priv->gpio))
			dm_gpio_free(dev, &priv->gpio);
	}

	return 0;
}

static int vibrator_gpio_bind(struct udevice *dev)
{
	ofnode node;
	struct vibrator_uc_plat *uc_plat;
	const char *label;

	node = dev_ofnode(dev);
	label = ofnode_get_name(node);

	uc_plat = dev_get_uclass_plat(dev);
	uc_plat->label = label;

	return 0;
}

static const struct vibrator_ops gpio_vibrator_ops = {
	.set_state	= gpio_vibrator_set_state,
	.get_state	= gpio_vibrator_get_state,
};

static const struct udevice_id vibrator_gpio_ids[] = {
	{ .compatible = "gpio-vibrator" },
	{ }
};

U_BOOT_DRIVER(vibrator_gpio) = {
	.name	= "gpio_vibrator",
	.id	= UCLASS_VIBRATOR,
	.of_match = vibrator_gpio_ids,
	.ops	= &gpio_vibrator_ops,
	.priv_auto	= sizeof(struct vibrator_gpio_priv),
	.bind	= vibrator_gpio_bind,
	.probe	= vibrator_gpio_probe,
	.remove	= vibrator_gpio_remove,
};
