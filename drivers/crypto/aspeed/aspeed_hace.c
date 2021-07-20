// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2021 IBM Corp.
 * Copyright 2021 ASPEED Technology Inc.
 */
#include <common.h>
#include <clk.h>
#include <log.h>
#include <asm/io.h>
#include <malloc.h>
#include <hash.h>
#include <reset.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/fdtaddr.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/iopoll.h>

/* register offsets*/
#define HACE_STS		0x1C
#define   HACE_HASH_DATA_OVF	BIT(23)
#define   HACE_HASH_INT		BIT(9)
#define   HACE_HASH_BUSY	BIT(0)
#define HACE_HASH_DATA		0x20
#define HACE_HASH_DIGEST	0x24
#define HACE_HASH_HMAC_KEY	0x28
#define HACE_HASH_DATA_LEN	0x2C
#define   HACE_SG_LAST		BIT(31)
#define HACE_HASH_CMD		0x30
#define   HACE_SG_EN		BIT(18)
#define   HACE_ALGO_SHA384	(BIT(10) | BIT(6) | BIT(5))
#define   HACE_ALGO_SHA512	(BIT(6) | BIT(5))
#define   HACE_ALGO_SHA256	(BIT(6) | BIT(4))
#define   HACE_ALGO_SHA224	BIT(6)
#define   HACE_ALGO_SHA1	BIT(5)
#define   HACE_SHA_BE_EN	BIT(3)
#define   HACE_MD5_LE_EN	BIT(2)

#define HACE_MAX_SG	32

struct aspeed_sg {
	u32 len;
	u32 addr;
};

struct aspeed_hash_ctx {
	u32 method;
	u32 digest_size;
	u32 len;
	u32 count;
	struct aspeed_sg list[HACE_MAX_SG]; /* Must be 8 byte aligned */
};

struct aspeed_hace {
	phys_addr_t base;
	struct clk clk;
	struct reset_ctl rst;
};

static int aspeed_hace_wait_completion(u32 reg, u32 flag, int timeout_us)
{
	u32 val;

	return readl_poll_timeout(reg, val, (val & flag) == flag, timeout_us);
}

static int digest_object(const void *data, unsigned int length, void *digest,
			 u32 method)
{
	int rc;
	u32 sts;
	struct udevice *dev;
	struct aspeed_hace *hace;

	if (!((u32)data & BIT(31))) {
		debug("HACE src out of bounds: can only copy from SDRAM\n");
		return -EINVAL;
	}

	if (!((u32)digest & BIT(31))) {
		debug("HACE dst out of bounds: can only copy to SDRAM\n");
		return -EINVAL;
	}

	if ((u32)digest & 0x7) {
		debug("HACE dst alignment incorrect: %p\n", digest);
		return -EINVAL;
	}

	/* get HACE device as crypto code does not pass us device/driver state */
	rc = uclass_get_device_by_driver(UCLASS_MISC,
					 DM_DRIVER_GET(aspeed_hace),
					 &dev);
	if (rc) {
		debug("Can't get HACE device, rc=%d\n", rc);
		return -ENODEV;
	}

	hace = dev_get_priv(dev);

	sts = readl(hace->base + HACE_STS);
	if (sts & HACE_HASH_BUSY) {
		debug("HACE error: engine busy\n");
		return -EBUSY;
	}

	/* Clear pending completion status */
	writel(HACE_HASH_INT, hace->base + HACE_STS);
	writel((u32)data, hace->base + HACE_HASH_DATA);
	writel((u32)digest, hace->base + HACE_HASH_DIGEST);
	writel(length, hace->base + HACE_HASH_DATA_LEN);
	writel(HACE_SHA_BE_EN | method, hace->base + HACE_HASH_CMD);

	/* SHA512 hashing appears to have a througput of about 12MB/s */
	rc = aspeed_hace_wait_completion(hace->base + HACE_STS,
					 HACE_HASH_INT,
					 1000 + (length >> 3));

	/*
	 * Reset HACE to work around unexpected hash input data
	 * buffer overflow caused by certain combinations of DMA
	 * base and length, even though they are valid.
	 */
	sts = readl(hace->base + HACE_STS);
	if (sts & HACE_HASH_DATA_OVF) {
		debug("HACE error: status=0x%08x, resetting\n", sts);
		reset_assert(&hace->rst);
		mdelay(5);
		reset_deassert(&hace->rst);
	}

	return rc;
}

void hw_sha1(const unsigned char *pbuf, unsigned int buf_len,
	     unsigned char *pout, unsigned int chunk_size)
{
	int rc;

	rc = digest_object(pbuf, buf_len, pout, HACE_ALGO_SHA1);
	if (rc)
		debug("HACE failure: %d\n", rc);
}

void hw_sha256(const unsigned char *pbuf, unsigned int buf_len,
	       unsigned char *pout, unsigned int chunk_size)
{
	int rc;

	rc = digest_object(pbuf, buf_len, pout, HACE_ALGO_SHA256);
	if (rc)
		debug("HACE failure: %d\n", rc);
}

void hw_sha384(const unsigned char *pbuf, unsigned int buf_len,
	       unsigned char *pout, unsigned int chunk_size)
{
	int rc;

	rc = digest_object(pbuf, buf_len, pout, HACE_ALGO_SHA384);
	if (rc)
		debug("HACE failure: %d\n", rc);
}

void hw_sha512(const unsigned char *pbuf, unsigned int buf_len,
	       unsigned char *pout, unsigned int chunk_size)
{
	int rc;

	rc = digest_object(pbuf, buf_len, pout, HACE_ALGO_SHA512);
	if (rc)
		debug("HACE failure: %d\n", rc);
}

int hw_sha_init(struct hash_algo *algo, void **ctxp)
{
	struct aspeed_hash_ctx *ctx;
	u32 method;

	if (!strcmp(algo->name, "sha1"))
		method = HACE_ALGO_SHA1;
	else if (!strcmp(algo->name, "sha256"))
		method = HACE_ALGO_SHA256;
	else if (!strcmp(algo->name, "sha384"))
		method = HACE_ALGO_SHA384;
	else if (!strcmp(algo->name, "sha512"))
		method = HACE_ALGO_SHA512;
	else
		return -ENOTSUPP;

	ctx = memalign(8, sizeof(*ctx));
	if (!ctx) {
		debug("HACE error: Cannot allocate memory for context\n");
		return -ENOMEM;
	}

	memset(ctx, '\0', sizeof(*ctx));

	if (((uintptr_t)ctx->list & 0x3) != 0) {
		debug("HACE error: Invalid alignment for input data\n");
		return -EINVAL;
	}

	ctx->method = method | HACE_SG_EN;
	ctx->digest_size = algo->digest_size;

	*ctxp = ctx;

	return 0;
}

int hw_sha_update(struct hash_algo *algo, void *hash_ctx, const void *buf,
		  unsigned int size, int is_last)
{
	struct aspeed_hash_ctx *ctx = hash_ctx;
	struct aspeed_sg *sg = &ctx->list[ctx->count];

	if (ctx->count >= ARRAY_SIZE(ctx->list)) {
		debug("HACE error: Reached maximum number of hash segments\n");
		free(ctx);
		return -EINVAL;
	}

	sg->addr = (u32)buf;
	sg->len = size;

	if (is_last)
		sg->len |= HACE_SG_LAST;

	ctx->count++;
	ctx->len += size;

	return 0;
}

int hw_sha_finish(struct hash_algo *algo, void *hash_ctx, void *dest_buf, int size)
{
	struct aspeed_hash_ctx *ctx = hash_ctx;
	int rc;

	if (size < ctx->digest_size) {
		debug("HACE error: insufficient size on destination buffer\n");
		free(ctx);
		return -EINVAL;
	}

	rc = digest_object(ctx->list, ctx->len, dest_buf, ctx->method);
	if (rc)
		debug("HACE Scatter-Gather failure\n");

	free(ctx);

	return rc;
}

static int aspeed_hace_probe(struct udevice *dev)
{
	struct aspeed_hace *hace = dev_get_priv(dev);
	int ret;

	ret = clk_get_by_index(dev, 0, &hace->clk);
	if (ret < 0) {
		debug("Can't get clock for %s: %d\n", dev->name, ret);
		return ret;
	}

	ret = reset_get_by_index(dev, 0, &hace->rst);
	if (ret < 0) {
		debug("Can't get reset for %s: %d\n", dev->name, ret);
		return ret;
	}

	ret = clk_enable(&hace->clk);
	if (ret) {
		debug("Failed to enable fsi clock (%d)\n", ret);
		return ret;
	}

	hace->base = devfdt_get_addr(dev);

	return ret;
}

static int aspeed_hace_remove(struct udevice *dev)
{
	struct aspeed_hace *hace = dev_get_priv(dev);

	clk_disable(&hace->clk);

	return 0;
}

static const struct udevice_id aspeed_hace_ids[] = {
	{ .compatible = "aspeed,ast2600-hace" },
	{ }
};

U_BOOT_DRIVER(aspeed_hace) = {
	.name = "aspeed_hace",
	.id = UCLASS_MISC,
	.of_match = aspeed_hace_ids,
	.probe = aspeed_hace_probe,
	.remove	= aspeed_hace_remove,
	.priv_auto = sizeof(struct aspeed_hace),
	.flags = DM_FLAG_PRE_RELOC,
};
