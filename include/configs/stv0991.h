/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ifndef __CONFIG_STV0991_H
#define __CONFIG_STV0991_H
#define CONFIG_SYS_EXCEPTION_VECTORS_HIGH

/* ram memory-related information */
#define PHYS_SDRAM_1				0x00000000
#define CONFIG_SYS_SDRAM_BASE			PHYS_SDRAM_1
#define PHYS_SDRAM_1_SIZE			0x00198000

/* user interface */

/* MISC */
#define CONFIG_SYS_INIT_RAM_SIZE		0x8000
#define CONFIG_SYS_INIT_RAM_ADDR		0x00190000
/* U-Boot Load Address */

/* GMAC related configs */

#define CONFIG_DW_ALTDESCRIPTOR

/* Misc configuration */

#endif /* __CONFIG_H */
