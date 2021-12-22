/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2021 Samuel Dionne-Riel <samuel@dionne-riel.com>
 * Copyright (c) 2015 Google, Inc
 * Largely derived from `include/led.h`
 * Original written by Simon Glass <sjg@chromium.org>
 */

#ifndef __VIBRATOR_H
#define __VIBRATOR_H

struct udevice;

/**
 * struct vibrator_uc_plat - Platform data the uclass stores about each device
 *
 * @label:	VIBRATOR label
 */
struct vibrator_uc_plat {
	const char *label;
};

/**
 * struct vibrator_uc_priv - Private data the uclass stores about each device
 *
 * @period_ms:	Flash period in milliseconds
 */
struct vibrator_uc_priv {
	int period_ms;
};

enum vibrator_state_t {
	VIBRATOR_STATE_OFF = 0,
	VIBRATOR_STATE_ON = 1,
	VIBRATOR_STATE_TOGGLE,

	VIBRATOR_STATE_COUNT,
};

struct vibrator_ops {
	/**
	 * set_state() - set the state of an VIBRATOR
	 *
	 * @dev:	VIBRATOR device to change
	 * @state:	VIBRATOR state to set
	 * @return 0 if OK, -ve on error
	 */
	int (*set_state)(struct udevice *dev, enum vibrator_state_t state);

	/**
	 * vibrator_get_state() - get the state of an VIBRATOR
	 *
	 * @dev:	VIBRATOR device to change
	 * @return VIBRATOR state vibrator_state_t, or -ve on error
	 */
	enum vibrator_state_t (*get_state)(struct udevice *dev);
};

#define vibrator_get_ops(dev)	((struct vibrator_ops *)(dev)->driver->ops)

/**
 * vibrator_get_by_label() - Find an VIBRATOR device by label
 *
 * @label:	VIBRATOR label to look up
 * @devp:	Returns the associated device, if found
 * @return 0 if found, -ENODEV if not found, other -ve on error
 */
int vibrator_get_by_label(const char *label, struct udevice **devp);

/**
 * vibrator_set_state() - set the state of an VIBRATOR
 *
 * @dev:	VIBRATOR device to change
 * @state:	VIBRATOR state to set
 * @return 0 if OK, -ve on error
 */
int vibrator_set_state(struct udevice *dev, enum vibrator_state_t state);

/**
 * vibrator_get_state() - get the state of an VIBRATOR
 *
 * @dev:	VIBRATOR device to change
 * @return VIBRATOR state vibrator_state_t, or -ve on error
 */
enum vibrator_state_t vibrator_get_state(struct udevice *dev);

#endif
