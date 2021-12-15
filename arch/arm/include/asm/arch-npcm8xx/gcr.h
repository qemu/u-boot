/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#ifndef _NPCM_GCR_H_
#define _NPCM_GCR_H_

#define NPCM_GCR_BA		0xF0800000
/* On-Chip ARBEL NPCM8XX VERSIONS */

#define ARBEL_Z1			0x00A35850
#define ARBEL_A1			0x04a35850
#define ARBEL_NPCM845			0x00000000
#define ARBEL_NPCM830			0x00300395
#define ARBEL_NPCM810			0x00000220

/* Function Lock Register 2 (FLOCKR2) */
#define FLOCKR2_MMCRST               12
#define FLOCKR2_MMCRSTLK             13
#define FLOCKR2_G35DA2P              18

/* Power-On Setting Register (PWRON) */
#define PWRON_BSPA			4         /* STRAP5 */
#define PWRON_SECEN			7         /* STRAP8 */

/* Multiple Function Pin Select Register 1 (MFSEL1) */
#define MFSEL1_SIRQSE               31
#define MFSEL1_IOX1SEL              30
#define MFSEL1_HSI2BSEL             29
#define MFSEL1_HSI1BSEL             28
#define MFSEL1_DVH1SEL              27
#define MFSEL1_LPCSEL               26
#define MFSEL1_PECIB                25
#define MFSEL1_GSPISEL              24
#define MFSEL1_SMISEL               22
#define MFSEL1_CLKOSEL              21
#define MFSEL1_DVOSEL               18
#define MFSEL1_KBCICSEL             17
#define MFSEL1_R2MDSEL              16
#define MFSEL1_R2ERRSEL             15
#define MFSEL1_RMII2SEL             14
#define MFSEL1_R1MDSEL              13
#define MFSEL1_R1ERRSEL             12
#define MFSEL1_HSI2ASEL             11
#define MFSEL1_HSI1ASEL             10
#define MFSEL1_BSPSEL               9
#define MFSEL1_SMB2SEL              8
#define MFSEL1_SMB1SEL              7
#define MFSEL1_SMB0SEL              6
#define MFSEL1_HSI2CSEL             5
#define MFSEL1_HSI1CSEL             4
#define MFSEL1_S0CS1SEL             3
#define MFSEL1_SMB5SEL              2
#define MFSEL1_SMB4SEL              1
#define MFSEL1_SMB3SEL              0

/* Multiple Function Pin Select Register 3 (MFSEL3) */
#define MFSEL3_DVODEDLY             27
#define MFSEL3_DDRDVOSEL            26
#define MFSEL3_MMCCDSEL             25
#define MFSEL3_BU1SEL               24
#define MFSEL3_I3C5SEL              22
#define MFSEL3_WDO2SEL              20
#define MFSEL3_WDO1SEL              19
#define MFSEL3_IOXHSEL              18
#define MFSEL3_PCIEPUSE             17
#define MFSEL3_CLKRUNSEL            16
#define MFSEL3_IOX2SEL              14
#define MFSEL3_PSPISEL              13
#define MFSEL3_MMC8SEL              11
#define MFSEL3_MMCSEL               10
#define MFSEL3_RMII1SEL             9
#define MFSEL3_SMB15SEL             8
#define MFSEL3_SMB14SEL             7
#define MFSEL3_SMB13SEL             6
#define MFSEL3_SMB12SEL             5
#define MFSEL3_SPI1SEL              4
#define MFSEL3_FIN1916SELB          3
#define MFSEL3_SMB7SEL              2
#define MFSEL3_SMB6SEL              1
#define MFSEL3_SCISEL               0

/* Multiple Function Pin Select Register 4 (MFSEL4) */
#define MFSEL4_SMB11DDC             29
#define MFSEL4_SXCS1SEL             28
#define MFSEL4_SPXSEL               27
#define MFSEL4_RG2SEL               24
#define MFSEL4_RG2MSEL              23
#define MFSEL4_BU2SELB              22
#define MFSEL4_SG1MSEL              21
#define MFSEL4_SP3QSEL              20
#define MFSEL4_S3CS3SEL             19
#define MFSEL4_S3CS2SEL             18
#define MFSEL4_S3CS1SEL             17
#define MFSEL4_SP3SEL               16
#define MFSEL4_SP0QSEL              15
#define MFSEL4_SMB11SEL             14
#define MFSEL4_SMB10SEL             13
#define MFSEL4_SMB9SEL              12
#define MFSEL4_SMB8SEL              11
#define MFSEL4_DBGTRSEL             10
#define MFSEL4_CKRQSEL              9
#define MFSEL4_ESPISEL              8
#define MFSEL4_MMCRSEL              6
#define MFSEL4_SD1PSEL              5
#define MFSEL4_ROSEL                4
#define MFSEL4_ESPIPMESEL           2
#define MFSEL4_BSPASEL              1
#define MFSEL4_JTAG2SEL             0

/* Multiple Function Pin Select Register 5 (MFSEL5) */
#define MFSEL5_R3OENSEL             14
#define MFSEL5_RMII3SEL             11
#define MFSEL5_R2OENSEL             10
#define MFSEL5_R1OENSEL             9
#define MFSEL5_NSPI1CS3SEL          5
#define MFSEL5_NSPI1CS2SEL          4
#define MFSEL5_SPI1D23SEL           3
#define MFSEL5_NSPI1CS1SEL          0

/* Multiple Function Pin Select Register 6 (MFSEL6) */
#define MFSEL6_GPIO1836SEL          19
#define MFSEL6_FM1SEL               17

/* Multiple Function Pin Select Register 7 (MFSEL7) */
#define MFSEL7_SMB15SELB            27
#define MFSEL7_GPIO1889SEL          25

/* USB PHY1 Control Register (USB1PHYCTL) */
#define USB1PHYCTL_RS				28
/* USB PHY2 Control Register (USB2PHYCTL) */
#define USB2PHYCTL_RS				28
/* USB PHY2 Control Register (USB3PHYCTL) */
#define USB3PHYCTL_RS				28

/* Integration Control Register (INTCR) */
#define  INTCR_DUDKSMOD             30
#define  INTCR_DDC3I                29
#define  INTCR_KVMSI                28
#define  INTCR_DEHS                 27
#define  INTCR_GGPCT2_0             24
#define  INTCR_SGC2                 23
#define  INTCR_DSNS_TRIG            21
#define  INTCR_DAC_SNS              20
#define  INTCR_SGC1                 19
#define  INTCR_LDDRB                18
#define  INTCR_GIRST                17
#define  INTCR_DUDKSEN              16
#define  INTCR_DACOFF               15
#define  INTCR_DACSEL               14
#define  INTCR_GFXINT               12
#define  INTCR_DACOSOVR             10
#define  INTCR_GFXIFDIS             8
#define  INTCR_H2RQDIS              9
#define  INTCR_H2DISPOFF            8
#define  INTCR_GFXINT2              7
#define  INTCR_VGAIOEN              6
#define  INTCR_PSPIFEN              4
#define  INTCR_HIFEN                3
#define  INTCR_SMBFEN               2
#define  INTCR_MFTFEN               1
#define  INTCR_KCSRST_MODE          0

/* Integration Control Register (INTCR2) */
#define  INTCR2_WDC                   21

/* Integration Control Register (INTCR3) */
#define  INTCR3_USBLPBK2              31          /* USB loop-backed HOST 1/2 */
#define  INTCR3_USBLPBK               24          /* USB loop-backed mode on/off */
#define  INTCR3_USBPHY3SW             14          /* 2 bits */
#define  INTCR3_USBPHY2SW             12          /* 2 bits */
#define  INTCR3_USBPPS                6
#define  INTCR3_UHUB_RWUD             5

/* Integration Control Register (INTCR4) */
#define  INTCR4_GMMAP1                24
#define  INTCR4_GMMAP0                16
#define  INTCR4_R3EN                  14
#define  INTCR4_R2EN                  13
#define  INTCR4_R1EN                  12
#define  INTCR4_RGMIIREF              6

/* I2C Segment Pin Select Register (I2CSEGSEL) */
#define I2CSEGSEL_S0DECFG			3
#define I2CSEGSEL_S4DECFG			17

/* I2C Segment Control Register (I2CSEGCTL) */
#define I2CSEGCTL_S0DEN				20
#define I2CSEGCTL_S0DWE				21
#define I2CSEGCTL_S4DEN				24
#define I2CSEGCTL_S4DWE				25
#define I2CSEGCTL_INIT_VAL		0x9333F000

struct npcm_gcr {
	unsigned int  pdid;
	unsigned int  pwron;
	unsigned int  swstrps;
	unsigned int  rsvd1[2];
	unsigned int  miscpe;
	unsigned int  spldcnt;
	unsigned int  rsvd2[1];
	unsigned int  flockr2;
	unsigned int  flockr3;
	unsigned int  rsvd3[3];
	unsigned int  a35_mode;
	unsigned int  spswc;
	unsigned int  intcr;
	unsigned int  intsr;
	unsigned int  obscr1;
	unsigned int  obsdr1;
	unsigned int  rsvd4[1];
	unsigned int  hifcr;
	unsigned int  rsvd5[3];
	unsigned int  intcr2;
	unsigned int  rsvd6[1];
	unsigned int  srcnt;
	unsigned int  ressr;
	unsigned int  rlockr1;
	unsigned int  flockr1;
	unsigned int  dscnt;
	unsigned int  mdlr;
	unsigned int  scrpad_c;
	/* scrpad_b: holds the active dram size (value set by bootblock) */
	unsigned int  scrpad_b;
	unsigned int  rsvd7[4];
	unsigned int  daclvlr;
	unsigned int  intcr3;
	unsigned int  pcirctl;
	unsigned int  rsvd8[2];
	unsigned int  vsintr;
	unsigned int  rsvd9[1];
	unsigned int  sd2sur1;
	unsigned int  sd2sur2;
	unsigned int  sd2irv3;
	unsigned int  intcr4;
	unsigned int  obscr2;
	unsigned int  obsdr2;
	unsigned int  rsvd10[5];
	unsigned int  i2csegsel;
	unsigned int  i2csegctl;
	unsigned int  vsrcr;
	unsigned int  mlockr;
	unsigned int  rsvd11[8];
	unsigned int  etsr;
	unsigned int  dft1r;
	unsigned int  dft2r;
	unsigned int  dft3r;
	unsigned int  edffsr;
	unsigned int  rsvd12[1];
	unsigned int  intcrpce3;
	unsigned int  intcrpce2;
	unsigned int  intcrpce0;
	unsigned int  intcrpce1;
	unsigned int  dactest;
	unsigned int  scrpad;
	unsigned int  usb1phyctl;
	unsigned int  usb2phyctl;
	unsigned int  usb3phyctl;
	unsigned int  intsr2;
	unsigned int  intcrpce2b;
	unsigned int  intcrpce0b;
	unsigned int  intcrpce1b;
	unsigned int  intcrpce3b;
	unsigned int  rsvd13[4];
	unsigned int  intcrpce2c;
	unsigned int  intcrpce0c;
	unsigned int  intcrpce1c;
	unsigned int  intcrpce3c;
	unsigned int  rsvd14[40];
	unsigned int  sd2irv4;
	unsigned int  sd2irv5;
	unsigned int  sd2irv6;
	unsigned int  sd2irv7;
	unsigned int  sd2irv8;
	unsigned int  sd2irv9;
	unsigned int  sd2irv10;
	unsigned int  sd2irv11;
	unsigned int  rsvd15[8];
	unsigned int  mfsel1;
	unsigned int  mfsel2;
	unsigned int  mfsel3;
	unsigned int  mfsel4;
	unsigned int  mfsel5;
	unsigned int  mfsel6;
	unsigned int  mfsel7;
	unsigned int  rsvd16[1];
	unsigned int  mfsel_lk1;
	unsigned int  mfsel_lk2;
	unsigned int  mfsel_lk3;
	unsigned int  mfsel_lk4;
	unsigned int  mfsel_lk5;
	unsigned int  mfsel_lk6;
	unsigned int  mfsel_lk7;
	unsigned int  rsvd17[1];
	unsigned int  mfsel_set1;
	unsigned int  mfsel_set2;
	unsigned int  mfsel_set3;
	unsigned int  mfsel_set4;
	unsigned int  mfsel_set5;
	unsigned int  mfsel_set6;
	unsigned int  mfsel_set7;
	unsigned int  rsvd18[1];
	unsigned int  mfsel_clr1;
	unsigned int  mfsel_clr2;
	unsigned int  mfsel_clr3;
	unsigned int  mfsel_clr4;
	unsigned int  mfsel_clr5;
	unsigned int  mfsel_clr6;
	unsigned int  mfsel_clr7;
	};

#endif
