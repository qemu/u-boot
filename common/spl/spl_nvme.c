// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 * Derived work from spl_sata.c
 */

#include <common.h>
#include <spl.h>
#include <errno.h>
#include <fat.h>
#include <init.h>
#include <nvme.h>

static int spl_nvme_load_image(struct spl_image_info *spl_image,
			       struct spl_boot_device *bootdev)
{
	int ret;

	ret = pci_init();
	if (ret < 0)
		return ret;

	ret = nvme_scan_namespace();
	if (ret < 0)
		return ret;

	ret = spl_blk_load_image(spl_image, bootdev, UCLASS_NVME,
				 CONFIG_SPL_NVME_BOOT_DEVICE);
	return ret;
}

SPL_LOAD_IMAGE_METHOD("NVME", 0, BOOT_DEVICE_NVME, spl_nvme_load_image);
