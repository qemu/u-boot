/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#ifndef _NPCM_CLOCK_H_
#define _NPCM_CLOCK_H_

#define NPCM_CLK_BA		0xF0801000
enum {
	APB1  = 1,
	APB2  = 2,
	APB3  = 3,
	APB4  = 4,
	APB5  = 5,
	SPI0  = 10,
	SPI1  = 11,
	SPI3  = 13,
	SPIX  = 14,
};

/* Clock Select Register (CLKSEL) */
#define CLKSEL_RCPCKSEL             27
#define CLKSEL_RGSEL                25
#define CLKSEL_GFXMSEL              21
#define CLKSEL_CLKOUTSEL            18
#define CLKSEL_PCICKSEL             16
#define CLKSEL_ADCCKSEL             14
#define CLKSEL_MCCKSEL              12
#define CLKSEL_SUCKSEL              10
#define CLKSEL_UARTCKSEL            8
#define CLKSEL_SDCKSEL              6
#define CLKSEL_PIXCKSEL             4
#define CLKSEL_CPUCKSEL             0

/* Clock Divider Control Register 1 (CLKDIV1) */
#define CLKDIV1_ADCCKDIV            28
#define CLKDIV1_CLK4DIV             26
#define CLKDIV1_PRE_ADCCKDIV        21
#define CLKDIV1_UARTDIV             16
#define CLKDIV1_MMCCKDIV            11
#define CLKDIV1_SPI3CKDIV           6
#define CLKDIV1_PCICKDIV            2

/* Clock Divider Control Register 2 (CLKDIV2) */
#define CLKDIV2_APB4CKDIV           30
#define CLKDIV2_APB3CKDIV           28
#define CLKDIV2_APB2CKDIV           26
#define CLKDIV2_APB1CKDIV           24
#define CLKDIV2_APB5CKDIV           22
#define CLKDIV2_CLKOUTDIV           16
#define CLKDIV2_GFXCKDIV            13
#define CLKDIV2_SUCKDIV             8
#define CLKDIV2_SU48CKDIV           4

/* PLL Control Register 2 (PLLCON2) */
#define PLLCON_LOKI                31
#define PLLCON_LOKS                30
#define PLLCON_FBDV                16
#define PLLCON_OTDV2               13
#define PLLCON_PWDEN               12
#define PLLCON_OTDV1               8
#define PLLCON_INDV                0

/* CPUCKSEL (CPU/AMBA/MC Clock Source Select Bit) */
#define CLKSEL_CPUCKSEL_PLL0        0x00   /* 0 0: PLL0 clock*/
#define CLKSEL_CPUCKSEL_PLL1        0x01   /* 0 1: PLL1 clock */
#define CLKSEL_CPUCKSEL_CLKREF      0x02   /* 1 0: CLKREF input (25 MHZ, default) */
#define CLKSEL_CPUCKSEL_SYSBPCK     0x03   /* 1 1: Bypass clock from pin SYSBPCK */

/* UARTCKSEL (Core and Host UART Clock Source Select Bit). */
#define CLKSEL_UARTCKSEL_PLL0       0x00  /* 0 0: PLL0    clock. */
#define CLKSEL_UARTCKSEL_PLL1       0x01  /* 0 1: PLL1    clock. */
#define CLKSEL_UARTCKSEL_CLKREF     0x02  /* 1 0: CLKREF  clock (25 MHZ, default). */
#define CLKSEL_UARTCKSEL_PLL2       0x03  /* 1 1: PLL2    clock divided by 2. */

/* SDCKSEL (SDHC Clock Source Select Bit). */
#define CLKSEL_SDCKSEL_PLL0         0x00   /* 0 0: PLL0    clock.  */
#define CLKSEL_SDCKSEL_PLL1         0x01   /* 0 1: PLL1    clock.  */
#define CLKSEL_SDCKSEL_CLKREF       0x02   /* 1 0: CLKREF clock (25 MHZ, default).  */
#define CLKSEL_SDCKSEL_PLL2         0x03   /* 1 1: PLL2    clock divided by 2.  */

/* IP Software Reset Register 1 (IPSRST1), offset 0x20 */
#define IPSRST1_USBDEV1             5
#define IPSRST1_USBDEV2             8
#define IPSRST1_USBDEV3             25
#define IPSRST1_USBDEV4             22
#define IPSRST1_USBDEV5             23
#define IPSRST1_USBDEV6             24
#define IPSRST1_GMAC4               21
#define IPSRST1_GMAC3               6

/* IP Software Reset Register 2 (IPSRST2), offset 0x24 */
#define IPSRST2_GMAC1               28
#define IPSRST2_GMAC2               25
#define IPSRST2_USBHOST1            26
#define IPSRST2_SDHC                9
#define IPSRST2_MMC                 8

/* IP Software Reset Register 3 (IPSRST3), offset 0x34 */
#define IPSRST3_USBPHY1             24
#define IPSRST3_USBPHY2             25
#define IPSRST3_USBHUB              8
#define IPSRST3_USBDEV9             7
#define IPSRST3_USBDEV8             6
#define IPSRST3_USBDEV7             5
#define IPSRST3_USBDEV0             4

/* IP Software Reset Register 4 (IPSRST4), offset 0x74 */
#define IPSRST4_USBHOST2            31
#define IPSRST4_USBPHY3             25

#define EXT_CLOCK_FREQUENCY_KHZ	    25 * 1000 * 1UL
#define EXT_CLOCK_FREQUENCY_MHZ	    25

struct clk_ctl {
	unsigned int  clken1;
	unsigned int  clksel;
	unsigned int  clkdiv1;
	unsigned int  pllcon0;
	unsigned int  pllcon1;
	unsigned int  swrstr;
	unsigned char res1[0x8];
	unsigned int  ipsrst1;
	unsigned int  ipsrst2;
	unsigned int  clken2;
	unsigned int  clkdiv2;
	unsigned int  clken3;
	unsigned int  ipsrst3;
	unsigned int  wd0rcr;
	unsigned int  wd1rcr;
	unsigned int  wd2rcr;
	unsigned int  swrstc1;
	unsigned int  swrstc2;
	unsigned int  swrstc3;
	unsigned int  tiprstc;
	unsigned int  pllcon2;
	unsigned int  clkdiv3;
	unsigned int  corstc;
	unsigned int  pllcong;
	unsigned int  ahbckfi;
	unsigned int  seccnt;
	unsigned int  cntr25m;
	unsigned int  clken4;
	unsigned int  ipsrst4;
	unsigned int  busto;
	unsigned int  clkdiv4;
	unsigned int  wd0rcrb;
	unsigned int  wd1rcrb;
	unsigned int  wd2rcrb;
	unsigned int  swrstc1b;
	unsigned int  swrstc2b;
	unsigned int  swrstc3b;
	unsigned int  tiprstcb;
	unsigned int  corstcb;
	unsigned int  ipsrstdis1;
	unsigned int  ipsrstdis2;
	unsigned int  ipsrstdis3;
	unsigned int  ipsrstdis4;
	unsigned char res2[0x10];
	unsigned int  thrtl_cnt;
};

#endif
