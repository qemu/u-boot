/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _NPCM_RST_H_
#define _NPCM_RST_H_

enum reset_type {
	PORST_TYPE    = 0x01,
	CORST_TYPE    = 0x02,
	WD0RST_TYPE   = 0x03,
	SWR1ST_TYPE   = 0x04,
	SWR2ST_TYPE   = 0x05,
	SWR3ST_TYPE   = 0x06,
	SWR4ST_TYPE   = 0x07,
	WD1RST_TYPE   = 0x08,
	WD2RST_TYPE   = 0x09,
	UNKNOWN_TYPE  = 0x10,
};

#define PORST 0x80000000
#define CORST 0x40000000
#define WD0RST 0x20000000
#define SWR1ST 0x10000000
#define SWR2ST 0x08000000
#define SWR3ST 0x04000000
#define SWR4ST 0x02000000
#define WD1RST 0x01000000
#define WD2RST 0x00800000
#define RESSR_MASK 0xff800000

enum reset_type npcm8xx_reset_reason(void);

#endif
