// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#include <common.h>
#include <dm.h>
#include <spi.h>
#include <spi-mem.h>
#include <linux/iopoll.h>
#include <linux/log2.h>
#include <asm/arch/fiu.h>

struct npcm_fiu_priv {
	struct npcm_fiu_regs *regs;
};

static int npcm_fiu_spi_set_speed(struct udevice *bus, uint speed)
{
	return 0;
}

static int npcm_fiu_spi_set_mode(struct udevice *bus, uint mode)
{
	return 0;
}

static inline void activate_cs(struct npcm_fiu_regs *regs, int cs)
{
	writel((cs & 0x3) << FIU_UMA_CTS_DEV_NUM, &regs->uma_cts);
}

static inline void deactivate_cs(struct npcm_fiu_regs *regs)
{
	writel((1 << FIU_UMA_CTS_SW_CS), &regs->uma_cts);
}

static int fiu_uma_read(struct udevice *bus, u8 *buf, u32 data_size)
{
	struct npcm_fiu_priv *priv = dev_get_priv(bus);
	struct npcm_fiu_regs *regs = priv->regs;
	u32 data_reg[4];
	u32 val;
	int ret;

	/* Set data size */
	writel((data_size << FIU_UMA_CFG_RDATSIZ), &regs->uma_cfg);

	/* Initiate the read */
	writel(readl(&regs->uma_cts) | (1 << FIU_UMA_CTS_EXEC_DONE), &regs->uma_cts);

	/* Wait for completion */
	ret = readl_poll_timeout(&regs->uma_cts, val,
				 !(val & (1 << FIU_UMA_CTS_EXEC_DONE)), 1000000);
	if (ret) {
		printf("npcm_fiu: read timeout\n");
		return ret;
	}

	/* Copy data from data registers */
	if (data_size >= 1)
		data_reg[0] = readl(&regs->uma_dr0);
	if (data_size >= 5)
		data_reg[1] = readl(&regs->uma_dr1);
	if (data_size >= 9)
		data_reg[2] = readl(&regs->uma_dr2);
	if (data_size >= 13)
		data_reg[3] = readl(&regs->uma_dr3);

	memcpy(buf, data_reg, data_size);

	return 0;
}

static int fiu_uma_write(struct udevice *bus, const u8 *buf, u32 data_size)
{
	struct npcm_fiu_priv *priv = dev_get_priv(bus);
	struct npcm_fiu_regs *regs = priv->regs;
	u32 data_reg[4];
	u32 val;
	int ret;

	/* Set data size */
	writel((data_size << FIU_UMA_CFG_WDATSIZ), &regs->uma_cfg);

	/* Write data to data registers */
	memcpy(data_reg, buf, data_size);

	if (data_size >= 1)
		writel(data_reg[0], &regs->uma_dw0);
	if (data_size >= 5)
		writel(data_reg[1], &regs->uma_dw1);
	if (data_size >= 9)
		writel(data_reg[2], &regs->uma_dw2);
	if (data_size >= 13)
		writel(data_reg[3], &regs->uma_dw3);

	/* Initiate the transaction */
	writel(readl(&regs->uma_cts) | (1 << FIU_UMA_CTS_EXEC_DONE), &regs->uma_cts);

	/* Wait for completion */
	ret = readl_poll_timeout(&regs->uma_cts, val,
				 !(val & (1 << FIU_UMA_CTS_EXEC_DONE)), 1000000);
	if (ret)
		printf("npcm_fiu: write timeout\n");

	return ret;
}

static int npcm_fiu_spi_xfer(struct udevice *dev, unsigned int bitlen,
			     const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct npcm_fiu_priv *priv = dev_get_priv(bus);
	struct npcm_fiu_regs *regs = priv->regs;
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	const u8 *tx = dout;
	u8 *rx = din;
	int bytes = bitlen / 8;
	int ret = 0;
	int len;

	if (flags & SPI_XFER_BEGIN)
		activate_cs(regs, slave_plat->cs);

	while (bytes) {
		len = (bytes > CHUNK_SIZE) ? CHUNK_SIZE : bytes;
		if (tx) {
			ret = fiu_uma_write(bus, tx, len);
			if (ret)
				break;
			tx += len;
		} else {
			ret = fiu_uma_read(bus, rx, len);
			if (ret)
				break;
			rx += len;
		}
		bytes -= len;
	}

	if (flags & SPI_XFER_END)
		deactivate_cs(regs);

	return ret;
}

static int npcm_fiu_uma_operation(struct npcm_fiu_priv *priv, const struct spi_mem_op *op,
				  u32 addr, const u8 *tx, u8 *rx, u32 nbytes, bool started)
{
	struct npcm_fiu_regs *regs = priv->regs;
	u32 uma_cfg = 0, val;
	u32 *data32;
	int ret;

	debug("fiu_uma: opcode 0x%x, dir %d, addr 0x%x, %d bytes\n",
	      op->cmd.opcode, op->data.dir, addr, nbytes);
	debug("         buswidth cmd:%d, addr:%d, dummy:%d, data:%d\n",
	      op->cmd.buswidth, op->addr.buswidth, op->dummy.buswidth,
	      op->data.buswidth);
	debug("         size cmd:%d, addr:%d, dummy:%d, data:%d\n",
	      1, op->addr.nbytes, op->dummy.nbytes, op->data.nbytes);
	debug("         tx %p, rx %p\n", tx, rx);

	if (!started) {
		/* Send cmd in the begin of an transaction */
		writel(op->cmd.opcode, &regs->uma_cmd);

		uma_cfg |= (ilog2(op->cmd.buswidth) << FIU_UMA_CFG_CMBPCK) |
			   (1 << FIU_UMA_CFG_CMDSIZ);
		if (op->addr.nbytes) {
			uma_cfg |= ilog2(op->addr.buswidth) << FIU_UMA_CFG_ADBPCK |
				  (op->addr.nbytes & 0x7) << FIU_UMA_CFG_ADDSIZ;
			writel(addr, &regs->uma_addr);
		}
		if (op->dummy.nbytes)
			uma_cfg |= ilog2(op->dummy.buswidth) << FIU_UMA_CFG_DBPCK |
				  (op->dummy.nbytes & 0x7) << FIU_UMA_CFG_DBSIZ;
	}
	if (op->data.dir == SPI_MEM_DATA_IN && nbytes)
		uma_cfg |= ilog2(op->data.buswidth) << FIU_UMA_CFG_RDBPCK |
				   (nbytes & 0x1f) << FIU_UMA_CFG_RDATSIZ;
	else if (op->data.dir == SPI_MEM_DATA_OUT && nbytes)
		uma_cfg |= ilog2(op->data.buswidth) << FIU_UMA_CFG_WDBPCK |
				   (nbytes & 0x1f) << FIU_UMA_CFG_WDATSIZ;
	writel(uma_cfg, &regs->uma_cfg);

	if (op->data.dir == SPI_MEM_DATA_OUT && nbytes) {
		data32 = (u32 *)tx;
		if (nbytes >= 1)
			writel(*data32++, &regs->uma_dw0);
		if (nbytes >= 5)
			writel(*data32++, &regs->uma_dw1);
		if (nbytes >= 9)
			writel(*data32++, &regs->uma_dw2);
		if (nbytes >= 13)
			writel(*data32++, &regs->uma_dw3);
	}
	/* Initiate the transaction */
	writel(readl(&regs->uma_cts) | (1 << FIU_UMA_CTS_EXEC_DONE), &regs->uma_cts);

	/* Wait for completion */
	ret = readl_poll_timeout(&regs->uma_cts, val,
				 !(val & (1 << FIU_UMA_CTS_EXEC_DONE)), 1000000);
	if (ret) {
		printf("npcm_fiu: UMA op timeout\n");
		return ret;
	}

	if (op->data.dir == SPI_MEM_DATA_IN && nbytes) {
		data32 = (u32 *)rx;
		if (nbytes >= 1)
			*data32++ = readl(&regs->uma_dr0);
		if (nbytes >= 5)
			*data32++ = readl(&regs->uma_dr1);
		if (nbytes >= 9)
			*data32++ = readl(&regs->uma_dr2);
		if (nbytes >= 13)
			*data32++ = readl(&regs->uma_dr3);
	}

	return 0;
}

static int npcm_fiu_exec_op(struct spi_slave *slave,
			    const struct spi_mem_op *op)
{
	struct udevice *bus = slave->dev->parent;
	struct npcm_fiu_priv *priv = dev_get_priv(bus);
	struct npcm_fiu_regs *regs = priv->regs;
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(slave->dev);
	u32 bytes, len;
	const u8 *tx;
	u8 *rx;
	int ret;
	bool started = false;
	u32 addr;

	bytes = op->data.nbytes;
	addr = (u32)op->addr.val;
	if (!bytes) {
		activate_cs(regs, slave_plat->cs);
		ret = npcm_fiu_uma_operation(priv, op, addr, NULL, NULL, 0, started);
		started = true;
		goto end;
	}

	tx = op->data.buf.out;
	rx = op->data.buf.in;
	while (bytes) {
		if (!started)
			activate_cs(regs, slave_plat->cs);

		len = (bytes > CHUNK_SIZE) ? CHUNK_SIZE : bytes;
		ret = npcm_fiu_uma_operation(priv, op, addr, tx, rx, len, started);
		started = true;
		if (ret)
			break;
		bytes -= len;
		addr += len;
		if (tx)
			tx += len;
		if (rx)
			rx += len;

		if (started && op->data.dir != SPI_MEM_DATA_OUT) {
			deactivate_cs(regs);
			started = false;
		}
	}
end:
	if (started)
		deactivate_cs(regs);

	return ret;
}

static int npcm_fiu_spi_probe(struct udevice *bus)
{
	struct npcm_fiu_priv *priv = dev_get_priv(bus);

	priv->regs = (struct npcm_fiu_regs *)dev_read_addr_ptr(bus);

	return 0;
}

static const struct spi_controller_mem_ops npcm_fiu_mem_ops = {
	.exec_op = npcm_fiu_exec_op,
};

static const struct dm_spi_ops npcm_fiu_spi_ops = {
	.xfer           = npcm_fiu_spi_xfer,
	.set_speed      = npcm_fiu_spi_set_speed,
	.set_mode       = npcm_fiu_spi_set_mode,
	.mem_ops        = &npcm_fiu_mem_ops,
};

static const struct udevice_id npcm_fiu_spi_ids[] = {
	{ .compatible = "nuvoton,npcm845-fiu" },
	{ .compatible = "nuvoton,npcm750-fiu" },
	{ }
};

U_BOOT_DRIVER(npcm_fiu_spi) = {
	.name   = "npcm_fiu_spi",
	.id     = UCLASS_SPI,
	.of_match = npcm_fiu_spi_ids,
	.ops    = &npcm_fiu_spi_ops,
	.priv_auto = sizeof(struct npcm_fiu_priv),
	.probe  = npcm_fiu_spi_probe,
};
