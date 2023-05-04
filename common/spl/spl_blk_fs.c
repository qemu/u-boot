// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 * Derived work from spl_sata.c
 */

#include <common.h>
#include <spl.h>

int spl_blk_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       enum uclass_id uclass_id, int devnum)
{
	int ret = -ENOSYS, part;
	struct blk_desc *blk_desc;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	if (!blk_desc)
		return ret;

	blk_show_device(uclass_id, devnum);

	if (IS_ENABLED(CONFIG_SPL_FS_EXT4)) {
		switch (uclass_id) {
		case UCLASS_NVME:
			part = CONFIG_SYS_NVME_EXT_BOOT_PARTITION;
			break;
		default:
			return ret;
		}
		ret = spl_load_image_ext(spl_image, bootdev, blk_desc, part,
					 CONFIG_SPL_PAYLOAD);
		if (!ret)
			return ret;
	}

	if (IS_ENABLED(CONFIG_SPL_FS_FAT)) {
		switch (uclass_id) {
		case UCLASS_NVME:
			part = CONFIG_SYS_NVME_FAT_BOOT_PARTITION;
			break;
		default:
			return ret;
		}
		ret = spl_load_image_fat(spl_image, bootdev, blk_desc, part,
					 CONFIG_SPL_PAYLOAD);
		if (!ret)
			return ret;
	}

	return ret;
}
