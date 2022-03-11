// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 * Copyright 2021 Purism
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <asm/io.h>
#include <miiphy.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <asm/arch/sys_proto.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/video.h>
#include <fuse.h>
#include <i2c.h>
#include <spl.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <linux/delay.h>
#include <linux/bitfield.h>
#include <power/regulator.h>
#include <usb/xhci.h>
#include "librem5.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}

int board_postclk_init(void)
{
	return 0;
}

uint board_mmc_get_env_part(struct mmc *mmc)
{
	uint part = (mmc->part_config >> 3) & PART_ACCESS_MASK;

	if (part == 7)
		part = 0;
	return part;
}

int bq_read_reg(int reg_index)
{
	struct udevice *udev, *bus;
	u8 reg;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C, 3, &bus);
	if (ret) {
		log_err("%s: No bus %d\n", __func__, 3);
		return -ENODEV;
	}

	ret = i2c_get_chip(bus, 0x6a, 1, &udev);
	if (ret) {
		log_err("%s: setting chip offset failed %d\n", __func__, ret);
		return -EINVAL;
	}

	dm_i2c_read(udev, reg_index, &reg, 1);

	return reg;
}

int bq_write_reg(int reg_index, u8 val)
{
	struct udevice *udev, *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C, 3, &bus);
	if (ret) {
		log_err("%s: No bus %d\n", __func__, 3);
		return 1;
	}

	ret = i2c_get_chip(bus, 0x6a, 1, &udev);
	if (ret) {
		log_err("%s: setting chip offset failed %d\n", __func__, ret);
		return 1;
	}

	dm_i2c_write(udev, reg_index, &val, 1);

	return 0;
}

int read_vbat(void)
{
	u8 reg02, reg03, reg0e;
	int timeout = 1000; // ms

	/* disable the charger and enable BAT_LOADEN to discharge the decoupling
	 * CAP
	 * https://e2e.ti.com/support/power-management/f/196/t/567401
	 */
	reg03 = bq_read_reg(0x03);
	bq_write_reg(0x03, (reg03 | 0x80) & ~0x10);

	mdelay(10);

	/* force a conversion */
	reg02 = bq_read_reg(0x02);
	bq_write_reg(0x02, (reg02 | 0x80) & ~0x40);

	do {
		mdelay(10);
		timeout -= 10;
	} while ((bq_read_reg(0x02) & 0x80) && (timeout > 0));

	reg0e = bq_read_reg(0x0e);

	/* return the charger to it's original state */
	bq_write_reg(0x03, reg03);
	bq_write_reg(0x02, reg02);

	return (reg0e & 0x7f) * 20 + 2304;
}

int tps65982_wait_for_app(int timeout, int timeout_step)
{
	int ret;
	char response[6];
	struct udevice *udev, *bus;

	log_debug("%s: starting\n", __func__);

	/* Set the i2c bus */
	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &bus);
	if (ret) {
		log_err("%s: No bus %d\n", __func__, 0);
		return 1;
	}

	ret = i2c_get_chip(bus, 0x3f, 1, &udev);
	if (ret) {
		log_err("%s: setting chip offset failed %d\n", __func__, ret);
		return 1;
	}

	while (timeout > 0) {
		ret = dm_i2c_read(udev, 0x03, (u8 *)response, 5);
		log_debug("tps65982 mode %s\n", response);
		if (response[1] == 'A')
			return 0;
		mdelay(timeout_step);
		timeout -= timeout_step;
		log_debug("tps65982 waited %d ms %c\n", timeout_step, response[1]);
	}

	return 1;
}

#define TPS_POWER_STATUS_PWROPMODE(x)	    FIELD_GET(GENMASK(3, 2), x)

#define TPS_PDO_CONTRACT_TYPE(x)	FIELD_GET(GENMASK(31, 30), x)
#define TPS_PDO_CONTRACT_FIXED	0
#define TPS_PDO_CONTRACT_BATTERY	1
#define TPS_PDO_CONTRACT_VARIABLE	2

#define TPS_TYPEC_PWR_MODE_USB	0
#define TPS_TYPEC_PWR_MODE_1_5A	1
#define TPS_TYPEC_PWR_MODE_3_0A	2
#define TPS_TYPEC_PWR_MODE_PD	3

#define TPS_PDO_FIXED_CONTRACT_MAX_CURRENT(x)	(FIELD_GET(GENMASK(9, 0), x) * 10)
#define TPS_PDO_VAR_CONTRACT_MAX_CURRENT(x)	(FIELD_GET(GENMASK(9, 0), x) * 10)
#define TPS_PDO_BAT_CONTRACT_MAX_VOLTAGE(x)	(FIELD_GET(GENMASK(29, 20), x) * 50)
#define TPS_PDO_BAT_CONTRACT_MAX_POWER(x)	(FIELD_GET(GENMASK(9, 0), x) * 250)

int tps65982_get_max_current(void)
{
	int ret;
	u8 buf[7];
	u8 pwr_status;
	u32 contract;
	int type, mode;
	struct udevice *udev, *bus;

	log_debug("%s: starting\n", __func__);

	/* Set the i2c bus */
	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &bus);
	if (ret) {
		log_debug("%s: No bus %d\n", __func__, 0);
		return -1;
	}

	ret = i2c_get_chip(bus, 0x3f, 1, &udev);
	if (ret) {
		log_debug("%s: setting chip offset failed %d\n", __func__, ret);
		return -1;
	}

	ret = dm_i2c_read(udev, 0x3f, buf, 3);
	if (ret) {
		log_debug("%s: reading pwr_status failed %d\n", __func__, ret);
		return -1;
	}

	pwr_status = buf[1];

	if (!(pwr_status & 1))
		return 0;

	mode = TPS_POWER_STATUS_PWROPMODE(pwr_status);
	switch (mode) {
	case TPS_TYPEC_PWR_MODE_1_5A:
		return 1500;
	case TPS_TYPEC_PWR_MODE_3_0A:
		return 3000;
	case TPS_TYPEC_PWR_MODE_PD:
		ret = dm_i2c_read(udev, 0x34, buf, 7);
		if (ret) {
			log_debug("%s: reading active contract failed %d\n", __func__, ret);
			return -1;
		}

		contract = buf[1] + (buf[2] << 8) + (buf[3] << 16) + (buf[4] << 24);

		type = TPS_PDO_CONTRACT_TYPE(contract);

		switch (type) {
		case TPS_PDO_CONTRACT_FIXED:
			return TPS_PDO_FIXED_CONTRACT_MAX_CURRENT(contract);
		case TPS_PDO_CONTRACT_BATTERY:
			return 1000 * TPS_PDO_BAT_CONTRACT_MAX_POWER(contract)
				/ TPS_PDO_BAT_CONTRACT_MAX_VOLTAGE(contract);
		case TPS_PDO_CONTRACT_VARIABLE:
			return TPS_PDO_VAR_CONTRACT_MAX_CURRENT(contract);
		default:
			log_debug("Unknown contract type: %d\n", type);
			return -1;
		}
	case TPS_TYPEC_PWR_MODE_USB:
		return 500;
	default:
		log_debug("Unknown power mode: %d\n", mode);
		return -1;
	}
}

int init_tps65982(void)
{
	int vbat;

	log_debug("%s: starting\n", __func__);

	if (tps65982_wait_for_app(500, 100)) {
		/* check that VBAT is present and greater than 3000 mV. resetting the
		 * tps65982 disconnects VBUS so we need a different power source.
		 * Battery voltage greater than 4.15V likely means the battery is not
		 * present.
		 */
		vbat = read_vbat();
		log_err("tps65982 APP boot failed %d mV\n", vbat);
		return 1;
	}

	log_info("tps65982 boot successful\n");
	return 0;
}

int bq25896_set_iinlim(int current)
{
	u8 val, iinlim;
	int ret;
	struct udevice *udev, *bus;

	/* Set the i2c bus */
	ret = uclass_get_device_by_seq(UCLASS_I2C, 3, &bus);
	if (ret) {
		log_err("%s: No bus 3\n", __func__);
		return ret;
	}

	ret = i2c_get_chip(bus, 0x6a, 1, &udev);
	if (ret) {
		log_err("%s: setting chip offset failed %d\n", __func__, ret);
		return ret;
	}

	if (current > 3250)
		current = 3250;
	if (current < 100)
		current = 100;

	val = dm_i2c_reg_read(udev, 0x00);
	iinlim = ((current - 100) / 50) & 0x3f;
	val = (val & 0xc0) | iinlim;
	dm_i2c_reg_write(udev, 0x00, val);
	log_debug("REG00 0x%x\n", val);

	return 0;
}

/*
 * set some safe defaults for the battery charger
 */
int init_charger_bq25896(void)
{
	u8 val;
	int iinlim, vbat, ret;
	struct udevice *udev, *bus;

	/* Set the i2c bus */
	ret = uclass_get_device_by_seq(UCLASS_I2C, 3, &bus);
	if (ret) {
		log_debug("%s: No bus 3\n", __func__);
		return ret;
	}

	ret = i2c_get_chip(bus, 0x6a, 1, &udev);
	if (ret) {
		log_debug("%s: setting chip offset failed %d\n", __func__, ret);
		return ret;
	}

	/* enable automatic DC conversions */
	val = dm_i2c_reg_read(udev, 0x02);
	log_debug("REG0B 0x%x\n", val);
	val = (val & ~0xc0) | 0x40;
	dm_i2c_reg_write(udev, 0x02, val);

	/* check VBUS good */
	val = dm_i2c_reg_read(udev, 0x11);
	log_debug("VBUS good %d\n", (val >> 7) & 1);
	log_debug("VBUS mV %d\n", (val & 0x7f) * 100 + 2600);

	val = dm_i2c_reg_read(udev, 0x0b);
	log_debug("REG0B 0x%x\n", val);

	log_debug("VBUS_STAT 0x%x\n", val >> 5);
	switch (val >> 5) {
	case 0:
		log_debug("VBUS not detected\n");
		break;
	case 1:
		log_debug("USB SDP IINLIM 500mA\n");
		break;
	case 2:
		log_debug("USB CDP IINLIM 1500mA\n");
		break;
	case 3:
		log_debug("USB DCP IINLIM 3500mA\n");
		break;
	case 4:
		log_debug("MAXCHARGE IINLIM 1500mA\n");
		break;
	case 5:
		log_debug("Unknown IINLIM 500mA\n");
		break;
	case 6:
		log_debug("DIVIDER IINLIM > 1000mA\n");
		break;
	case 7:
		log_debug("OTG\n");
		break;
	};

	log_debug("CHRG_STAT 0x%x\n", (val >> 3) & 0x3);
	log_debug("PG_STAT 0x%x\n", (val >> 2) & 1);
	log_debug("SDP_STAT 0x%x\n", (val >> 1) & 1);
	log_debug("VSYS_STAT 0x%x\n", val & 1);

	val = dm_i2c_reg_read(udev, 0x00);
	log_debug("REG00 0x%x\n", val);
	iinlim = 100 + (val & 0x3f) * 50;
	log_debug("IINLIM %d mA\n", iinlim);
	log_debug("EN_HIZ 0x%x\n", (val >> 7) & 1);
	log_debug("EN_ILIM 0x%x\n", (val >> 6) & 1);

	/* check VBAT */
	vbat = read_vbat();
	log_debug("VBAT mV %d\n", vbat);

	if (vbat < 2800 && iinlim <= 500) {
		/* battery voltage too low and
		 * insufficient current to boot linux.
		 */
		log_err("%s: voltage and current too low linux probably won't boot\n", __func__);
	}

	/* set 1.6A charge limit */
	dm_i2c_reg_write(udev, 0x04, 0x19);

	/* re-enable charger */
	val = dm_i2c_reg_read(udev, 0x03);
	val = val | 0x10;
	dm_i2c_reg_write(udev, 0x03, val);

	/* limit the VINDPM to 3.9V  */
	dm_i2c_reg_write(udev, 0x0d, 0x8d);

	/* set the max voltage to 4.192V */
	val = dm_i2c_reg_read(udev, 0x6);
	val = (val & ~0xFC) | 0x16 << 2;
	dm_i2c_reg_write(udev, 0x6, val);

	/* set the SYS_MIN to 3.7V */
	val = dm_i2c_reg_read(udev, 0x3);
	val = val | 0xE;
	dm_i2c_reg_write(udev, 0x3, val);

	return 0;
}

int board_init(void)
{
	struct udevice *dev;
	int tps_ret;

	log_debug("%s: initializing USB clk\n", __func__);

	/* init_usb_clk won't enable the second clock if it's a USB boot */
	if (is_usb_boot()) {
		imx8m_usb_power(1, true);

		clock_enable(CCGR_USB_CTRL2, 1);
		clock_enable(CCGR_USB_PHY2, 1);
	}

	printf("Enabling regulator-hub\n");
	if (!regulator_get_by_devname("regulator-hub", &dev)) {
		if (regulator_set_enable(dev, true))
			printf("Failed to enable regulator-hub\n");
	}

	tps_ret = init_tps65982();
	init_charger_bq25896();

	if (!tps_ret) {
		int current = tps65982_get_max_current();

		if (current > 500)
			bq25896_set_iinlim(current);
	}

	return 0;
}

int board_late_init(void)
{
	u32 rev;
	char rev_str[3];

	env_set("board_name", "librem5");
	if (fuse_read(9, 0, &rev)) {
		env_set("board_rev", BOARD_REV_ERROR);
	} else if (rev == 0) {
		env_set("board_rev", BOARD_REV_UNKNOWN);
	} else if (rev > 0) {
		sprintf(rev_str, "%u", rev);
		env_set("board_rev", rev_str);
	}

	printf("Board name: %s\n", env_get("board_name"));
	printf("Board rev:  %s\n", env_get("board_rev"));

	if (is_usb_boot()) {
		puts("USB Boot\n");
		env_set("bootcmd", "fastboot 0");
	}

	return 0;
}
