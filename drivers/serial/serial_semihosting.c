// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#include <common.h>
#include <dm.h>
#include <serial.h>
#include <semihosting.h>

/**
 * struct smh_serial_priv - Semihosting serial private data
 * @infd: stdin file descriptor (or error)
 */
struct smh_serial_priv {
	int infd;
	int outfd;
};

#if CONFIG_IS_ENABLED(DM_SERIAL)
static int smh_serial_getc(struct udevice *dev)
{
	char ch = 0;
	struct smh_serial_priv *priv = dev_get_priv(dev);

	if (priv->infd < 0)
		return smh_getc();

	smh_read(priv->infd, &ch, sizeof(ch));
	return ch;
}

static int smh_serial_putc(struct udevice *dev, const char ch)
{
	smh_putc(ch);
	return 0;
}

static const struct dm_serial_ops smh_serial_ops = {
	.putc = smh_serial_putc,
	.getc = smh_serial_getc,
};

static int smh_serial_bind(struct udevice *dev)
{
	if (semihosting_enabled())
		return 0;
	return -ENOENT;
}

static int smh_serial_probe(struct udevice *dev)
{
	struct smh_serial_priv *priv = dev_get_priv(dev);

	priv->infd = smh_open(":tt", MODE_READ);
	return 0;
}

U_BOOT_DRIVER(smh_serial) = {
	.name	= "serial_semihosting",
	.id	= UCLASS_SERIAL,
	.bind	= smh_serial_bind,
	.probe	= smh_serial_probe,
	.priv_auto = sizeof(struct smh_serial_priv),
	.ops	= &smh_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

U_BOOT_DRVINFO(smh_serial) = {
	.name = "serial_semihosting",
};
#else /* DM_SERIAL */
static int infd = -ENODEV;
static int outfd = -ENODEV;

static int smh_serial_start(void)
{
	infd = smh_open(":tt", MODE_READ);
	outfd = smh_open(":tt", MODE_WRITE);
	return 0;
}

static int smh_serial_stop(void)
{
	if (outfd >= 0)
		smh_close(outfd);
	return 0;
}

static void smh_serial_setbrg(void)
{
}

static int smh_serial_getc(void)
{
	char ch = 0;

	if (infd < 0)
		return smh_getc();

	smh_read(infd, &ch, sizeof(ch));
	return ch;
}

static int smh_serial_tstc(void)
{
	return 1;
}

static void smh_serial_puts(const char *s)
{
	ulong unused;

	if (outfd < 0)
		smh_puts(s);
	else
		smh_write(outfd, s, strlen(s), &unused);
}

struct serial_device serial_smh_device = {
	.name	= "serial_smh",
	.start	= smh_serial_start,
	.stop	= smh_serial_stop,
	.setbrg	= smh_serial_setbrg,
	.getc	= smh_serial_getc,
	.tstc	= smh_serial_tstc,
	.putc	= smh_putc,
	.puts	= smh_serial_puts,
};

void smh_serial_initialize(void)
{
	if (semihosting_enabled())
		serial_register(&serial_smh_device);
}

__weak struct serial_device *default_serial_console(void)
{
	return &serial_smh_device;
}
#endif

#ifdef CONFIG_DEBUG_UART_SEMIHOSTING
#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
}

static inline void _debug_uart_putc(int c)
{
	smh_putc(c);
}

DEBUG_UART_FUNCS
#endif
