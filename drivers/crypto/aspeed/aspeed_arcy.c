// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2021 ASPEED Technology Inc.
 */
#include <config.h>
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <asm/types.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/fdtaddr.h>
#include <linux/delay.h>
#include <u-boot/rsa-mod-exp.h>

/* ARCY register offsets */
#define ARCY_CTRL1		0x00
#define   ARCY_CTRL1_RSA_DMA		BIT(1)
#define   ARCY_CTRL1_RSA_START		BIT(0)
#define ARCY_CTRL2		0x44
#define ARCY_CTRL3		0x48
#define   ARCY_CTRL3_SRAM_AHB_ACCESS	BIT(8)
#define   ARCY_CTRL3_ECC_RSA_MODE_MASK	GENMASK(5, 4)
#define   ARCY_CTRL3_ECC_RSA_MODE_SHIFT	4
#define ARCY_DMA_DRAM_SADDR	0x4c
#define ARCY_DMA_DMEM_TADDR	0x50
#define   ARCY_DMA_DMEM_TADDR_LEN_MASK	GENMASK(15, 0)
#define   ARCY_DMA_DMEM_TADDR_LEN_SHIFT	0
#define ARCY_RSA_PARAM		0x58
#define   ARCY_RSA_PARAM_EXP_MASK	GENMASK(31, 16)
#define   ARCY_RSA_PARAM_EXP_SHIFT	16
#define   ARCY_RSA_PARAM_MOD_MASK	GENMASK(15, 0)
#define   ARCY_RSA_PARAM_MOD_SHIFT	0
#define ARCY_RSA_INT_EN		0x3f8
#define   ARCY_RSA_INT_EN_RSA_READY	BIT(2)
#define   ARCY_RSA_INT_EN_RSA_CMPLT	BIT(1)
#define ARCY_RSA_INT_STS	0x3fc
#define   ARCY_RSA_INT_STS_RSA_READY	BIT(2)
#define   ARCY_RSA_INT_STS_RSA_CMPLT	BIT(1)

/* misc. constant */
#define ARCY_ECC_MODE	2
#define ARCY_RSA_MODE	3
#define ARCY_CTX_BUFSZ	0x600

struct aspeed_arcy {
	phys_addr_t base;
	phys_addr_t sram_base; /* internal sram */
	struct clk clk;
};

static int aspeed_arcy_mod_exp(struct udevice *dev, const uint8_t *sig, uint32_t sig_len,
			       struct key_prop *prop, uint8_t *out)
{
	int i, j;
	u8 *ctx;
	u8 *ptr;
	u32 reg;
	struct aspeed_arcy *arcy = dev_get_priv(dev);

	ctx = memalign(16, ARCY_CTX_BUFSZ);
	if (!ctx)
		return -ENOMEM;

	memset(ctx, 0, ARCY_CTX_BUFSZ);

	ptr = (u8 *)prop->public_exponent;
	for (i = prop->exp_len - 1, j = 0; i >= 0; --i) {
		ctx[j] = ptr[i];
		j++;
		j = (j % 16) ? j : j + 32;
	}

	ptr = (u8 *)prop->modulus;
	for (i = (prop->num_bits >> 3) - 1, j = 0; i >= 0; --i) {
		ctx[j + 16] = ptr[i];
		j++;
		j = (j % 16) ? j : j + 32;
	}

	ptr = (u8 *)sig;
	for (i = sig_len - 1, j = 0; i >= 0; --i) {
		ctx[j + 32] = ptr[i];
		j++;
		j = (j % 16) ? j : j + 32;
	}

	writel((u32)ctx, arcy->base + ARCY_DMA_DRAM_SADDR);

	reg = (((prop->exp_len << 3) << ARCY_RSA_PARAM_EXP_SHIFT) & ARCY_RSA_PARAM_EXP_MASK) |
		  ((prop->num_bits << ARCY_RSA_PARAM_MOD_SHIFT) & ARCY_RSA_PARAM_MOD_MASK);
	writel(reg, arcy->base + ARCY_RSA_PARAM);

	reg = (ARCY_CTX_BUFSZ << ARCY_DMA_DMEM_TADDR_LEN_SHIFT) & ARCY_DMA_DMEM_TADDR_LEN_MASK;
	writel(reg, arcy->base + ARCY_DMA_DMEM_TADDR);

	reg = (ARCY_RSA_MODE << ARCY_CTRL3_ECC_RSA_MODE_SHIFT) & ARCY_CTRL3_ECC_RSA_MODE_MASK;
	writel(reg, arcy->base + ARCY_CTRL3);

	writel(ARCY_CTRL1_RSA_DMA | ARCY_CTRL1_RSA_START, arcy->base + ARCY_CTRL1);

	/* polling RSA status */
	while (1) {
		reg = readl(arcy->base + ARCY_RSA_INT_STS);
		if ((reg & ARCY_RSA_INT_STS_RSA_READY) && (reg & ARCY_RSA_INT_STS_RSA_CMPLT))
			break;
		udelay(20);
	}

	writel(0x0, arcy->base + ARCY_CTRL1);
	writel(ARCY_CTRL3_SRAM_AHB_ACCESS, arcy->base + ARCY_CTRL3);
	udelay(20);

	for (i = (prop->num_bits / 8) - 1, j = 0; i >= 0; --i) {
		out[i] = readb(arcy->sram_base + (j + 32));
		j++;
		j = (j % 16) ? j : j + 32;
	}

	return 0;
}

static int aspeed_arcy_probe(struct udevice *dev)
{
	struct aspeed_arcy *arcy = dev_get_priv(dev);
	int ret;

	ret = clk_get_by_index(dev, 0, &arcy->clk);
	if (ret < 0) {
		debug("Can't get clock for %s: %d\n", dev->name, ret);
		return ret;
	}

	ret = clk_enable(&arcy->clk);
	if (ret) {
		debug("Failed to enable arcy clock (%d)\n", ret);
		return ret;
	}

	arcy->base = devfdt_get_addr_index(dev, 0);
	if (arcy->base == FDT_ADDR_T_NONE) {
		debug("Failed to get arcy base\n");
		return arcy->base;
	}

	arcy->sram_base = devfdt_get_addr_index(dev, 1);
	if (arcy->sram_base == FDT_ADDR_T_NONE) {
		debug("Failed to get arcy SRAM base\n");
		return arcy->sram_base;
	}

	return ret;
}

static int aspeed_arcy_remove(struct udevice *dev)
{
	struct aspeed_arcy *arcy = dev_get_priv(dev);

	clk_disable(&arcy->clk);

	return 0;
}

static const struct mod_exp_ops aspeed_arcy_ops = {
	.mod_exp = aspeed_arcy_mod_exp,
};

static const struct udevice_id aspeed_arcy_ids[] = {
	{ .compatible = "aspeed,ast2600-arcy" },
	{ }
};

U_BOOT_DRIVER(aspeed_arcy) = {
	.name = "aspeed_arcy",
	.id = UCLASS_MOD_EXP,
	.of_match = aspeed_arcy_ids,
	.probe = aspeed_arcy_probe,
	.remove = aspeed_arcy_remove,
	.priv_auto = sizeof(struct aspeed_arcy),
	.ops = &aspeed_arcy_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
