// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023
 * Ventana Micro Systems Inc.
 *
 */

#include <common.h>
#include <spl.h>
#include <spl_load.h>
#include <image.h>
#include <fs.h>
#include <part.h>
#include <asm/cache.h>
#include <asm/io.h>

struct blk_dev {
	const char *ifname;
	const char *filename;
	int devnum;
	int partnum;
	char dev_part_str[8];
};

static ulong spl_fit_read(struct spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	loff_t actlen;
	int ret;
	struct blk_dev *dev = (struct blk_dev *)load->priv;

	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	if (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		return ret;
	}

	ret = fs_read(dev->filename, virt_to_phys(buf), file_offset, size,
		      &actlen);
	if (ret < 0) {
		printf("spl: error reading image %s. Err - %d\n",
		       dev->filename, ret);
		return ret;
	}

	return actlen;
}

static int spl_blk_file_size(struct blk_dev *dev, loff_t *filesize)
{
	int ret;

	snprintf(dev->dev_part_str, sizeof(dev->dev_part_str) - 1, "%x:%x",
		 dev->devnum, dev->partnum);
	debug("Loading file %s from %s %s\n", dev->filename, dev->ifname,
	      dev->dev_part_str);
	ret = fs_set_blk_dev(dev->ifname, dev->dev_part_str, FS_TYPE_ANY);
	if (ret) {
		printf("spl: unable to set blk_dev %s %s. Err - %d\n",
		       dev->ifname, dev->dev_part_str, ret);
		return ret;
	}

	ret = fs_size(dev->filename, filesize);
	if (ret)
		printf("spl: unable to get size, file: %s. Err - %d\n",
		       dev->filename, ret);
	return ret;
}

int spl_blk_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       enum uclass_id uclass_id, int devnum, int partnum)
{
	struct blk_desc *blk_desc;
	loff_t filesize;
	struct blk_dev dev;
	struct spl_load_info load;
	int ret, part;

	blk_desc = blk_get_devnum_by_uclass_id(uclass_id, devnum);
	if (!blk_desc) {
		printf("blk desc for %d %d not found\n", uclass_id, devnum);
		return -ENODEV;
	}

	blk_show_device(uclass_id, devnum);

	dev.filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	dev.ifname = blk_get_uclass_name(uclass_id);
	dev.devnum = devnum;
	/*
	 * First try to boot from EFI System partition. In case of failure,
	 * fall back to the configured partition.
	 */
	if (IS_ENABLED(CONFIG_SPL_ESP_BOOT)) {
		part = part_get_esp(blk_desc);
		if (part) {
			dev.partnum = part;
			ret = spl_blk_file_size(&dev, &filesize);
			if (!ret)
				goto out;
		}
	}

	dev.partnum = partnum;
	ret = spl_blk_file_size(&dev, &filesize);
	if (ret)
		return ret;
out:
	load.read = spl_fit_read;
	if (IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN))
		spl_set_bl_len(&load, ARCH_DMA_MINALIGN);
	else
		spl_set_bl_len(&load, 1);
	load.priv = &dev;
	return spl_load(spl_image, bootdev, &load, filesize, 0);
}
