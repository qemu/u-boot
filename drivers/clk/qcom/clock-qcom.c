// SPDX-License-Identifier: BSD-3-Clause AND GPL-2.0
/*
 * Clock and reset drivers for Qualcomm platforms Global Clock
 * Controller (GCC).
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 * (C) Copyright 2020 Sartura Ltd. (reset driver)
 *     Author: Robert Marko <robert.marko@sartura.hr>
 * (C) Copyright 2022 Linaro Ltd. (reset driver)
 *     Author: Sumit Garg <sumit.garg@linaro.org>
 *
 * Based on Little Kernel driver, simplified
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <reset-uclass.h>
#include <power-domain-uclass.h>

#include "clock-qcom.h"

/* CBCR register fields */
#define CBCR_BRANCH_ENABLE_BIT  BIT(0)
#define CBCR_BRANCH_OFF_BIT     BIT(31)

#define GDSC_SW_COLLAPSE_MASK		BIT(0)
#define GDSC_POWER_DOWN_COMPLETE	BIT(15)
#define GDSC_POWER_UP_COMPLETE		BIT(16)
#define GDSC_PWR_ON_MASK		BIT(31)
#define CFG_GDSCR_OFFSET		0x4
#define GDSC_STATUS_POLL_TIMEOUT_US	1500

/* Enable clock controlled by CBC soft macro */
void clk_enable_cbc(phys_addr_t cbcr)
{
	setbits_le32(cbcr, CBCR_BRANCH_ENABLE_BIT);

	while (readl(cbcr) & CBCR_BRANCH_OFF_BIT)
		;
}

void clk_enable_gpll0(phys_addr_t base, const struct pll_vote_clk *gpll0)
{
	if (readl(base + gpll0->status) & gpll0->status_bit)
		return; /* clock already enabled */

	setbits_le32(base + gpll0->ena_vote, gpll0->vote_bit);

	while ((readl(base + gpll0->status) & gpll0->status_bit) == 0)
		;
}

#define BRANCH_ON_VAL (0)
#define BRANCH_NOC_FSM_ON_VAL BIT(29)
#define BRANCH_CHECK_MASK GENMASK(31, 28)

void clk_enable_vote_clk(phys_addr_t base, const struct vote_clk *vclk)
{
	u32 val;

	setbits_le32(base + vclk->ena_vote, vclk->vote_bit);
	do {
		val = readl(base + vclk->cbcr_reg);
		val &= BRANCH_CHECK_MASK;
	} while ((val != BRANCH_ON_VAL) && (val != BRANCH_NOC_FSM_ON_VAL));
}

#define APPS_CMD_RCGR_UPDATE BIT(0)

/* Update clock command via CMD_RCGR */
void clk_bcr_update(phys_addr_t apps_cmd_rcgr)
{
	u32 count;
	setbits_le32(apps_cmd_rcgr, APPS_CMD_RCGR_UPDATE);

	/* Wait for frequency to be updated. */
	for (count = 0; count < 50000; count++) {
		if (!(readl(apps_cmd_rcgr) & APPS_CMD_RCGR_UPDATE))
			break;
		udelay(1);
	}
	WARN(count == 50000, "WARNING: RCG @ %#llx [%#010x] stuck at off\n",
	     apps_cmd_rcgr, readl(apps_cmd_rcgr));
}

#define CFG_SRC_DIV_MASK	0b11111
#define CFG_SRC_SEL_SHIFT	8
#define CFG_SRC_SEL_MASK	(0x7 << CFG_SRC_SEL_SHIFT)
#define CFG_MODE_SHIFT		12
#define CFG_MODE_MASK		(0x3 << CFG_MODE_SHIFT)
#define CFG_MODE_DUAL_EDGE	(0x2 << CFG_MODE_SHIFT)
#define CFG_HW_CLK_CTRL_MASK	BIT(20)

/*
 * root set rate for clocks with half integer and MND divider
 * div should be pre-calculated ((div * 2) - 1)
 */
void clk_rcg_set_rate_mnd(phys_addr_t base, uint32_t cmd_rcgr,
			  int div, int m, int n, int source, u8 mnd_width)
{
	u32 cfg;
	/* M value for MND divider. */
	u32 m_val = m;
	u32 n_minus_m = n - m;
	/* NOT(N-M) value for MND divider. */
	u32 n_val = ~n_minus_m * !!(n);
	/* NOT 2D value for MND divider. */
	u32 d_val = ~(clamp_t(u32, n, m, n_minus_m));
	u32 mask = BIT(mnd_width) - 1;

	debug("m %#x n %#x d %#x div %#x mask %#x\n", m_val, n_val, d_val, div, mask);

	/* Program MND values */
	writel(m_val & mask, base + cmd_rcgr + RCG_M_REG);
	writel(n_val & mask, base + cmd_rcgr + RCG_N_REG);
	writel(d_val & mask, base + cmd_rcgr + RCG_D_REG);

	/* setup src select and divider */
	cfg  = readl(base + cmd_rcgr + RCG_CFG_REG);
	cfg &= ~(CFG_SRC_SEL_MASK | CFG_MODE_MASK | CFG_HW_CLK_CTRL_MASK |
		 CFG_SRC_DIV_MASK);
	cfg |= source & CFG_SRC_SEL_MASK; /* Select clock source */

	if (div)
		cfg |= div & CFG_SRC_DIV_MASK;

	if (n && n != m)
		cfg |= CFG_MODE_DUAL_EDGE;

	writel(cfg, base + cmd_rcgr + RCG_CFG_REG); /* Write new clock configuration */

	/* Inform h/w to start using the new config. */
	clk_bcr_update(base + cmd_rcgr);
}

/* root set rate for clocks with half integer and mnd_width=0 */
void clk_rcg_set_rate(phys_addr_t base, uint32_t cmd_rcgr, int div,
		      int source)
{
	u32 cfg;

	/* setup src select and divider */
	cfg  = readl(base + cmd_rcgr + RCG_CFG_REG);
	cfg &= ~(CFG_SRC_SEL_MASK | CFG_MODE_MASK | CFG_HW_CLK_CTRL_MASK);
	cfg |= source & CFG_CLK_SRC_MASK; /* Select clock source */

	/*
	 * Set the divider; HW permits fraction dividers (+0.5), but
	 * for simplicity, we will support integers only
	 */
	if (div)
		cfg |= (2 * div - 1) & CFG_SRC_DIV_MASK;

	writel(cfg, base + cmd_rcgr + RCG_CFG_REG); /* Write new clock configuration */

	/* Inform h/w to start using the new config. */
	clk_bcr_update(base + cmd_rcgr);
}

const struct freq_tbl *qcom_find_freq(const struct freq_tbl *f, uint rate)
{
	if (!f)
		return NULL;

	if (!f->freq)
		return f;

	for (; f->freq; f++)
		if (rate <= f->freq)
			return f;

	/* Default to our fastest rate */
	return f - 1;
}

static int msm_clk_probe(struct udevice *dev)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(dev);
	struct msm_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->data = data;

	return 0;
}

static ulong msm_clk_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(clk->dev);

	if (data->set_rate)
		return data->set_rate(clk, rate);

	return 0;
}

static int msm_clk_enable(struct clk *clk)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(clk->dev);

	if (data->enable)
		return data->enable(clk);

	return 0;
}

static struct clk_ops msm_clk_ops = {
	.set_rate = msm_clk_set_rate,
	.enable = msm_clk_enable,
};

U_BOOT_DRIVER(qcom_clk) = {
	.name		= "qcom_clk",
	.id		= UCLASS_CLK,
	.ops		= &msm_clk_ops,
	.priv_auto	= sizeof(struct msm_clk_priv),
	.probe		= msm_clk_probe,
	.flags		= DM_FLAG_PRE_RELOC | DM_FLAG_DEFAULT_PD_CTRL_OFF,
};

int qcom_cc_bind(struct udevice *parent)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(parent);
	struct udevice *clkdev = NULL, *rstdev = NULL, *pwrdev;
	struct driver *drv;
	int ret;

	/* Get a handle to the common clk handler */
	drv = lists_driver_lookup_name("qcom_clk");
	if (!drv)
		return -ENOENT;

	/* Register the clock controller */
	ret = device_bind_with_driver_data(parent, drv, "qcom_clk", (ulong)data,
					   dev_ofnode(parent), &clkdev);
	if (ret)
		return ret;

	if (data->resets) {
		/* Get a handle to the common reset handler */
		drv = lists_driver_lookup_name("qcom_reset");
		if (!drv) {
			ret = -ENOENT;
			goto unbind_clkdev;
		}

		/* Register the reset controller */
		ret = device_bind_with_driver_data(parent, drv, "qcom_reset", (ulong)data,
						   dev_ofnode(parent), &rstdev);
		if (ret)
			goto unbind_clkdev;
	}

	if (data->power_domains) {
		/* Get a handle to the common power domain handler */
		drv = lists_driver_lookup_name("qcom_power");
		if (!drv) {
			ret = -ENOENT;
			goto unbind_rstdev;
		}
		/* Register the power domain controller */
		ret = device_bind_with_driver_data(parent, drv, "qcom_power", (ulong)data,
						   dev_ofnode(parent), &pwrdev);
		if (ret)
			goto unbind_rstdev;
	}

	return 0;

unbind_rstdev:
	device_unbind(rstdev);
unbind_clkdev:
	device_unbind(clkdev);

	return ret;
}

static int qcom_reset_set(struct reset_ctl *rst, bool assert)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(rst->dev);
	void __iomem *base = dev_get_priv(rst->dev);
	const struct qcom_reset_map *map;
	u32 value;

	map = &data->resets[rst->id];

	value = readl(base + map->reg);

	if (assert)
		value |= BIT(map->bit);
	else
		value &= ~BIT(map->bit);

	writel(value, base + map->reg);

	return 0;
}

static int qcom_reset_assert(struct reset_ctl *rst)
{
	return qcom_reset_set(rst, true);
}

static int qcom_reset_deassert(struct reset_ctl *rst)
{
	return qcom_reset_set(rst, false);
}

static const struct reset_ops qcom_reset_ops = {
	.rst_assert = qcom_reset_assert,
	.rst_deassert = qcom_reset_deassert,
};

static int qcom_reset_probe(struct udevice *dev)
{
	/* Set our priv pointer to the base address */
	dev_set_priv(dev, (void *)dev_read_addr(dev));

	return 0;
}

U_BOOT_DRIVER(qcom_reset) = {
	.name = "qcom_reset",
	.id = UCLASS_RESET,
	.ops = &qcom_reset_ops,
	.probe = qcom_reset_probe,
};

static int qcom_power_set(struct power_domain *pwr, bool on)
{
	struct msm_clk_data *data = (struct msm_clk_data *)dev_get_driver_data(pwr->dev);
	void __iomem *base = dev_get_priv(pwr->dev);
	const struct qcom_power_map *map;
	u32 value;
	int ret;

	if (pwr->id >= data->num_power_domains)
		return -ENODEV;

	map = &data->power_domains[pwr->id];

	if (!map->reg)
		return -ENODEV;

	value = readl(base + map->reg);

	if (on)
		value &= ~GDSC_SW_COLLAPSE_MASK;
	else
		value |= GDSC_SW_COLLAPSE_MASK;

	writel(value, base + map->reg);

	if (on)
		ret = readl_poll_timeout(base + map->reg + CFG_GDSCR_OFFSET,
					 value,
					 (value & GDSC_POWER_UP_COMPLETE) ||
					 (value & GDSC_PWR_ON_MASK),
					 GDSC_STATUS_POLL_TIMEOUT_US);

	else
		ret = readl_poll_timeout(base + map->reg + CFG_GDSCR_OFFSET,
					 value,
					 (value & GDSC_POWER_DOWN_COMPLETE) ||
					 !(value & GDSC_PWR_ON_MASK),
					 GDSC_STATUS_POLL_TIMEOUT_US);


	if (ret == -ETIMEDOUT)
		printf("WARNING: GDSC %lu is stuck during power on/off\n",
		       pwr->id);
	return ret;
}

static int qcom_power_on(struct power_domain *pwr)
{
	return qcom_power_set(pwr, true);
}

static int qcom_power_off(struct power_domain *pwr)
{
	return qcom_power_set(pwr, false);
}

static const struct power_domain_ops qcom_power_ops = {
	.on = qcom_power_on,
	.off = qcom_power_off,
};

static int qcom_power_probe(struct udevice *dev)
{
	/* Set our priv pointer to the base address */
	dev_set_priv(dev, (void *)dev_read_addr(dev));

	return 0;
}

U_BOOT_DRIVER(qcom_power) = {
	.name = "qcom_power",
	.id = UCLASS_POWER_DOMAIN,
	.ops = &qcom_power_ops,
	.probe = qcom_power_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
