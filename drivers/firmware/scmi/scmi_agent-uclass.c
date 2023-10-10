// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Linaro Limited.
 */

#define LOG_CATEGORY UCLASS_SCMI_AGENT

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <scmi_protocols.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <linux/compat.h>

/**
 * struct error_code - Helper structure for SCMI error code conversion
 * @scmi:	SCMI error code
 * @errno:	Related standard error number
 */
struct error_code {
	int scmi;
	int errno;
};

static const struct error_code scmi_linux_errmap[] = {
	{ .scmi = SCMI_NOT_SUPPORTED, .errno = -EOPNOTSUPP, },
	{ .scmi = SCMI_INVALID_PARAMETERS, .errno = -EINVAL, },
	{ .scmi = SCMI_DENIED, .errno = -EACCES, },
	{ .scmi = SCMI_NOT_FOUND, .errno = -ENOENT, },
	{ .scmi = SCMI_OUT_OF_RANGE, .errno = -ERANGE, },
	{ .scmi = SCMI_BUSY, .errno = -EBUSY, },
	{ .scmi = SCMI_COMMS_ERROR, .errno = -ECOMM, },
	{ .scmi = SCMI_GENERIC_ERROR, .errno = -EIO, },
	{ .scmi = SCMI_HARDWARE_ERROR, .errno = -EREMOTEIO, },
	{ .scmi = SCMI_PROTOCOL_ERROR, .errno = -EPROTO, },
};

/*
 * NOTE: The only one instance should exist according to
 * the current specification and device tree bindings.
 */
struct udevice *scmi_agent;

struct udevice *scmi_get_protocol(struct udevice *dev,
				  enum scmi_std_protocol id)
{
	struct scmi_agent_priv *priv;
	struct udevice *proto;

	priv = dev_get_uclass_plat(dev);
	if (!priv) {
		dev_err(dev, "No priv data found\n");
		return NULL;
	}

	switch (id) {
	case SCMI_PROTOCOL_ID_CLOCK:
		proto = priv->clock_dev;
		break;
	case SCMI_PROTOCOL_ID_RESET_DOMAIN:
		proto = priv->resetdom_dev;
		break;
	case SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN:
		proto = priv->voltagedom_dev;
		break;
	default:
		dev_err(dev, "Protocol not supported\n");
		proto = NULL;
		break;
	}
	if (proto && device_probe(proto))
		dev_err(dev, "Probe failed\n");

	return proto;
}

/**
 * scmi_add_protocol - add protocol to agent
 * @dev:	SCMI agent device
 * @proto_id:	SCMI protocol ID
 * @proto:	SCMI protocol device
 *
 * Associate the protocol instance, @proto, to the agent, @dev,
 * for later use.
 *
 * Return:	0 on success, error code on failure
 */
static int scmi_add_protocol(struct udevice *dev,
			     enum scmi_std_protocol proto_id,
			     struct udevice *proto)
{
	struct scmi_agent_priv *priv;

	priv = dev_get_uclass_plat(dev);
	if (!priv) {
		dev_err(dev, "No priv data found\n");
		return -ENODEV;
	}

	switch (proto_id) {
	case SCMI_PROTOCOL_ID_CLOCK:
		priv->clock_dev = proto;
		break;
	case SCMI_PROTOCOL_ID_RESET_DOMAIN:
		priv->resetdom_dev = proto;
		break;
	case SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN:
		priv->voltagedom_dev = proto;
		break;
	default:
		dev_err(dev, "Protocol not supported\n");
		return -EPROTO;
	}

	return 0;
}

int scmi_to_linux_errno(s32 scmi_code)
{
	int n;

	if (!scmi_code)
		return 0;

	for (n = 0; n < ARRAY_SIZE(scmi_linux_errmap); n++)
		if (scmi_code == scmi_linux_errmap[n].scmi)
			return scmi_linux_errmap[n].errno;

	return -EPROTO;
}

static struct udevice *find_scmi_protocol_device(struct udevice *dev)
{
	struct udevice *parent = NULL, *protocol;

	for (protocol = dev; protocol; protocol = parent) {
		parent = dev_get_parent(protocol);
		if (!parent ||
		    device_get_uclass_id(parent) == UCLASS_SCMI_AGENT)
			break;
	}

	if (!parent) {
		dev_err(dev, "Invalid SCMI device, agent not found\n");
		return NULL;
	}

	return protocol;
}

static const struct scmi_agent_ops *transport_dev_ops(struct udevice *dev)
{
	return (const struct scmi_agent_ops *)dev->driver->ops;
}

/**
 * scmi_of_get_channel() - Get SCMI channel handle
 *
 * @dev:	SCMI agent device
 * @channel:	Output reference to the SCMI channel upon success
 *
 * On return, @channel will be set.
 * Return	0 on success and a negative errno on failure
 */
static int scmi_of_get_channel(struct udevice *dev, struct udevice *protocol,
			       struct scmi_channel **channel)
{
	const struct scmi_agent_ops *ops;

	ops = transport_dev_ops(dev);
	if (ops->of_get_channel)
		return ops->of_get_channel(dev, protocol, channel);
	else
		return -EPROTONOSUPPORT;
}

int devm_scmi_of_get_channel(struct udevice *dev)
{
	struct udevice *protocol;
	struct scmi_agent_proto_priv *priv;
	int ret;

	protocol = find_scmi_protocol_device(dev);
	if (!protocol)
		return -ENODEV;

	priv = dev_get_parent_priv(protocol);
	ret = scmi_of_get_channel(protocol->parent, protocol, &priv->channel);
	if (ret == -EPROTONOSUPPORT) {
		/* Drivers without a get_channel operator don't need a channel ref */
		priv->channel = NULL;

		return 0;
	}

	return ret;
}

/**
 * scmi_process_msg() - Send and process an SCMI message
 *
 * Send a message to an SCMI server.
 * Caller sets scmi_msg::out_msg_sz to the output message buffer size.
 *
 * @dev:	SCMI agent device
 * @channel:	Communication channel for the device
 * @msg:	Message structure reference
 *
 * On return, scmi_msg::out_msg_sz stores the response payload size.
 * Return:	0 on success and a negative errno on failure
 */
static int scmi_process_msg(struct udevice *dev, struct scmi_channel *channel,
			    struct scmi_msg *msg)
{
	const struct scmi_agent_ops *ops;

	ops = transport_dev_ops(dev);
	if (ops->process_msg)
		return ops->process_msg(dev, channel, msg);
	else
		return -EPROTONOSUPPORT;
}

int devm_scmi_process_msg(struct udevice *dev, struct scmi_msg *msg)
{
	struct udevice *protocol;
	struct scmi_agent_proto_priv *priv;

	protocol = find_scmi_protocol_device(dev);
	if (!protocol)
		return -ENODEV;

	priv = dev_get_parent_priv(protocol);

	return scmi_process_msg(protocol->parent, priv->channel, msg);
}

/*
 * SCMI agent devices binds devices of various uclasses depending on
 * the FDT description. scmi_bind_protocol() is a generic bind sequence
 * called by the uclass at bind stage, that is uclass post_bind.
 */
static int scmi_bind_protocols(struct udevice *dev)
{
	int ret = 0;
	ofnode node;
	const char *name;
	struct driver *drv;
	struct udevice *proto;

	dev_for_each_subnode(node, dev) {
		u32 protocol_id;

		if (!ofnode_is_enabled(node))
			continue;

		if (ofnode_read_u32(node, "reg", &protocol_id))
			continue;

		drv = NULL;
		name = ofnode_get_name(node);
		switch (protocol_id) {
		case SCMI_PROTOCOL_ID_CLOCK:
			if (CONFIG_IS_ENABLED(CLK_SCMI))
				drv = DM_DRIVER_GET(scmi_clock);
			break;
		case SCMI_PROTOCOL_ID_RESET_DOMAIN:
			if (IS_ENABLED(CONFIG_RESET_SCMI))
				drv = DM_DRIVER_GET(scmi_reset_domain);
			break;
		case SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN:
			if (IS_ENABLED(CONFIG_DM_REGULATOR_SCMI)) {
				node = ofnode_find_subnode(node, "regulators");
				if (!ofnode_valid(node)) {
					dev_err(dev, "no regulators node\n");
					return -ENXIO;
				}
				drv = DM_DRIVER_GET(scmi_voltage_domain);
			}
			break;
		default:
			break;
		}

		if (!drv) {
			dev_dbg(dev, "Ignore unsupported SCMI protocol %#x\n",
				protocol_id);
			continue;
		}

		ret = device_bind(dev, drv, name, NULL, node, &proto);
		if (ret) {
			dev_err(dev, "failed to bind %s protocol\n", drv->name);
			break;
		}
		ret = scmi_add_protocol(dev, protocol_id, proto);
		if (ret) {
			dev_err(dev, "failed to add protocol: %s, ret: %d\n",
				proto->name, ret);
			break;
		}
	}

	if (!ret)
		scmi_agent = dev;

	return ret;
}

UCLASS_DRIVER(scmi_agent) = {
	.id		= UCLASS_SCMI_AGENT,
	.name		= "scmi_agent",
	.post_bind	= scmi_bind_protocols,
	.per_device_plat_auto = sizeof(struct scmi_agent_priv),
	.per_child_auto	= sizeof(struct scmi_agent_proto_priv),
};
