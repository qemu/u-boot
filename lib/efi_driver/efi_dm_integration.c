// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#define LOG_CATEGORY LOGC_EFI

#include <common.h>
#include <dm.h>
#include <efi_loader.h>
#include <log.h>

/**
 * efi_post_probe_device() - set up handle for probed device
 *
 * This function is called by device_probe(). After the UEFI sub-system is
 * initialized this function adds handles for new devices.
 *
 * @dev:	probed device
 * Return:	0 on success
 */
int efi_post_probe_device(struct udevice *dev)
{
	if (!dev || !dev->uclass)
		return -EINVAL;

	switch (dev->uclass->uc_drv->id) {
	case UCLASS_BLK:
		if (efi_block_device_register(dev) != EFI_SUCCESS)
			log_err("Failed to register %s\n", dev->name);
	default:
		break;
	}

	return 0;
}
