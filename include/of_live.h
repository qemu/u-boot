/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * Support for a 'live' (as opposed to flat) device tree
 */

#ifndef _OF_LIVE_H
#define _OF_LIVE_H

struct device_node;

/**
 * of_live_build() - build a live (hierarchical) tree from a flat DT
 *
 * @fdt_blob: Input tree to convert
 * @rootp: Returns live tree that was created
 * Return: 0 if OK, -ve on error
 */
int of_live_build(const void *fdt_blob, struct device_node **rootp);

/**
 * unflatten_device_tree() - create tree of device_nodes from flat blob
 *
 * Note that this allocates a single block of memory, pointed to by *mynodes.
 * To free the tree, use free(*mynodes)
 *
 * unflattens a device-tree, creating the
 * tree of struct device_node. It also fills the "name" and "type"
 * pointers of the nodes so the normal device-tree walking functions
 * can be used.
 * @blob: The blob to expand
 * @mynodes: The device_node tree created by the call
 * Return: 0 if OK, -ve on error
 */
int unflatten_device_tree(const void *blob, struct device_node **mynodes);

#endif
