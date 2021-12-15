// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Nuvoton Technology.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/gpio.h>
#include <dm/device.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/sizes.h>

#define NPCM_GPIO_PORTS_PER_BANK    32

#define NPCM_GPIO_REG_DIN		0x04	/* RO - Data In */
#define NPCM_GPIO_REG_DOUT		0x0C	/* RW - Data Out */
#define NPCM_GPIO_REG_IEM		0x58	/* RW - Input Enable Mask */
#define NPCM_GPIO_REG_OE		0x10	/* RW - Output Enable */
#define NPCM_GPIO_REG_OES		0x70	/* WO - Output Enable Register Set */
#define NPCM_GPIO_REG_OEC		0x74	/* WO - Output Enable Register Clear */

struct npcm_gpio_priv {
	void __iomem *base;
};

static void npcm_gpio_offset_write(struct udevice *dev, unsigned int offset,
				   unsigned int reg, int value)
{
	struct npcm_gpio_priv *priv = dev_get_priv(dev);
	u32 tmp;

	tmp = readl(priv->base + reg);

	if (value)
		tmp |= BIT(offset);
	else
		tmp &= ~BIT(offset);

	writel(tmp, priv->base + reg);
}

static int npcm_gpio_offset_read(struct udevice *dev, unsigned int offset,
				 unsigned int reg)
{
	struct npcm_gpio_priv *priv = dev_get_priv(dev);

	return !!(readl(priv->base + reg) & BIT(offset));
}

static int npcm_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	npcm_gpio_offset_write(dev, offset, NPCM_GPIO_REG_OEC, 1);
	npcm_gpio_offset_write(dev, offset, NPCM_GPIO_REG_IEM, 1);

	return 0;
}

static int npcm_gpio_direction_output(struct udevice *dev, unsigned int offset,
				      int value)
{
	npcm_gpio_offset_write(dev, offset, NPCM_GPIO_REG_IEM, 0);
	npcm_gpio_offset_write(dev, offset, NPCM_GPIO_REG_OES, 1);
	npcm_gpio_offset_write(dev, offset, NPCM_GPIO_REG_DOUT, value);

	return 0;
}

static int npcm_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	if (npcm_gpio_offset_read(dev, offset, NPCM_GPIO_REG_IEM))
		return npcm_gpio_offset_read(dev, offset, NPCM_GPIO_REG_DIN);

	if (npcm_gpio_offset_read(dev, offset, NPCM_GPIO_REG_OE))
		return npcm_gpio_offset_read(dev, offset, NPCM_GPIO_REG_DOUT);

	return -EINVAL;
}

static int npcm_gpio_set_value(struct udevice *dev, unsigned int offset,
			       int value)
{
	npcm_gpio_offset_write(dev, offset, NPCM_GPIO_REG_DOUT, value);

	return 0;
}

static int npcm_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	if (npcm_gpio_offset_read(dev, offset, NPCM_GPIO_REG_IEM))
		return GPIOF_INPUT;

	if (npcm_gpio_offset_read(dev, offset, NPCM_GPIO_REG_OE))
		return GPIOF_OUTPUT;

	return GPIOF_FUNC;
}

static const struct dm_gpio_ops npcm_gpio_ops = {
	.direction_input	= npcm_gpio_direction_input,
	.direction_output	= npcm_gpio_direction_output,
	.get_value		= npcm_gpio_get_value,
	.set_value		= npcm_gpio_set_value,
	.get_function		= npcm_gpio_get_function,
};

static int npcm_gpio_probe(struct udevice *dev)
{
	struct npcm_gpio_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	uc_priv->gpio_count = NPCM_GPIO_PORTS_PER_BANK;
	uc_priv->bank_name = dev_read_string(dev, "gpio-bank-name");

	return 0;
}

static const struct udevice_id npcm_gpio_match[] = {
	{ .compatible = "nuvoton,npcm845-gpio" },
	{ .compatible = "nuvoton,npcm750-gpio" },
	{ }
};

U_BOOT_DRIVER(npcm_gpio) = {
	.name	= "npcm_gpio",
	.id	= UCLASS_GPIO,
	.of_match = npcm_gpio_match,
	.probe	= npcm_gpio_probe,
	.priv_auto = sizeof(struct npcm_gpio_priv),
	.ops	= &npcm_gpio_ops,
};
