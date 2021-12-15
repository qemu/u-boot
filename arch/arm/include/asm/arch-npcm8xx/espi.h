/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _NPCM_ESPI_H_
#define _NPCM_ESPI_H_

#define NPCM_ESPI_BA		0xF009F000
/* Register offsets */
#define ESPICFG			0x04
#define ESPIHINDP		0x80

/* Channel Supported */
#define ESPICFG_CHNSUPP_MASK	0x0F
#define ESPICFG_CHNSUPP_SHFT	24

/* I/O Mode Supported */
#define ESPICFG_IOMODE_SHIFT		8
#define ESPI_IO_MODE_SINGLE_DUAL_QUAD	3

/* Maximum Frequency Supported */
#define ESPICFG_MAXFREQ_SHIFT		10
#define ESPI_MAX_33_MHZ			2

#endif
