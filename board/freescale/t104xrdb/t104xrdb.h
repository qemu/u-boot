/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#ifndef __T104x_RDB_H__
#define __T104x_RDB_H__

#include <asm/u-boot.h>
void fdt_fixup_board_enet(void *blob);
void pci_of_setup(void *blob, struct bd_info *bd);

#endif
