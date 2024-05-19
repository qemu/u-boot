// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 PHYTEC Messtechnik GmbH
 * Author: Teresa Remmet <t.remmet@phytec.de>
 */

#include <common.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <i2c.h>
#include <u-boot/crc.h>
#include <malloc.h>
#include <extension_board.h>

#include "phytec_som_detection.h"

struct phytec_eeprom_data eeprom_data;

#if IS_ENABLED(CONFIG_PHYTEC_SOM_DETECTION)

int phytec_eeprom_data_setup_fallback(struct phytec_eeprom_data *data,
				      int bus_num, int addr, int addr_fallback)
{
	int ret;

	ret = phytec_eeprom_data_init(data, bus_num, addr);
	if (ret) {
		pr_err("%s: init failed. Trying fall back address 0x%x\n",
		       __func__, addr_fallback);
		ret = phytec_eeprom_data_init(data, bus_num, addr_fallback);
	}

	if (ret)
		pr_err("%s: EEPROM data init failed\n", __func__);

	return ret;
}

int phytec_eeprom_data_setup(struct phytec_eeprom_data *data,
			     int bus_num, int addr)
{
	int ret;

	ret = phytec_eeprom_data_init(data, bus_num, addr);
	if (ret)
		pr_err("%s: EEPROM data init failed\n", __func__);

	return ret;
}

int phytec_eeprom_data_init(struct phytec_eeprom_data *data,
			    int bus_num, int addr)
{
	int ret, i;
	unsigned int crc;
	u8 *ptr;
	const unsigned int payload_size = sizeof(struct phytec_eeprom_payload);

	if (!data)
		data = &eeprom_data;

#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(bus_num, addr, 2, &dev);
	if (ret) {
		pr_err("%s: i2c EEPROM not found: %i.\n", __func__, ret);
		goto err;
	}

	ret = dm_i2c_read(dev, 0, (uint8_t *)data, payload_size);
	if (ret) {
		pr_err("%s: Unable to read EEPROM data: %i\n", __func__, ret);
		goto err;
	}
#else
	i2c_set_bus_num(bus_num);
	ret = i2c_read(addr, 0, 2, (uint8_t *)data,
		       sizeof(struct phytec_eeprom_data));
#endif

	if (data->payload.api_rev == 0xff) {
		pr_err("%s: EEPROM is not flashed. Prototype?\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	ptr = (u8 *)data;
	for (i = 0; i < payload_size; ++i)
		if (ptr[i] != 0x0)
			break;

	if (i == payload_size) {
		pr_err("%s: EEPROM data is all zero. Erased?\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	/* We are done here for early revisions */
	if (data->payload.api_rev <= PHYTEC_API_REV1) {
		data->valid = true;
		return 0;
	}

	crc = crc8(0, (const unsigned char *)&data->payload, payload_size);
	debug("%s: crc: %x\n", __func__, crc);

	if (crc) {
		pr_err("%s: CRC mismatch. EEPROM data is not usable.\n",
		       __func__);
		ret = -EINVAL;
		goto err;
	}

	data->valid = true;
	return 0;
err:
	data->valid = false;
	return ret;
}

void __maybe_unused phytec_print_som_info(struct phytec_eeprom_data *data)
{
	struct phytec_api2_data *api2;
	char pcb_sub_rev;
	unsigned int ksp_no, sub_som_type1, sub_som_type2;

	if (!data)
		data = &eeprom_data;

	if (!data->valid || data->payload.api_rev < PHYTEC_API_REV2)
		return;

	api2 = &data->payload.data.data_api2;

	/* Calculate PCB subrevision */
	pcb_sub_rev = api2->pcb_sub_opt_rev & 0x0f;
	pcb_sub_rev = pcb_sub_rev ? ((pcb_sub_rev - 1) + 'a') : ' ';

	/* print standard product string */
	if (api2->som_type <= 1) {
		printf("SoM: %s-%03u-%s.%s PCB rev: %u%c\n",
		       phytec_som_type_str[api2->som_type], api2->som_no,
		       api2->opt, api2->bom_rev, api2->pcb_rev, pcb_sub_rev);
		return;
	}
	/* print KSP/KSM string */
	if (api2->som_type <= 3) {
		ksp_no = (api2->ksp_no << 8) | api2->som_no;
		printf("SoM: %s-%u ",
		       phytec_som_type_str[api2->som_type], ksp_no);
	/* print standard product based KSP/KSM strings */
	} else {
		switch (api2->som_type) {
		case 4:
			sub_som_type1 = 0;
			sub_som_type2 = 3;
			break;
		case 5:
			sub_som_type1 = 0;
			sub_som_type2 = 2;
			break;
		case 6:
			sub_som_type1 = 1;
			sub_som_type2 = 3;
			break;
		case 7:
			sub_som_type1 = 1;
			sub_som_type2 = 2;
			break;
		default:
			pr_err("%s: Invalid SoM type: %i", __func__, api2->som_type);
			return;
		};

		printf("SoM: %s-%03u-%s-%03u ",
		       phytec_som_type_str[sub_som_type1],
		       api2->som_no, phytec_som_type_str[sub_som_type2],
		       api2->ksp_no);
	}

	printf("Option: %s BOM rev: %s PCB rev: %u%c\n", api2->opt,
	       api2->bom_rev, api2->pcb_rev, pcb_sub_rev);
}

char * __maybe_unused phytec_get_opt(struct phytec_eeprom_data *data)
{
	char *opt;

	if (!data)
		data = &eeprom_data;

	if (!data->valid)
		return NULL;

	if (data->payload.api_rev < PHYTEC_API_REV2)
		opt = data->payload.data.data_api0.opt;
	else
		opt = data->payload.data.data_api2.opt;

	return opt;
}

u8 __maybe_unused phytec_get_rev(struct phytec_eeprom_data *data)
{
	struct phytec_api2_data *api2;

	if (!data)
		data = &eeprom_data;

	if (!data->valid || data->payload.api_rev < PHYTEC_API_REV2)
		return PHYTEC_EEPROM_INVAL;

	api2 = &data->payload.data.data_api2;

	return api2->pcb_rev;
}

u8 __maybe_unused phytec_get_som_type(struct phytec_eeprom_data *data)
{
	if (!data)
		data = &eeprom_data;

	if (!data->valid || data->payload.api_rev < PHYTEC_API_REV2)
		return PHYTEC_EEPROM_INVAL;

	return data->payload.data.data_api2.som_type;
}

#if IS_ENABLED(CONFIG_CMD_EXTENSION)
struct extension *phytec_add_extension(const char *name, const char *overlay,
				       const char *other)
{
	struct extension *extension;

	if (strlen(overlay) > sizeof(extension->overlay)) {
		pr_err("Overlay name %s is longer than %lu.\n", overlay,
		       sizeof(extension->overlay));
		return NULL;
	}

	extension = calloc(1, sizeof(struct extension));
	snprintf(extension->name, sizeof(extension->name), name);
	snprintf(extension->overlay, sizeof(extension->overlay), overlay);
	snprintf(extension->other, sizeof(extension->other), other);
	snprintf(extension->owner, sizeof(extension->owner), "PHYTEC");

	return extension;
}
#endif /* IS_ENABLED(CONFIG_CMD_EXTENSION) */

#else

inline int phytec_eeprom_data_setup(struct phytec_eeprom_data *data,
				    int bus_num, int addr)
{
	return PHYTEC_EEPROM_INVAL;
}

inline int phytec_eeprom_data_setup_fallback(struct phytec_eeprom_data *data,
					     int bus_num, int addr,
					     int addr_fallback)
{
	return PHYTEC_EEPROM_INVAL;
}

inline int phytec_eeprom_data_init(struct phytec_eeprom_data *data,
				   int bus_num, int addr)
{
	return PHYTEC_EEPROM_INVAL;
}

inline void __maybe_unused phytec_print_som_info(struct phytec_eeprom_data *data)
{
}

inline char *__maybe_unused phytec_get_opt(struct phytec_eeprom_data *data)
{
	return NULL;
}

u8 __maybe_unused phytec_get_rev(struct phytec_eeprom_data *data)
{
	return PHYTEC_EEPROM_INVAL;
}

u8 __maybe_unused phytec_get_som_type(struct phytec_eeprom_data *data)
{
	return PHYTEC_EEPROM_INVAL;
}

#if IS_ENABLED(CONFIG_CMD_EXTENSION)
inline struct extension *phytec_add_extension(const char *name,
					      const char *overlay,
					      const char *other)
{
	return NULL;
}
#endif /* IS_ENABLED(CONFIG_CMD_EXTENSION) */

#endif /* IS_ENABLED(CONFIG_PHYTEC_SOM_DETECTION) */
