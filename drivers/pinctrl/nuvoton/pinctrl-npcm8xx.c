// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Nuvoton Technology Corp.
 * Author: Joseph Liu <kwliu@nuvoton.com>
 * Author: Tomer Maimon <tomer.maimon@nuvoton.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/gcr.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/rst.h>
#include <dm/pinctrl.h>
#include <dm/device_compat.h>

/* GCR registers */
#define NPCM8XX_GCR_PDID	0x00
#define NPCM8XX_GCR_SRCNT	0x68
#define NPCM8XX_GCR_FLOCKR1	0x74
#define NPCM8XX_GCR_DSCNT	0x78
#define NPCM8XX_GCR_INTCR4	0xC0
#define NPCM8XX_GCR_I2CSEGCTL	0xE4
#define NPCM8XX_GCR_I2CSEGSEL	0xE0
#define NPCM8XX_GCR_MFSEL1	0x260
#define NPCM8XX_GCR_MFSEL2	0x264
#define NPCM8XX_GCR_MFSEL3	0x268
#define NPCM8XX_GCR_MFSEL4	0x26C
#define NPCM8XX_GCR_MFSEL5	0x270
#define NPCM8XX_GCR_MFSEL6	0x274
#define NPCM8XX_GCR_MFSEL7	0x278
#define NPCM8XX_GCR_MFSEL7	0x278

#define SRCNT_ESPI		BIT(3)

/* reset registers */
#define NPCM8XX_RST_WD0RCR	0x38
#define NPCM8XX_RST_WD1RCR	0x3C
#define NPCM8XX_RST_WD2RCR	0x40
#define NPCM8XX_RST_SWRSTC1	0x44
#define NPCM8XX_RST_SWRSTC2	0x48
#define NPCM8XX_RST_SWRSTC3	0x4C
#define NPCM8XX_RST_SWRSTC4	0x50
#define NPCM8XX_RST_CORSTC	0x5C

#define GPIOX_MODULE_RESET	16
#define CA9C_MODULE_RESET	BIT(0)

/* GPIO registers */
#define NPCM8XX_GP_N_TLOCK1	0x00
#define NPCM8XX_GP_N_DIN	0x04 /* Data IN */
#define NPCM8XX_GP_N_POL	0x08 /* Polarity */
#define NPCM8XX_GP_N_DOUT	0x0c /* Data OUT */
#define NPCM8XX_GP_N_OE		0x10 /* Output Enable */
#define NPCM8XX_GP_N_OTYP	0x14
#define NPCM8XX_GP_N_MP		0x18
#define NPCM8XX_GP_N_PU		0x1c /* Pull-up */
#define NPCM8XX_GP_N_PD		0x20 /* Pull-down */
#define NPCM8XX_GP_N_DBNC	0x24 /* Debounce */
#define NPCM8XX_GP_N_EVTYP	0x28 /* Event Type */
#define NPCM8XX_GP_N_EVBE	0x2c /* Event Both Edge */
#define NPCM8XX_GP_N_OBL0	0x30
#define NPCM8XX_GP_N_OBL1	0x34
#define NPCM8XX_GP_N_OBL2	0x38
#define NPCM8XX_GP_N_OBL3	0x3c
#define NPCM8XX_GP_N_EVEN	0x40 /* Event Enable */
#define NPCM8XX_GP_N_EVENS	0x44 /* Event Set (enable) */
#define NPCM8XX_GP_N_EVENC	0x48 /* Event Clear (disable) */
#define NPCM8XX_GP_N_EVST	0x4c /* Event Status */
#define NPCM8XX_GP_N_SPLCK	0x50
#define NPCM8XX_GP_N_MPLCK	0x54
#define NPCM8XX_GP_N_IEM	0x58 /* Input Enable */
#define NPCM8XX_GP_N_OSRC	0x5c
#define NPCM8XX_GP_N_ODSC	0x60
#define NPCM8XX_GP_N_DOS	0x68 /* Data OUT Set */
#define NPCM8XX_GP_N_DOC	0x6c /* Data OUT Clear */
#define NPCM8XX_GP_N_OES	0x70 /* Output Enable Set */
#define NPCM8XX_GP_N_OEC	0x74 /* Output Enable Clear */
#define NPCM8XX_GP_N_DBNCS0	0x80
#define NPCM8XX_GP_N_DBNCS1	0x84
#define NPCM8XX_GP_N_DBNCP0	0x88
#define NPCM8XX_GP_N_DBNCP1	0x8C
#define NPCM8XX_GP_N_DBNCP2	0x90
#define NPCM8XX_GP_N_DBNCP3	0x94
#define NPCM8XX_GP_N_TLOCK2	0xAC

#define NPCM8XX_GPIO_BANK_OFFSET 0x1000
#define NPCM8XX_GPIO_PER_BITS	32
#define NPCM8XX_GPIO_PER_BANK	32
#define NPCM8XX_GPIO_BANK_NUM	8
#define NPCM8XX_GCR_NONE	0
/* not gpio */
static const unsigned int gpi35_pins[] = { 35 };
static const unsigned int gpi36_pins[] = { 36 };

static const unsigned int tp_jtag3_pins[] = { 44, 62, 45, 46 };
static const unsigned int tp_uart_pins[] = { 50, 51 };

static const unsigned int tp_smb2_pins[] = { 24, 25 };
static const unsigned int tp_smb1_pins[] = { 142, 143 };

static const unsigned int tp_gpio7_pins[] = { 96 };
static const unsigned int tp_gpio6_pins[] = { 97 };
static const unsigned int tp_gpio5_pins[] = { 98 };
static const unsigned int tp_gpio4_pins[] = { 99 };
static const unsigned int tp_gpio3_pins[] = { 100 };
static const unsigned int tp_gpio2_pins[] = { 16 };
static const unsigned int tp_gpio1_pins[] = { 9 };
static const unsigned int tp_gpio0_pins[] = { 8 };

static const unsigned int tp_gpio2b_pins[] = { 101 };
static const unsigned int tp_gpio1b_pins[] = { 92 };
static const unsigned int tp_gpio0b_pins[] = { 91 };

static const unsigned int vgadig_pins[] = { 102, 103, 104, 105 };

static const unsigned int nbu1crts_pins[] = { 44, 62 };

static const unsigned int fm2_pins[] = { 224, 225, 226, 227, 228, 229, 230 };
static const unsigned int fm1_pins[] = { 175, 176, 177, 203, 191, 192, 233 };
static const unsigned int fm0_pins[] = { 194, 195, 196, 202, 199, 198, 197 };

static const unsigned int gpio1836_pins[] = { 183, 184, 185, 186 };
static const unsigned int gpio1889_pins[] = { 188, 189 };
static const unsigned int gpio187_pins[] = { 187 };

static const unsigned int cp1urxd_pins[] = { 41 };
static const unsigned int r3rxer_pins[] = { 212 };

static const unsigned int cp1gpio2c_pins[] = { 101 };
static const unsigned int cp1gpio3c_pins[] = { 100 };

static const unsigned int cp1gpio0b_pins[] = { 127 };
static const unsigned int cp1gpio1b_pins[] = { 126 };
static const unsigned int cp1gpio2b_pins[] = { 125 };
static const unsigned int cp1gpio3b_pins[] = { 124 };
static const unsigned int cp1gpio4b_pins[] = { 99 };
static const unsigned int cp1gpio5b_pins[] = { 98 };
static const unsigned int cp1gpio6b_pins[] = { 97 };
static const unsigned int cp1gpio7b_pins[] = { 96 };

static const unsigned int cp1gpio0_pins[] = {  };
static const unsigned int cp1gpio1_pins[] = {  };
static const unsigned int cp1gpio2_pins[] = {  };
static const unsigned int cp1gpio3_pins[] = {  };
static const unsigned int cp1gpio4_pins[] = {  };
static const unsigned int cp1gpio5_pins[] = { 17 };
static const unsigned int cp1gpio6_pins[] = { 91 };
static const unsigned int cp1gpio7_pins[] = { 92 };

static const unsigned int cp1utxd_pins[] = { 42 };

static const unsigned int j2j3_pins[] = { 44, 62, 45, 46 };

static const unsigned int r3oen_pins[] = { 58 };
static const unsigned int r2oen_pins[] = { 90, 249 };
static const unsigned int r1oen_pins[] = { 56 };
static const unsigned int bu4b_pins[] = { 98, 99 };
static const unsigned int bu4_pins[] = { 54, 55 };
static const unsigned int bu5b_pins[] = { 100, 101 };
static const unsigned int bu5_pins[] = { 52, 53 };
static const unsigned int bu6_pins[] = { 50, 51 };
static const unsigned int rmii3_pins[] = { 110, 111, 209, 212, 211, 210, 214, 215 };

static const unsigned int jm1_pins[] = { 136, 137, 138, 139, 140 };
static const unsigned int jm2_pins[] = { 0, 1, 2, 3 };

static const unsigned int tpgpio5b_pins[] = { 58 };
static const unsigned int tpgpio4b_pins[] = { 57 };

static const unsigned int clkrun_pins[] = { 162 };

static const unsigned int i3c5_pins[] = { 106, 107 };
static const unsigned int i3c4_pins[] = { 33, 34 };
static const unsigned int i3c3_pins[] = { 246, 247 };
static const unsigned int i3c2_pins[] = { 244, 245 };
static const unsigned int i3c1_pins[] = { 242, 243 };
static const unsigned int i3c0_pins[] = { 240, 241 };

static const unsigned int hsi1a_pins[] = { 43, 63 };
static const unsigned int hsi2a_pins[] = { 48, 49 };

static const unsigned int hsi1b_pins[] = { 44, 62 };
static const unsigned int hsi2b_pins[] = { 50, 51 };

static const unsigned int hsi1c_pins[] = { 45, 46, 47, 61 };
static const unsigned int hsi2c_pins[] = { 45, 46, 47, 61 };

/* pinmux handing in the pinctrl driver*/
static const unsigned int smb0_pins[]  = { 115, 114 };
static const unsigned int smb0b_pins[] = { 195, 194 };
static const unsigned int smb0c_pins[] = { 202, 196 };
static const unsigned int smb0d_pins[] = { 198, 199 };
static const unsigned int smb0den_pins[] = { 197 };

static const unsigned int smb1_pins[]  = { 117, 116 };
static const unsigned int smb1b_pins[] = { 126, 127 };
static const unsigned int smb1c_pins[] = { 124, 125 };
static const unsigned int smb1d_pins[] = { 4, 5 };

static const unsigned int smb2_pins[]  = { 119, 118 };
static const unsigned int smb2b_pins[] = { 122, 123 };
static const unsigned int smb2c_pins[] = { 120, 121 };
static const unsigned int smb2d_pins[] = { 6, 7 };

static const unsigned int smb3_pins[]  = { 30, 31 };
static const unsigned int smb3b_pins[] = { 39, 40 };
static const unsigned int smb3c_pins[] = { 37, 38 };
static const unsigned int smb3d_pins[] = { 59, 60 };

static const unsigned int smb4_pins[]  = { 28, 29 };
static const unsigned int smb4b_pins[] = { 18, 19 };
static const unsigned int smb4c_pins[] = { 20, 21 };
static const unsigned int smb4d_pins[] = { 22, 23 };
static const unsigned int smb4den_pins[] = { 17 };

static const unsigned int smb5_pins[]  = { 26, 27 };
static const unsigned int smb5b_pins[] = { 13, 12 };
static const unsigned int smb5c_pins[] = { 15, 14 };
static const unsigned int smb5d_pins[] = { 94, 93 };
static const unsigned int ga20kbc_pins[] = { 94, 93 };

static const unsigned int smb6_pins[]  = { 172, 171 };
static const unsigned int smb6b_pins[] = { 2, 3 };
static const unsigned int smb6c_pins[]  = { 0, 1 };
static const unsigned int smb6d_pins[]  = { 10, 11 };
static const unsigned int smb7_pins[]  = { 174, 173 };
static const unsigned int smb7b_pins[]  = { 16, 141 };
static const unsigned int smb7c_pins[]  = { 24, 25 };
static const unsigned int smb7d_pins[]  = { 142, 143 };
static const unsigned int smb8_pins[]  = { 129, 128 };
static const unsigned int smb9_pins[]  = { 131, 130 };
static const unsigned int smb10_pins[] = { 133, 132 };
static const unsigned int smb11_pins[] = { 135, 134 };
static const unsigned int smb12_pins[] = { 221, 220 };
static const unsigned int smb13_pins[] = { 223, 222 };
static const unsigned int smb14_pins[] = { 22, 23 };
static const unsigned int smb14b_pins[] = { 32, 187 };
static const unsigned int smb15_pins[] = { 20, 21 };
static const unsigned int smb15b_pins[] = { 192, 191 };

static const unsigned int smb16_pins[] = { 10, 11 };
static const unsigned int smb16b_pins[] = { 218, 219 };
static const unsigned int smb17_pins[] = { 3, 2 };
static const unsigned int smb18_pins[] = { 0, 1 };
static const unsigned int smb19_pins[] = { 60, 59 };
static const unsigned int smb20_pins[] = { 234, 235 };
static const unsigned int smb21_pins[] = { 169, 170 };
static const unsigned int smb22_pins[] = { 40, 39 };
static const unsigned int smb23_pins[] = { 38, 37 };
static const unsigned int smb23b_pins[] = { 134, 134 };

static const unsigned int fanin0_pins[] = { 64 };
static const unsigned int fanin1_pins[] = { 65 };
static const unsigned int fanin2_pins[] = { 66 };
static const unsigned int fanin3_pins[] = { 67 };
static const unsigned int fanin4_pins[] = { 68 };
static const unsigned int fanin5_pins[] = { 69 };
static const unsigned int fanin6_pins[] = { 70 };
static const unsigned int fanin7_pins[] = { 71 };
static const unsigned int fanin8_pins[] = { 72 };
static const unsigned int fanin9_pins[] = { 73 };
static const unsigned int fanin10_pins[] = { 74 };
static const unsigned int fanin11_pins[] = { 75 };
static const unsigned int fanin12_pins[] = { 76 };
static const unsigned int fanin13_pins[] = { 77 };
static const unsigned int fanin14_pins[] = { 78 };
static const unsigned int fanin15_pins[] = { 79 };
static const unsigned int faninx_pins[] = { 175, 176, 177, 203 };

static const unsigned int pwm0_pins[] = { 80 };
static const unsigned int pwm1_pins[] = { 81 };
static const unsigned int pwm2_pins[] = { 82 };
static const unsigned int pwm3_pins[] = { 83 };
static const unsigned int pwm4_pins[] = { 144 };
static const unsigned int pwm5_pins[] = { 145 };
static const unsigned int pwm6_pins[] = { 146 };
static const unsigned int pwm7_pins[] = { 147 };
static const unsigned int pwm8_pins[] = { 220 };
static const unsigned int pwm9_pins[] = { 221 };
static const unsigned int pwm10_pins[] = { 234 };
static const unsigned int pwm11_pins[] = { 235 };

static const unsigned int uart1_pins[] = { 43, 44, 45, 46, 47, 61, 62, 63 };
static const unsigned int uart2_pins[] = { 48, 49, 50, 51, 52, 53, 54, 55 };

static const unsigned int bu2_pins[] = { 96, 97};

/* RGMII 1 MD interface pin group */
static const unsigned int sg1mdio_pins[] = { 108, 109 };

/* RGMII 2 pin group */
static const unsigned int rg2_pins[] = { 110, 111, 112, 113, 208, 209, 210, 211, 212,
	213, 214, 215 };
/* RGMII 2 refck pin group */
static const unsigned int rg2refck_pins[] = { 250 };

/* RGMII 2 MD interface pin group */
static const unsigned int rg2mdio_pins[] = { 216, 217 };

static const unsigned int ddr_pins[] = { 110, 111, 112, 113, 208, 209, 210, 211, 212,
	213, 214, 215, 216, 217, 250 };
/* Serial I/O Expander 1 */
static const unsigned int iox1_pins[] = { 0, 1, 2, 3 };
/* Serial I/O Expander 2 */
static const unsigned int iox2_pins[] = { 4, 5, 6, 7 };
/* Host Serial I/O Expander 2 */
static const unsigned int ioxh_pins[] = { 10, 11, 24, 25 };

static const unsigned int mmc_pins[] = { 152, 154, 156, 157, 158, 159 };
static const unsigned int mmcwp_pins[] = { 153 };
static const unsigned int mmccd_pins[] = { 155 };
static const unsigned int mmcrst_pins[] = { 155 };
static const unsigned int mmc8_pins[] = { 148, 149, 150, 151 };

/* RMII 1 pin groups */
static const unsigned int r1_pins[] = { 178, 179, 180, 181, 182, 193, 201 };
static const unsigned int r1err_pins[] = { 56 };
static const unsigned int r1md_pins[] = { 57, 58 };

/* RMII 2 pin groups */
static const unsigned int r2_pins[] = { 84, 85, 86, 87, 88, 89, 200 };
static const unsigned int r2err_pins[] = { 90 };
static const unsigned int r2md_pins[] = { 91, 92 };

static const unsigned int sd1_pins[] = { 136, 137, 138, 139, 140, 141, 142, 143 };
static const unsigned int sd1pwr_pins[] = { 143 };

static const unsigned int wdog1_pins[] = { 218 };
static const unsigned int wdog2_pins[] = { 219 };

/* BMC serial port 0 */
static const unsigned int bmcuart0a_pins[] = { 41, 42 };
static const unsigned int bmcuart0b_pins[] = { 48, 49 };

static const unsigned int bmcuart1_pins[] = { 43, 63 };

/* System Control Interrupt and Power Management Event pin group */
static const unsigned int scipme_pins[] = { 169 };
/* System Management Interrupt pin group */
static const unsigned int sci_pins[] = { 170 };
/* Serial Interrupt Line pin group */
static const unsigned int serirq_pins[] = { 168 };

static const unsigned int clkout_pins[] = { 160 };
static const unsigned int clkreq_pins[] = { 231 };

static const unsigned int jtag2_pins[] = { 43, 44, 45, 46, 47 };
/* Graphics SPI Clock pin group */
static const unsigned int gspi_pins[] = { 12, 13, 14, 15 };

static const unsigned int spix_pins[] = { 224, 225, 226, 227, 229, 230 };
static const unsigned int spixcs1_pins[] = { 228 };

static const unsigned int pspi_pins[] = { 17, 18, 19 };

static const unsigned int spi0cs1_pins[] = { 32 };

static const unsigned int spi1_pins[] = { 175, 176, 177, 203 };
static const unsigned int spi1cs1_pins[] = { 233 };
static const unsigned int spi1d23_pins[] = { 191, 192 };
static const unsigned int spi1cs2_pins[] = { 191 };
static const unsigned int spi1cs3_pins[] = { 192 };

static const unsigned int spi3_pins[] = { 183, 184, 185, 186 };
static const unsigned int spi3cs1_pins[] = { 187 };
static const unsigned int spi3quad_pins[] = { 188, 189 };
static const unsigned int spi3cs2_pins[] = { 188 };
static const unsigned int spi3cs3_pins[] = { 189 };

static const unsigned int ddc_pins[] = { 204, 205, 206, 207 };

static const unsigned int lpc_pins[] = { 95, 161, 163, 164, 165, 166, 167 };
static const unsigned int lpcclk_pins[] = { 168 };
static const unsigned int espi_pins[] = { 95, 161, 163, 164, 165, 166, 167, 168 };

static const unsigned int lkgpo0_pins[] = { 16 };
static const unsigned int lkgpo1_pins[] = { 8 };
static const unsigned int lkgpo2_pins[] = { 9 };

static const unsigned int nprd_smi_pins[] = { 190 };

static const unsigned int hgpio0_pins[] = { 20 };
static const unsigned int hgpio1_pins[] = { 21 };
static const unsigned int hgpio2_pins[] = { 22 };
static const unsigned int hgpio3_pins[] = { 23 };
static const unsigned int hgpio4_pins[] = { 24 };
static const unsigned int hgpio5_pins[] = { 25 };
static const unsigned int hgpio6_pins[] = { 59 };
static const unsigned int hgpio7_pins[] = { 60 };

/*
 * pin:	     name, number
 * group:    name, npins,   pins
 * function: name, ngroups, groups
 */
struct npcm8xx_group {
	const char *name;
	const unsigned int *pins;
	unsigned int npins;
};

#define NPCM8XX_GRPS \
	NPCM8XX_GRP(gpi35), \
	NPCM8XX_GRP(gpi36), \
	NPCM8XX_GRP(tp_jtag3), \
	NPCM8XX_GRP(tp_uart), \
	NPCM8XX_GRP(tp_smb2), \
	NPCM8XX_GRP(tp_smb1), \
	NPCM8XX_GRP(tp_gpio7), \
	NPCM8XX_GRP(tp_gpio6), \
	NPCM8XX_GRP(tp_gpio5), \
	NPCM8XX_GRP(tp_gpio4), \
	NPCM8XX_GRP(tp_gpio3), \
	NPCM8XX_GRP(tp_gpio2), \
	NPCM8XX_GRP(tp_gpio1), \
	NPCM8XX_GRP(tp_gpio0), \
	NPCM8XX_GRP(tp_gpio2b), \
	NPCM8XX_GRP(tp_gpio1b), \
	NPCM8XX_GRP(tp_gpio0b), \
	NPCM8XX_GRP(vgadig), \
	NPCM8XX_GRP(nbu1crts), \
	NPCM8XX_GRP(fm2), \
	NPCM8XX_GRP(fm1), \
	NPCM8XX_GRP(fm0), \
	NPCM8XX_GRP(gpio1836), \
	NPCM8XX_GRP(gpio1889), \
	NPCM8XX_GRP(gpio187), \
	NPCM8XX_GRP(cp1urxd), \
	NPCM8XX_GRP(r3rxer), \
	NPCM8XX_GRP(cp1gpio2c), \
	NPCM8XX_GRP(cp1gpio3c), \
	NPCM8XX_GRP(cp1gpio0b), \
	NPCM8XX_GRP(cp1gpio1b), \
	NPCM8XX_GRP(cp1gpio2b), \
	NPCM8XX_GRP(cp1gpio3b), \
	NPCM8XX_GRP(cp1gpio4b), \
	NPCM8XX_GRP(cp1gpio5b), \
	NPCM8XX_GRP(cp1gpio6b), \
	NPCM8XX_GRP(cp1gpio7b), \
	NPCM8XX_GRP(cp1gpio0), \
	NPCM8XX_GRP(cp1gpio1), \
	NPCM8XX_GRP(cp1gpio2), \
	NPCM8XX_GRP(cp1gpio3), \
	NPCM8XX_GRP(cp1gpio4), \
	NPCM8XX_GRP(cp1gpio5), \
	NPCM8XX_GRP(cp1gpio6), \
	NPCM8XX_GRP(cp1gpio7), \
	NPCM8XX_GRP(cp1utxd), \
	NPCM8XX_GRP(spi1cs3), \
	NPCM8XX_GRP(spi1cs2), \
	NPCM8XX_GRP(spi1cs1), \
	NPCM8XX_GRP(spi1), \
	NPCM8XX_GRP(spi1d23), \
	NPCM8XX_GRP(j2j3), \
	NPCM8XX_GRP(r3oen), \
	NPCM8XX_GRP(r2oen), \
	NPCM8XX_GRP(r1oen), \
	NPCM8XX_GRP(bu4b), \
	NPCM8XX_GRP(bu4), \
	NPCM8XX_GRP(bu5b), \
	NPCM8XX_GRP(bu5), \
	NPCM8XX_GRP(bu6), \
	NPCM8XX_GRP(rmii3), \
	NPCM8XX_GRP(jm1), \
	NPCM8XX_GRP(jm2), \
	NPCM8XX_GRP(tpgpio5b), \
	NPCM8XX_GRP(tpgpio4b), \
	NPCM8XX_GRP(clkrun), \
	NPCM8XX_GRP(i3c5), \
	NPCM8XX_GRP(i3c4), \
	NPCM8XX_GRP(i3c3), \
	NPCM8XX_GRP(i3c2), \
	NPCM8XX_GRP(i3c1), \
	NPCM8XX_GRP(i3c0), \
	NPCM8XX_GRP(hsi1a), \
	NPCM8XX_GRP(hsi2a), \
	NPCM8XX_GRP(hsi1b), \
	NPCM8XX_GRP(hsi2b), \
	NPCM8XX_GRP(hsi1c), \
	NPCM8XX_GRP(hsi2c), \
	NPCM8XX_GRP(smb0), \
	NPCM8XX_GRP(smb0b), \
	NPCM8XX_GRP(smb0c), \
	NPCM8XX_GRP(smb0d), \
	NPCM8XX_GRP(smb0den), \
	NPCM8XX_GRP(smb1), \
	NPCM8XX_GRP(smb1b), \
	NPCM8XX_GRP(smb1c), \
	NPCM8XX_GRP(smb1d), \
	NPCM8XX_GRP(smb2), \
	NPCM8XX_GRP(smb2b), \
	NPCM8XX_GRP(smb2c), \
	NPCM8XX_GRP(smb2d), \
	NPCM8XX_GRP(smb3), \
	NPCM8XX_GRP(smb3b), \
	NPCM8XX_GRP(smb3c), \
	NPCM8XX_GRP(smb3d), \
	NPCM8XX_GRP(smb4), \
	NPCM8XX_GRP(smb4b), \
	NPCM8XX_GRP(smb4c), \
	NPCM8XX_GRP(smb4d), \
	NPCM8XX_GRP(smb4den), \
	NPCM8XX_GRP(smb5), \
	NPCM8XX_GRP(smb5b), \
	NPCM8XX_GRP(smb5c), \
	NPCM8XX_GRP(smb5d), \
	NPCM8XX_GRP(ga20kbc), \
	NPCM8XX_GRP(smb6), \
	NPCM8XX_GRP(smb6b), \
	NPCM8XX_GRP(smb6c), \
	NPCM8XX_GRP(smb6d), \
	NPCM8XX_GRP(smb7), \
	NPCM8XX_GRP(smb7b), \
	NPCM8XX_GRP(smb7c), \
	NPCM8XX_GRP(smb7d), \
	NPCM8XX_GRP(smb8), \
	NPCM8XX_GRP(smb9), \
	NPCM8XX_GRP(smb10), \
	NPCM8XX_GRP(smb11), \
	NPCM8XX_GRP(smb12), \
	NPCM8XX_GRP(smb13), \
	NPCM8XX_GRP(smb14), \
	NPCM8XX_GRP(smb14b), \
	NPCM8XX_GRP(smb15), \
	NPCM8XX_GRP(smb15b), \
	NPCM8XX_GRP(smb16), \
	NPCM8XX_GRP(smb16b), \
	NPCM8XX_GRP(smb17), \
	NPCM8XX_GRP(smb18), \
	NPCM8XX_GRP(smb19), \
	NPCM8XX_GRP(smb20), \
	NPCM8XX_GRP(smb21), \
	NPCM8XX_GRP(smb22), \
	NPCM8XX_GRP(smb23), \
	NPCM8XX_GRP(smb23b), \
	NPCM8XX_GRP(fanin0), \
	NPCM8XX_GRP(fanin1), \
	NPCM8XX_GRP(fanin2), \
	NPCM8XX_GRP(fanin3), \
	NPCM8XX_GRP(fanin4), \
	NPCM8XX_GRP(fanin5), \
	NPCM8XX_GRP(fanin6), \
	NPCM8XX_GRP(fanin7), \
	NPCM8XX_GRP(fanin8), \
	NPCM8XX_GRP(fanin9), \
	NPCM8XX_GRP(fanin10), \
	NPCM8XX_GRP(fanin11), \
	NPCM8XX_GRP(fanin12), \
	NPCM8XX_GRP(fanin13), \
	NPCM8XX_GRP(fanin14), \
	NPCM8XX_GRP(fanin15), \
	NPCM8XX_GRP(faninx), \
	NPCM8XX_GRP(pwm0), \
	NPCM8XX_GRP(pwm1), \
	NPCM8XX_GRP(pwm2), \
	NPCM8XX_GRP(pwm3), \
	NPCM8XX_GRP(pwm4), \
	NPCM8XX_GRP(pwm5), \
	NPCM8XX_GRP(pwm6), \
	NPCM8XX_GRP(pwm7), \
	NPCM8XX_GRP(pwm8), \
	NPCM8XX_GRP(pwm9), \
	NPCM8XX_GRP(pwm10), \
	NPCM8XX_GRP(pwm11), \
	NPCM8XX_GRP(bu2), \
	NPCM8XX_GRP(sg1mdio), \
	NPCM8XX_GRP(rg2), \
	NPCM8XX_GRP(rg2refck), \
	NPCM8XX_GRP(rg2mdio), \
	NPCM8XX_GRP(ddr), \
	NPCM8XX_GRP(uart1), \
	NPCM8XX_GRP(uart2), \
	NPCM8XX_GRP(bmcuart0a), \
	NPCM8XX_GRP(bmcuart0b), \
	NPCM8XX_GRP(bmcuart1), \
	NPCM8XX_GRP(iox1), \
	NPCM8XX_GRP(iox2), \
	NPCM8XX_GRP(ioxh), \
	NPCM8XX_GRP(gspi), \
	NPCM8XX_GRP(mmc), \
	NPCM8XX_GRP(mmcwp), \
	NPCM8XX_GRP(mmccd), \
	NPCM8XX_GRP(mmcrst), \
	NPCM8XX_GRP(mmc8), \
	NPCM8XX_GRP(r1), \
	NPCM8XX_GRP(r1err), \
	NPCM8XX_GRP(r1md), \
	NPCM8XX_GRP(r2), \
	NPCM8XX_GRP(r2err), \
	NPCM8XX_GRP(r2md), \
	NPCM8XX_GRP(sd1), \
	NPCM8XX_GRP(sd1pwr), \
	NPCM8XX_GRP(wdog1), \
	NPCM8XX_GRP(wdog2), \
	NPCM8XX_GRP(scipme), \
	NPCM8XX_GRP(sci), \
	NPCM8XX_GRP(serirq), \
	NPCM8XX_GRP(jtag2), \
	NPCM8XX_GRP(spix), \
	NPCM8XX_GRP(spixcs1), \
	NPCM8XX_GRP(pspi), \
	NPCM8XX_GRP(ddc), \
	NPCM8XX_GRP(clkreq), \
	NPCM8XX_GRP(clkout), \
	NPCM8XX_GRP(spi3), \
	NPCM8XX_GRP(spi3cs1), \
	NPCM8XX_GRP(spi3quad), \
	NPCM8XX_GRP(spi3cs2), \
	NPCM8XX_GRP(spi3cs3), \
	NPCM8XX_GRP(spi0cs1), \
	NPCM8XX_GRP(lpc), \
	NPCM8XX_GRP(lpcclk), \
	NPCM8XX_GRP(espi), \
	NPCM8XX_GRP(lkgpo0), \
	NPCM8XX_GRP(lkgpo1), \
	NPCM8XX_GRP(lkgpo2), \
	NPCM8XX_GRP(nprd_smi), \
	NPCM8XX_GRP(hgpio0), \
	NPCM8XX_GRP(hgpio1), \
	NPCM8XX_GRP(hgpio2), \
	NPCM8XX_GRP(hgpio3), \
	NPCM8XX_GRP(hgpio4), \
	NPCM8XX_GRP(hgpio5), \
	NPCM8XX_GRP(hgpio6), \
	NPCM8XX_GRP(hgpio7), \
	\

enum {
#define NPCM8XX_GRP(x) fn_ ## x
	NPCM8XX_GRPS
	/* add placeholder for none/gpio */
	NPCM8XX_GRP(none),
	NPCM8XX_GRP(gpio),
#undef NPCM8XX_GRP
};

static struct npcm8xx_group npcm8xx_groups[] = {
#define NPCM8XX_GRP(x) { .name = #x, .pins = x ## _pins, \
			.npins = ARRAY_SIZE(x ## _pins) }
	NPCM8XX_GRPS
#undef NPCM8XX_GRP
};

#define NPCM8XX_SFUNC(a) NPCM8XX_FUNC(a, #a)
#define NPCM8XX_FUNC(a, b...) static const char *a ## _grp[] = { b }
#define NPCM8XX_MKFUNC(nm) { .name = #nm, .ngroups = ARRAY_SIZE(nm ## _grp), \
			.groups = nm ## _grp }
struct npcm8xx_func {
	const char *name;
	const unsigned int ngroups;
	const char *const *groups;
};

NPCM8XX_SFUNC(gpi35);
NPCM8XX_SFUNC(gpi36);
NPCM8XX_SFUNC(tp_jtag3);
NPCM8XX_SFUNC(tp_uart);
NPCM8XX_SFUNC(tp_smb2);
NPCM8XX_SFUNC(tp_smb1);
NPCM8XX_SFUNC(tp_gpio7);
NPCM8XX_SFUNC(tp_gpio6);
NPCM8XX_SFUNC(tp_gpio5);
NPCM8XX_SFUNC(tp_gpio4);
NPCM8XX_SFUNC(tp_gpio3);
NPCM8XX_SFUNC(tp_gpio2);
NPCM8XX_SFUNC(tp_gpio1);
NPCM8XX_SFUNC(tp_gpio0);
NPCM8XX_SFUNC(tp_gpio2b);
NPCM8XX_SFUNC(tp_gpio1b);
NPCM8XX_SFUNC(tp_gpio0b);
NPCM8XX_SFUNC(vgadig);
NPCM8XX_SFUNC(nbu1crts);
NPCM8XX_SFUNC(fm2);
NPCM8XX_SFUNC(fm1);
NPCM8XX_SFUNC(fm0);
NPCM8XX_SFUNC(gpio1836);
NPCM8XX_SFUNC(gpio1889);
NPCM8XX_SFUNC(gpio187);
NPCM8XX_SFUNC(cp1urxd);
NPCM8XX_SFUNC(r3rxer);
NPCM8XX_SFUNC(cp1gpio2c);
NPCM8XX_SFUNC(cp1gpio3c);
NPCM8XX_SFUNC(cp1gpio0b);
NPCM8XX_SFUNC(cp1gpio1b);
NPCM8XX_SFUNC(cp1gpio2b);
NPCM8XX_SFUNC(cp1gpio3b);
NPCM8XX_SFUNC(cp1gpio4b);
NPCM8XX_SFUNC(cp1gpio5b);
NPCM8XX_SFUNC(cp1gpio6b);
NPCM8XX_SFUNC(cp1gpio7b);
NPCM8XX_SFUNC(cp1gpio0);
NPCM8XX_SFUNC(cp1gpio1);
NPCM8XX_SFUNC(cp1gpio2);
NPCM8XX_SFUNC(cp1gpio3);
NPCM8XX_SFUNC(cp1gpio4);
NPCM8XX_SFUNC(cp1gpio5);
NPCM8XX_SFUNC(cp1gpio6);
NPCM8XX_SFUNC(cp1gpio7);
NPCM8XX_SFUNC(cp1utxd);
NPCM8XX_SFUNC(spi1cs3);
NPCM8XX_SFUNC(spi1cs2);
NPCM8XX_SFUNC(spi1cs1);
NPCM8XX_SFUNC(spi1);
NPCM8XX_SFUNC(spi1d23);
NPCM8XX_SFUNC(j2j3);
NPCM8XX_SFUNC(r3oen);
NPCM8XX_SFUNC(r2oen);
NPCM8XX_SFUNC(r1oen);
NPCM8XX_SFUNC(bu4b);
NPCM8XX_SFUNC(bu4);
NPCM8XX_SFUNC(bu5b);
NPCM8XX_SFUNC(bu5);
NPCM8XX_SFUNC(bu6);
NPCM8XX_SFUNC(rmii3);
NPCM8XX_SFUNC(jm1);
NPCM8XX_SFUNC(jm2);
NPCM8XX_SFUNC(tpgpio5b);
NPCM8XX_SFUNC(tpgpio4b);
NPCM8XX_SFUNC(clkrun);
NPCM8XX_SFUNC(i3c5);
NPCM8XX_SFUNC(i3c4);
NPCM8XX_SFUNC(i3c3);
NPCM8XX_SFUNC(i3c2);
NPCM8XX_SFUNC(i3c1);
NPCM8XX_SFUNC(i3c0);
NPCM8XX_SFUNC(hsi1a);
NPCM8XX_SFUNC(hsi2a);
NPCM8XX_SFUNC(hsi1b);
NPCM8XX_SFUNC(hsi2b);
NPCM8XX_SFUNC(hsi1c);
NPCM8XX_SFUNC(hsi2c);
NPCM8XX_SFUNC(smb0);
NPCM8XX_SFUNC(smb0b);
NPCM8XX_SFUNC(smb0c);
NPCM8XX_SFUNC(smb0d);
NPCM8XX_SFUNC(smb0den);
NPCM8XX_SFUNC(smb1);
NPCM8XX_SFUNC(smb1b);
NPCM8XX_SFUNC(smb1c);
NPCM8XX_SFUNC(smb1d);
NPCM8XX_SFUNC(smb2);
NPCM8XX_SFUNC(smb2b);
NPCM8XX_SFUNC(smb2c);
NPCM8XX_SFUNC(smb2d);
NPCM8XX_SFUNC(smb3);
NPCM8XX_SFUNC(smb3b);
NPCM8XX_SFUNC(smb3c);
NPCM8XX_SFUNC(smb3d);
NPCM8XX_SFUNC(smb4);
NPCM8XX_SFUNC(smb4b);
NPCM8XX_SFUNC(smb4c);
NPCM8XX_SFUNC(smb4d);
NPCM8XX_SFUNC(smb4den);
NPCM8XX_SFUNC(smb5);
NPCM8XX_SFUNC(smb5b);
NPCM8XX_SFUNC(smb5c);
NPCM8XX_SFUNC(smb5d);
NPCM8XX_SFUNC(ga20kbc);
NPCM8XX_SFUNC(smb6);
NPCM8XX_SFUNC(smb6b);
NPCM8XX_SFUNC(smb6c);
NPCM8XX_SFUNC(smb6d);
NPCM8XX_SFUNC(smb7);
NPCM8XX_SFUNC(smb7b);
NPCM8XX_SFUNC(smb7c);
NPCM8XX_SFUNC(smb7d);
NPCM8XX_SFUNC(smb8);
NPCM8XX_SFUNC(smb9);
NPCM8XX_SFUNC(smb10);
NPCM8XX_SFUNC(smb11);
NPCM8XX_SFUNC(smb12);
NPCM8XX_SFUNC(smb13);
NPCM8XX_SFUNC(smb14);
NPCM8XX_SFUNC(smb14b);
NPCM8XX_SFUNC(smb15);
NPCM8XX_SFUNC(smb16);
NPCM8XX_SFUNC(smb16b);
NPCM8XX_SFUNC(smb17);
NPCM8XX_SFUNC(smb18);
NPCM8XX_SFUNC(smb19);
NPCM8XX_SFUNC(smb20);
NPCM8XX_SFUNC(smb21);
NPCM8XX_SFUNC(smb22);
NPCM8XX_SFUNC(smb23);
NPCM8XX_SFUNC(smb23b);
NPCM8XX_SFUNC(fanin0);
NPCM8XX_SFUNC(fanin1);
NPCM8XX_SFUNC(fanin2);
NPCM8XX_SFUNC(fanin3);
NPCM8XX_SFUNC(fanin4);
NPCM8XX_SFUNC(fanin5);
NPCM8XX_SFUNC(fanin6);
NPCM8XX_SFUNC(fanin7);
NPCM8XX_SFUNC(fanin8);
NPCM8XX_SFUNC(fanin9);
NPCM8XX_SFUNC(fanin10);
NPCM8XX_SFUNC(fanin11);
NPCM8XX_SFUNC(fanin12);
NPCM8XX_SFUNC(fanin13);
NPCM8XX_SFUNC(fanin14);
NPCM8XX_SFUNC(fanin15);
NPCM8XX_SFUNC(faninx);
NPCM8XX_SFUNC(pwm0);
NPCM8XX_SFUNC(pwm1);
NPCM8XX_SFUNC(pwm2);
NPCM8XX_SFUNC(pwm3);
NPCM8XX_SFUNC(pwm4);
NPCM8XX_SFUNC(pwm5);
NPCM8XX_SFUNC(pwm6);
NPCM8XX_SFUNC(pwm7);
NPCM8XX_SFUNC(pwm8);
NPCM8XX_SFUNC(pwm9);
NPCM8XX_SFUNC(pwm10);
NPCM8XX_SFUNC(pwm11);
NPCM8XX_SFUNC(bu2);
NPCM8XX_SFUNC(sg1mdio);
NPCM8XX_SFUNC(rg2);
NPCM8XX_SFUNC(rg2refck);
NPCM8XX_SFUNC(rg2mdio);
NPCM8XX_SFUNC(ddr);
NPCM8XX_SFUNC(uart1);
NPCM8XX_SFUNC(uart2);
NPCM8XX_SFUNC(bmcuart0a);
NPCM8XX_SFUNC(bmcuart0b);
NPCM8XX_SFUNC(bmcuart1);
NPCM8XX_SFUNC(iox1);
NPCM8XX_SFUNC(iox2);
NPCM8XX_SFUNC(ioxh);
NPCM8XX_SFUNC(gspi);
NPCM8XX_SFUNC(mmc);
NPCM8XX_SFUNC(mmcwp);
NPCM8XX_SFUNC(mmccd);
NPCM8XX_SFUNC(mmcrst);
NPCM8XX_SFUNC(mmc8);
NPCM8XX_SFUNC(r1);
NPCM8XX_SFUNC(r1err);
NPCM8XX_SFUNC(r1md);
NPCM8XX_SFUNC(r2);
NPCM8XX_SFUNC(r2err);
NPCM8XX_SFUNC(r2md);
NPCM8XX_SFUNC(sd1);
NPCM8XX_SFUNC(sd1pwr);
NPCM8XX_SFUNC(wdog1);
NPCM8XX_SFUNC(wdog2);
NPCM8XX_SFUNC(scipme);
NPCM8XX_SFUNC(sci);
NPCM8XX_SFUNC(serirq);
NPCM8XX_SFUNC(jtag2);
NPCM8XX_SFUNC(spix);
NPCM8XX_SFUNC(spixcs1);
NPCM8XX_SFUNC(pspi);
NPCM8XX_SFUNC(ddc);
NPCM8XX_SFUNC(clkreq);
NPCM8XX_SFUNC(clkout);
NPCM8XX_SFUNC(spi3);
NPCM8XX_SFUNC(spi3cs1);
NPCM8XX_SFUNC(spi3quad);
NPCM8XX_SFUNC(spi3cs2);
NPCM8XX_SFUNC(spi3cs3);
NPCM8XX_SFUNC(spi0cs1);
NPCM8XX_SFUNC(lpc);
NPCM8XX_SFUNC(lpcclk);
NPCM8XX_SFUNC(espi);
NPCM8XX_SFUNC(lkgpo0);
NPCM8XX_SFUNC(lkgpo1);
NPCM8XX_SFUNC(lkgpo2);
NPCM8XX_SFUNC(nprd_smi);
NPCM8XX_SFUNC(hgpio0);
NPCM8XX_SFUNC(hgpio1);
NPCM8XX_SFUNC(hgpio2);
NPCM8XX_SFUNC(hgpio3);
NPCM8XX_SFUNC(hgpio4);
NPCM8XX_SFUNC(hgpio5);
NPCM8XX_SFUNC(hgpio6);
NPCM8XX_SFUNC(hgpio7);

/* Function names */
static struct npcm8xx_func npcm8xx_funcs[] = {
	NPCM8XX_MKFUNC(gpi35),
	NPCM8XX_MKFUNC(gpi36),
	NPCM8XX_MKFUNC(tp_jtag3),
	NPCM8XX_MKFUNC(tp_uart),
	NPCM8XX_MKFUNC(tp_smb2),
	NPCM8XX_MKFUNC(tp_smb1),
	NPCM8XX_MKFUNC(tp_gpio7),
	NPCM8XX_MKFUNC(tp_gpio6),
	NPCM8XX_MKFUNC(tp_gpio5),
	NPCM8XX_MKFUNC(tp_gpio4),
	NPCM8XX_MKFUNC(tp_gpio3),
	NPCM8XX_MKFUNC(tp_gpio2),
	NPCM8XX_MKFUNC(tp_gpio1),
	NPCM8XX_MKFUNC(tp_gpio0),
	NPCM8XX_MKFUNC(tp_gpio2b),
	NPCM8XX_MKFUNC(tp_gpio1b),
	NPCM8XX_MKFUNC(tp_gpio0b),
	NPCM8XX_MKFUNC(vgadig),
	NPCM8XX_MKFUNC(nbu1crts),
	NPCM8XX_MKFUNC(fm2),
	NPCM8XX_MKFUNC(fm1),
	NPCM8XX_MKFUNC(fm0),
	NPCM8XX_MKFUNC(gpio1836),
	NPCM8XX_MKFUNC(gpio1889),
	NPCM8XX_MKFUNC(gpio187),
	NPCM8XX_MKFUNC(cp1urxd),
	NPCM8XX_MKFUNC(r3rxer),
	NPCM8XX_MKFUNC(cp1gpio2c),
	NPCM8XX_MKFUNC(cp1gpio3c),
	NPCM8XX_MKFUNC(cp1gpio0b),
	NPCM8XX_MKFUNC(cp1gpio1b),
	NPCM8XX_MKFUNC(cp1gpio2b),
	NPCM8XX_MKFUNC(cp1gpio3b),
	NPCM8XX_MKFUNC(cp1gpio4b),
	NPCM8XX_MKFUNC(cp1gpio5b),
	NPCM8XX_MKFUNC(cp1gpio6b),
	NPCM8XX_MKFUNC(cp1gpio7b),
	NPCM8XX_MKFUNC(cp1gpio0),
	NPCM8XX_MKFUNC(cp1gpio1),
	NPCM8XX_MKFUNC(cp1gpio2),
	NPCM8XX_MKFUNC(cp1gpio3),
	NPCM8XX_MKFUNC(cp1gpio4),
	NPCM8XX_MKFUNC(cp1gpio5),
	NPCM8XX_MKFUNC(cp1gpio6),
	NPCM8XX_MKFUNC(cp1gpio7),
	NPCM8XX_MKFUNC(cp1utxd),
	NPCM8XX_MKFUNC(spi1cs3),
	NPCM8XX_MKFUNC(spi1cs2),
	NPCM8XX_MKFUNC(spi1cs1),
	NPCM8XX_MKFUNC(spi1),
	NPCM8XX_MKFUNC(spi1d23),
	NPCM8XX_MKFUNC(j2j3),
	NPCM8XX_MKFUNC(r3oen),
	NPCM8XX_MKFUNC(r2oen),
	NPCM8XX_MKFUNC(r1oen),
	NPCM8XX_MKFUNC(bu4b),
	NPCM8XX_MKFUNC(bu4),
	NPCM8XX_MKFUNC(bu5b),
	NPCM8XX_MKFUNC(bu5),
	NPCM8XX_MKFUNC(bu6),
	NPCM8XX_MKFUNC(rmii3),
	NPCM8XX_MKFUNC(jm1),
	NPCM8XX_MKFUNC(jm2),
	NPCM8XX_MKFUNC(tpgpio5b),
	NPCM8XX_MKFUNC(tpgpio4b),
	NPCM8XX_MKFUNC(clkrun),
	NPCM8XX_MKFUNC(i3c5),
	NPCM8XX_MKFUNC(i3c4),
	NPCM8XX_MKFUNC(i3c3),
	NPCM8XX_MKFUNC(i3c2),
	NPCM8XX_MKFUNC(i3c1),
	NPCM8XX_MKFUNC(i3c0),
	NPCM8XX_MKFUNC(hsi1a),
	NPCM8XX_MKFUNC(hsi2a),
	NPCM8XX_MKFUNC(hsi1b),
	NPCM8XX_MKFUNC(hsi2b),
	NPCM8XX_MKFUNC(hsi1c),
	NPCM8XX_MKFUNC(hsi2c),
	NPCM8XX_MKFUNC(smb0),
	NPCM8XX_MKFUNC(smb0b),
	NPCM8XX_MKFUNC(smb0c),
	NPCM8XX_MKFUNC(smb0d),
	NPCM8XX_MKFUNC(smb0den),
	NPCM8XX_MKFUNC(smb1),
	NPCM8XX_MKFUNC(smb1b),
	NPCM8XX_MKFUNC(smb1c),
	NPCM8XX_MKFUNC(smb1d),
	NPCM8XX_MKFUNC(smb2),
	NPCM8XX_MKFUNC(smb2b),
	NPCM8XX_MKFUNC(smb2c),
	NPCM8XX_MKFUNC(smb2d),
	NPCM8XX_MKFUNC(smb3),
	NPCM8XX_MKFUNC(smb3b),
	NPCM8XX_MKFUNC(smb3c),
	NPCM8XX_MKFUNC(smb3d),
	NPCM8XX_MKFUNC(smb4),
	NPCM8XX_MKFUNC(smb4b),
	NPCM8XX_MKFUNC(smb4c),
	NPCM8XX_MKFUNC(smb4d),
	NPCM8XX_MKFUNC(smb4den),
	NPCM8XX_MKFUNC(smb5),
	NPCM8XX_MKFUNC(smb5b),
	NPCM8XX_MKFUNC(smb5c),
	NPCM8XX_MKFUNC(smb5d),
	NPCM8XX_MKFUNC(ga20kbc),
	NPCM8XX_MKFUNC(smb6),
	NPCM8XX_MKFUNC(smb6b),
	NPCM8XX_MKFUNC(smb6c),
	NPCM8XX_MKFUNC(smb6d),
	NPCM8XX_MKFUNC(smb7),
	NPCM8XX_MKFUNC(smb7b),
	NPCM8XX_MKFUNC(smb7c),
	NPCM8XX_MKFUNC(smb7d),
	NPCM8XX_MKFUNC(smb8),
	NPCM8XX_MKFUNC(smb9),
	NPCM8XX_MKFUNC(smb10),
	NPCM8XX_MKFUNC(smb11),
	NPCM8XX_MKFUNC(smb12),
	NPCM8XX_MKFUNC(smb13),
	NPCM8XX_MKFUNC(smb14),
	NPCM8XX_MKFUNC(smb14b),
	NPCM8XX_MKFUNC(smb15),
	NPCM8XX_MKFUNC(smb16),
	NPCM8XX_MKFUNC(smb16b),
	NPCM8XX_MKFUNC(smb17),
	NPCM8XX_MKFUNC(smb18),
	NPCM8XX_MKFUNC(smb19),
	NPCM8XX_MKFUNC(smb20),
	NPCM8XX_MKFUNC(smb21),
	NPCM8XX_MKFUNC(smb22),
	NPCM8XX_MKFUNC(smb23),
	NPCM8XX_MKFUNC(smb23b),
	NPCM8XX_MKFUNC(fanin0),
	NPCM8XX_MKFUNC(fanin1),
	NPCM8XX_MKFUNC(fanin2),
	NPCM8XX_MKFUNC(fanin3),
	NPCM8XX_MKFUNC(fanin4),
	NPCM8XX_MKFUNC(fanin5),
	NPCM8XX_MKFUNC(fanin6),
	NPCM8XX_MKFUNC(fanin7),
	NPCM8XX_MKFUNC(fanin8),
	NPCM8XX_MKFUNC(fanin9),
	NPCM8XX_MKFUNC(fanin10),
	NPCM8XX_MKFUNC(fanin11),
	NPCM8XX_MKFUNC(fanin12),
	NPCM8XX_MKFUNC(fanin13),
	NPCM8XX_MKFUNC(fanin14),
	NPCM8XX_MKFUNC(fanin15),
	NPCM8XX_MKFUNC(faninx),
	NPCM8XX_MKFUNC(pwm0),
	NPCM8XX_MKFUNC(pwm1),
	NPCM8XX_MKFUNC(pwm2),
	NPCM8XX_MKFUNC(pwm3),
	NPCM8XX_MKFUNC(pwm4),
	NPCM8XX_MKFUNC(pwm5),
	NPCM8XX_MKFUNC(pwm6),
	NPCM8XX_MKFUNC(pwm7),
	NPCM8XX_MKFUNC(pwm8),
	NPCM8XX_MKFUNC(pwm9),
	NPCM8XX_MKFUNC(pwm10),
	NPCM8XX_MKFUNC(pwm11),
	NPCM8XX_MKFUNC(bu2),
	NPCM8XX_MKFUNC(sg1mdio),
	NPCM8XX_MKFUNC(rg2),
	NPCM8XX_MKFUNC(rg2refck),
	NPCM8XX_MKFUNC(rg2mdio),
	NPCM8XX_MKFUNC(ddr),
	NPCM8XX_MKFUNC(uart1),
	NPCM8XX_MKFUNC(uart2),
	NPCM8XX_MKFUNC(bmcuart0a),
	NPCM8XX_MKFUNC(bmcuart0b),
	NPCM8XX_MKFUNC(bmcuart1),
	NPCM8XX_MKFUNC(iox1),
	NPCM8XX_MKFUNC(iox2),
	NPCM8XX_MKFUNC(ioxh),
	NPCM8XX_MKFUNC(gspi),
	NPCM8XX_MKFUNC(mmc),
	NPCM8XX_MKFUNC(mmcwp),
	NPCM8XX_MKFUNC(mmccd),
	NPCM8XX_MKFUNC(mmcrst),
	NPCM8XX_MKFUNC(mmc8),
	NPCM8XX_MKFUNC(r1),
	NPCM8XX_MKFUNC(r1err),
	NPCM8XX_MKFUNC(r1md),
	NPCM8XX_MKFUNC(r2),
	NPCM8XX_MKFUNC(r2err),
	NPCM8XX_MKFUNC(r2md),
	NPCM8XX_MKFUNC(sd1),
	NPCM8XX_MKFUNC(sd1pwr),
	NPCM8XX_MKFUNC(wdog1),
	NPCM8XX_MKFUNC(wdog2),
	NPCM8XX_MKFUNC(scipme),
	NPCM8XX_MKFUNC(sci),
	NPCM8XX_MKFUNC(serirq),
	NPCM8XX_MKFUNC(jtag2),
	NPCM8XX_MKFUNC(spix),
	NPCM8XX_MKFUNC(spixcs1),
	NPCM8XX_MKFUNC(pspi),
	NPCM8XX_MKFUNC(ddc),
	NPCM8XX_MKFUNC(clkreq),
	NPCM8XX_MKFUNC(clkout),
	NPCM8XX_MKFUNC(spi3),
	NPCM8XX_MKFUNC(spi3cs1),
	NPCM8XX_MKFUNC(spi3quad),
	NPCM8XX_MKFUNC(spi3cs2),
	NPCM8XX_MKFUNC(spi3cs3),
	NPCM8XX_MKFUNC(spi0cs1),
	NPCM8XX_MKFUNC(lpc),
	NPCM8XX_MKFUNC(lpcclk),
	NPCM8XX_MKFUNC(espi),
	NPCM8XX_MKFUNC(lkgpo0),
	NPCM8XX_MKFUNC(lkgpo1),
	NPCM8XX_MKFUNC(lkgpo2),
	NPCM8XX_MKFUNC(nprd_smi),
	NPCM8XX_MKFUNC(hgpio0),
	NPCM8XX_MKFUNC(hgpio1),
	NPCM8XX_MKFUNC(hgpio2),
	NPCM8XX_MKFUNC(hgpio3),
	NPCM8XX_MKFUNC(hgpio4),
	NPCM8XX_MKFUNC(hgpio5),
	NPCM8XX_MKFUNC(hgpio6),
	NPCM8XX_MKFUNC(hgpio7),
};

#define NPCM8XX_PINCFG(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q) \
	[a] { .fn0 = fn_ ## b, .reg0 = NPCM8XX_GCR_ ## c, .bit0 = d, \
			.fn1 = fn_ ## e, .reg1 = NPCM8XX_GCR_ ## f, .bit1 = g, \
			.fn2 = fn_ ## h, .reg2 = NPCM8XX_GCR_ ## i, .bit2 = j, \
			.fn3 = fn_ ## k, .reg3 = NPCM8XX_GCR_ ## l, .bit3 = m, \
			.fn4 = fn_ ## n, .reg4 = NPCM8XX_GCR_ ## o, .bit4 = p, \
			.flag = q }

/* Drive strength controlled by NPCM8XX_GP_N_ODSC */
#define DRIVE_STRENGTH_LO_SHIFT		8
#define DRIVE_STRENGTH_HI_SHIFT		12
#define DRIVE_STRENGTH_MASK		0x0000FF00

#define DS(lo, hi)	(((lo) << DRIVE_STRENGTH_LO_SHIFT) | \
			 ((hi) << DRIVE_STRENGTH_HI_SHIFT))
#define DSLO(x)		(((x) >> DRIVE_STRENGTH_LO_SHIFT) & 0xF)
#define DSHI(x)		(((x) >> DRIVE_STRENGTH_HI_SHIFT) & 0xF)

#define GPI		0x1 /* Not GPO */
#define GPO		0x2 /* Not GPI */
#define SLEW		0x4 /* Has Slew Control, NPCM8XX_GP_N_OSRC */
#define SLEWLPC		0x8 /* Has Slew Control, SRCNT.3 */

struct npcm8xx_pincfg {
	int flag;
	int fn0, reg0, bit0;
	int fn1, reg1, bit1;
	int fn2, reg2, bit2;
	int fn3, reg3, bit3;
	int fn4, reg4, bit4;
};

static const struct npcm8xx_pincfg pincfgs[] = {
	/*            PIN       FUNCTION 1		   FUNCTION 2		     FUNCTION 3		   FUNCTION 4            FUNCTION 5       FLAGS  */
	NPCM8XX_PINCFG(0,	iox1, MFSEL1, 30,	smb6c, I2CSEGSEL, 25,	smb18, MFSEL5, 26,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(1,	iox1, MFSEL1, 30,	smb6c, I2CSEGSEL, 25,	smb18, MFSEL5, 26,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(2,	iox1, MFSEL1, 30,	smb6b, I2CSEGSEL, 24,	smb17, MFSEL5, 25,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(3,	iox1, MFSEL1, 30,	smb6b, I2CSEGSEL, 24,	smb17, MFSEL5, 25,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(4,	iox2, MFSEL3, 14,	smb1d, I2CSEGSEL, 7,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(5,	iox2, MFSEL3, 14,	smb1d, I2CSEGSEL, 7,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(6,	iox2, MFSEL3, 14,	smb2d, I2CSEGSEL, 10,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(7,	iox2, MFSEL3, 14,	smb2d, I2CSEGSEL, 10,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(8,	lkgpo1,	FLOCKR1, 4,	tp_gpio0, MFSEL7, 8,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12)),
	NPCM8XX_PINCFG(9,	lkgpo2,	FLOCKR1, 8,	tp_gpio1, MFSEL7, 9,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12)),
	NPCM8XX_PINCFG(10,	ioxh, MFSEL3, 18,	smb6d, I2CSEGSEL, 26,	smb16, MFSEL5, 24,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(11,	ioxh, MFSEL3, 18,	smb6d, I2CSEGSEL, 26,	smb16, MFSEL5, 24,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(12,	gspi, MFSEL1, 24,	smb5b, I2CSEGSEL, 19,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(13,	gspi, MFSEL1, 24,	smb5b, I2CSEGSEL, 19,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(14,	gspi, MFSEL1, 24,	smb5c, I2CSEGSEL, 20,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(15,	gspi, MFSEL1, 24,	smb5c, I2CSEGSEL, 20,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(16,	lkgpo0, FLOCKR1, 0,	smb7b, I2CSEGSEL, 27,	tp_gpio2, MFSEL7, 10,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(17,	pspi, MFSEL3, 13,	cp1gpio5, MFSEL6, 7,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(18,	pspi, MFSEL3, 13,	smb4b, I2CSEGSEL, 14,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(19,	pspi, MFSEL3, 13,	smb4b, I2CSEGSEL, 14,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(20,	hgpio0,	MFSEL2, 24,	smb15, MFSEL3, 8,	smb4c, I2CSEGSEL, 15,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(21,	hgpio1,	MFSEL2, 25,	smb15, MFSEL3, 8,	smb4c, I2CSEGSEL, 15,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(22,	hgpio2,	MFSEL2, 26,	smb14, MFSEL3, 7,	smb4d, I2CSEGSEL, 16,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(23,	hgpio3,	MFSEL2, 27,	smb14, MFSEL3, 7,	smb4d, I2CSEGSEL, 16,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(24,	hgpio4,	MFSEL2, 28,	ioxh, MFSEL3, 18,	smb7c, I2CSEGSEL, 28,	tp_smb2, MFSEL7, 28,	none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(25,	hgpio5,	MFSEL2, 29,	ioxh, MFSEL3, 18,	smb7c, I2CSEGSEL, 28,	tp_smb2, MFSEL7, 28,	none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(26,	smb5, MFSEL1, 2,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(27,	smb5, MFSEL1, 2,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(28,	smb4, MFSEL1, 1,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(29,	smb4, MFSEL1, 1,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(30,	smb3, MFSEL1, 0,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(31,	smb3, MFSEL1, 0,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(32,	smb14b, MFSEL7, 26,	spi0cs1, MFSEL1, 3,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(33,	i3c4, MFSEL6, 10,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(34,	i3c4, MFSEL6, 10,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(35,	gpi35, MFSEL5, 16,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPI),
	NPCM8XX_PINCFG(36,	gpi36, MFSEL5, 18,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPI),
	NPCM8XX_PINCFG(37,	smb3c, I2CSEGSEL, 12,	smb23, MFSEL5, 31,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(38,	smb3c, I2CSEGSEL, 12,	smb23, MFSEL5, 31,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(39,	smb3b, I2CSEGSEL, 11,	smb22, MFSEL5, 30,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(40,	smb3b, I2CSEGSEL, 11,	smb22, MFSEL5, 30,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(41,	bmcuart0a, MFSEL1, 9,	cp1urxd, MFSEL6, 31,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(42,	bmcuart0a, MFSEL1, 9,	cp1utxd, MFSEL6, 1,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(2, 4) | GPO),
	NPCM8XX_PINCFG(43,	uart1, MFSEL1, 10,	bmcuart1, MFSEL3, 24,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(44,	hsi1b, MFSEL1, 28,	nbu1crts, MFSEL6, 15,	jtag2, MFSEL4, 0,	tp_jtag3, MFSEL7, 13,	j2j3, MFSEL5, 2,	GPO),
	NPCM8XX_PINCFG(45,	hsi1c, MFSEL1, 4,	jtag2, MFSEL4, 0,	j2j3, MFSEL5, 2,	tp_jtag3, MFSEL7, 13,	none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(46,	hsi1c, MFSEL1, 4,	jtag2, MFSEL4, 0,	j2j3, MFSEL5, 2,	tp_jtag3, MFSEL7, 13,	none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(47,	hsi1c, MFSEL1, 4,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(2, 8)),
	NPCM8XX_PINCFG(48,	hsi2a, MFSEL1, 11,	bmcuart0b, MFSEL4, 1,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(49,	hsi2a, MFSEL1, 11,	bmcuart0b, MFSEL4, 1,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(50,	hsi2b, MFSEL1, 29,	bu6, MFSEL5, 6,		tp_uart, MFSEL7, 12,	none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(51,	hsi2b, MFSEL1, 29,	bu6, MFSEL5, 6,		tp_uart, MFSEL7, 12,	none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(52,	hsi2c, MFSEL1, 5,	bu5, MFSEL5, 7,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(53,	hsi2c, MFSEL1, 5,	bu5, MFSEL5, 7,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(54,	hsi2c, MFSEL1, 5,	bu4, MFSEL5, 8,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(55,	hsi2c, MFSEL1, 5,	bu4, MFSEL5, 8,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(56,	r1err, MFSEL1, 12,	r1oen, MFSEL5, 9,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(57,	r1md, MFSEL1, 13,	tpgpio4b, MFSEL5, 20,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(2, 4)),
	NPCM8XX_PINCFG(58,	r1md, MFSEL1, 13,	tpgpio5b, MFSEL5, 22,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(2, 4)),
	NPCM8XX_PINCFG(59,	hgpio6, MFSEL2, 30,	smb3d, I2CSEGSEL, 13,	smb19, MFSEL5, 27,	none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(60,	hgpio7, MFSEL2, 31,	smb3d, I2CSEGSEL, 13,	smb19, MFSEL5, 27,	none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(61,	hsi1c, MFSEL1, 4,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(62,	hsi1b, MFSEL1, 28,	jtag2, MFSEL4, 0,	j2j3, MFSEL5, 2,	nbu1crts, MFSEL6, 15,	tp_jtag3, MFSEL7, 13,	GPO),
	NPCM8XX_PINCFG(63,	hsi1a, MFSEL1, 10,	bmcuart1, MFSEL3, 24,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(64,	fanin0, MFSEL2, 0,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(65,	fanin1, MFSEL2, 1,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(66,	fanin2, MFSEL2, 2,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(67,	fanin3, MFSEL2, 3,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(68,	fanin4, MFSEL2, 4,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(69,	fanin5, MFSEL2, 5,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(70,	fanin6, MFSEL2, 6,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(71,	fanin7, MFSEL2, 7,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(72,	fanin8, MFSEL2, 8,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(73,	fanin9, MFSEL2, 9,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(74,	fanin10, MFSEL2, 10,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(75,	fanin11, MFSEL2, 11,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(76,	fanin12, MFSEL2, 12,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(77,	fanin13, MFSEL2, 13,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(78,	fanin14, MFSEL2, 14,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(79,	fanin15, MFSEL2, 15,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(80,	pwm0, MFSEL2, 16,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(4, 8)),
	NPCM8XX_PINCFG(81,	pwm1, MFSEL2, 17,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(4, 8)),
	NPCM8XX_PINCFG(82,	pwm2, MFSEL2, 18,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(4, 8)),
	NPCM8XX_PINCFG(83,	pwm3, MFSEL2, 19,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(4, 8)),
	NPCM8XX_PINCFG(84,	r2, MFSEL1, 14,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(85,	r2, MFSEL1, 14,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(86,	r2, MFSEL1, 14,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(87,	r2, MFSEL1, 14,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(88,	r2, MFSEL1, 14,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(89,	r2, MFSEL1, 14,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(90,	r2err, MFSEL1, 15,	r2oen, MFSEL5, 10,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(91,	r2md, MFSEL1, 16,	cp1gpio6, MFSEL6, 8,	tp_gpio0b, MFSEL7, 0,	none, NONE, 0,		none, NONE, 0,		DS(2, 4)),
	NPCM8XX_PINCFG(92,	r2md, MFSEL1, 16,	cp1gpio7, MFSEL6, 9,	tp_gpio1b, MFSEL7, 1,	none, NONE, 0,		none, NONE, 0,		DS(2, 4)),
	NPCM8XX_PINCFG(93,	ga20kbc, MFSEL1, 17,	smb5d, I2CSEGSEL, 21,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(94,	ga20kbc, MFSEL1, 17,	smb5d, I2CSEGSEL, 21,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(95,	lpc, NONE, 0,		espi, MFSEL4, 8,	gpio, MFSEL1, 26,	none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(96,	cp1gpio7b, MFSEL6, 24, bu2, MFSEL4, 22,	tp_gpio7, MFSEL7, 7,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(97,	cp1gpio6b, MFSEL6, 25, bu2, MFSEL4, 22,	tp_gpio6, MFSEL7, 6,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(98,	bu4b, MFSEL5, 13,	cp1gpio5b, MFSEL6, 26,	tp_gpio5, MFSEL7, 5,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(99,	bu4b, MFSEL5, 13,	cp1gpio4b, MFSEL6, 27,	tp_gpio4, MFSEL7, 4,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(100,	bu5b, MFSEL5, 13,	cp1gpio3c, MFSEL6, 28,	tp_gpio3, MFSEL7, 3,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(101,	bu5b, MFSEL5, 13,	cp1gpio2c, MFSEL6, 29,	tp_gpio2b, MFSEL7, 2,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(102,	vgadig, MFSEL7, 29,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(4, 8)),
	NPCM8XX_PINCFG(103,	vgadig, MFSEL7, 29,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(4, 8)),
	NPCM8XX_PINCFG(104,	vgadig, MFSEL7, 29,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(105,	vgadig, MFSEL7, 29,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(106,	i3c5, MFSEL3, 22,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(107,	i3c5, MFSEL3, 22,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(108,	sg1mdio, MFSEL4, 21,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(109,	sg1mdio, MFSEL4, 21,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(110,	rg2, MFSEL4, 24,	ddr, MFSEL3, 26,	rmii3, MFSEL5, 11,	none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(111,	rg2, MFSEL4, 24,	ddr, MFSEL3, 26,	rmii3, MFSEL5, 11,	none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(112,	rg2, MFSEL4, 24,	ddr, MFSEL3, 26,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(113,	rg2, MFSEL4, 24,	ddr, MFSEL3, 26,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(114,	smb0, MFSEL1, 6,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(115,	smb0, MFSEL1, 6,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(116,	smb1, MFSEL1, 7,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(117,	smb1, MFSEL1, 7,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(118,	smb2, MFSEL1, 8,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(119,	smb2, MFSEL1, 8,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(120,	smb2c, I2CSEGSEL, 9,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(121,	smb2c, I2CSEGSEL, 9,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(122,	smb2b, I2CSEGSEL, 8,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(123,	smb2b, I2CSEGSEL, 8,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(124,	smb1c, I2CSEGSEL, 6,	cp1gpio3b, MFSEL6, 23,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(125,	smb1c, I2CSEGSEL, 6,	cp1gpio2b, MFSEL6, 22,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(126,	smb1b, I2CSEGSEL, 5,	cp1gpio1b, MFSEL6, 21,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(127,	smb1b, I2CSEGSEL, 5,	cp1gpio0b, MFSEL6, 20,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(128,	smb8, MFSEL4, 11,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(129,	smb8, MFSEL4, 11,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(130,	smb9, MFSEL4, 12,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(131,	smb9, MFSEL4, 12,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(132,	smb10, MFSEL4, 13,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(133,	smb10, MFSEL4, 13,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(134,	smb11, MFSEL4, 14,	smb23b, MFSEL6, 0,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(135,	smb11, MFSEL4, 14,	smb23b, MFSEL6, 0,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(136,	jm1, MFSEL5, 15,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(137,	jm1, MFSEL5, 15,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(138,	jm1, MFSEL5, 15,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(139,	jm1, MFSEL5, 15,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(140,	jm1, MFSEL5, 15,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(141,	smb7b, I2CSEGSEL, 27,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(142,	smb7d, I2CSEGSEL, 29,	tp_smb1, MFSEL7, 11,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(143,	smb7d, I2CSEGSEL, 29,	tp_smb1, MFSEL7, 11,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(144,	pwm4, MFSEL2, 20,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(4, 8)),
	NPCM8XX_PINCFG(145,	pwm5, MFSEL2, 21,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(4, 8)),
	NPCM8XX_PINCFG(146,	pwm6, MFSEL2, 22,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(4, 8)),
	NPCM8XX_PINCFG(147,	pwm7, MFSEL2, 23,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(4, 8)),
	NPCM8XX_PINCFG(148,	mmc8, MFSEL3, 11,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(149,	mmc8, MFSEL3, 11,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(150,	mmc8, MFSEL3, 11,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(151,	mmc8, MFSEL3, 11,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(152,	mmc, MFSEL3, 10,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(153,	mmcwp, FLOCKR1, 24,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),  /* Z1/A1 */
	NPCM8XX_PINCFG(154,	mmc, MFSEL3, 10,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(155,	mmccd, MFSEL3, 25,	mmcrst, MFSEL4, 6,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),  /* Z1/A1 */
	NPCM8XX_PINCFG(156,	mmc, MFSEL3, 10,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(157,	mmc, MFSEL3, 10,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(158,	mmc, MFSEL3, 10,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(159,	mmc, MFSEL3, 10,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(160,	clkout, MFSEL1, 21,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(161,	lpc, NONE, 0,		espi, MFSEL4, 8,	gpio, MFSEL1, 26,	none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(162,	serirq, NONE, 0,	gpio, MFSEL1, 31,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12)),
	NPCM8XX_PINCFG(163,	lpc, MFSEL1, 26,	espi, MFSEL4, 8,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(164,	lpc, MFSEL1, 26,	espi, MFSEL4, 8,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(165,	lpc, MFSEL1, 26,	espi, MFSEL4, 8,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(166,	lpc, MFSEL1, 26,	espi, MFSEL4, 8,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(167,	lpc, MFSEL1, 26,	espi, MFSEL4, 8,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(168,	lpcclk, MFSEL1, 31,	espi, MFSEL4, 8,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(169,	scipme, MFSEL3, 0,	smb21, MFSEL5, 29,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(170,	sci, MFSEL1, 22,	smb21, MFSEL5, 29,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(171,	smb6, MFSEL3, 1,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(172,	smb6, MFSEL3, 1,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(173,	smb7, MFSEL3, 2,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(174,	smb7, MFSEL3, 2,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(175,	spi1, MFSEL3, 4,	faninx, MFSEL3, 3,	fm1, MFSEL6, 17,	none, NONE, 0,		none, NONE, 0,		DS(8, 12)),
	NPCM8XX_PINCFG(176,	spi1, MFSEL3, 4,	faninx, MFSEL3, 3,	fm1, MFSEL6, 17,	none, NONE, 0,		none, NONE, 0,		DS(8, 12)),
	NPCM8XX_PINCFG(177,	spi1, MFSEL3, 4,	faninx, MFSEL3, 3,	fm1, MFSEL6, 17,	none, NONE, 0,		none, NONE, 0,		DS(8, 12)),
	NPCM8XX_PINCFG(178,	r1, MFSEL3, 9,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(179,	r1, MFSEL3, 9,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(180,	r1, MFSEL3, 9,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(181,	r1, MFSEL3, 9,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(182,	r1, MFSEL3, 9,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(183,	spi3, MFSEL4, 16,	gpio1836, MFSEL6, 19,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(184,	spi3, MFSEL4, 16,	gpio1836, MFSEL6, 19,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(185,	spi3, MFSEL4, 16,	gpio1836, MFSEL6, 19,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(186,	spi3, MFSEL4, 16,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12)),
	NPCM8XX_PINCFG(187,	spi3cs1, MFSEL4, 17,	smb14b, MFSEL7, 26,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(188,	spi3quad, MFSEL4, 20,	spi3cs2, MFSEL4, 18,	gpio1889, MFSEL7, 25,	none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(189,	spi3quad, MFSEL4, 20,	spi3cs3, MFSEL4, 19,	gpio1889, MFSEL7, 25,	none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(190,	gpio, FLOCKR1, 20,	nprd_smi, NONE, 0,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(2, 4)),
	NPCM8XX_PINCFG(191,	spi1d23, MFSEL5, 3,	spi1cs2, MFSEL5, 4,	fm1, MFSEL6, 17,	smb15, MFSEL7, 27,	none, NONE, 0,		DS(0, 2)),  /* XX */
	NPCM8XX_PINCFG(192,	spi1d23, MFSEL5, 3,	spi1cs3, MFSEL5, 5,	fm1, MFSEL6, 17,	smb15, MFSEL7, 27,	none, NONE, 0,		DS(0, 2)),  /* XX */
	NPCM8XX_PINCFG(193,	r1, MFSEL3, 9,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(194,	smb0b, I2CSEGSEL, 0,	fm0, MFSEL6, 16,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(0, 1)),
	NPCM8XX_PINCFG(195,	smb0b, I2CSEGSEL, 0,	fm0, MFSEL6, 16,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(0, 1)),
	NPCM8XX_PINCFG(196,	smb0c, I2CSEGSEL, 1,	fm0, MFSEL6, 16,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(0, 1)),
	NPCM8XX_PINCFG(197,	smb0den, I2CSEGSEL, 22,	fm0, MFSEL6, 16,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(0, 1)),
	NPCM8XX_PINCFG(198,	smb0d, I2CSEGSEL, 2,	fm0, MFSEL6, 16,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(0, 1)),
	NPCM8XX_PINCFG(199,	smb0d, I2CSEGSEL, 2,	fm0, MFSEL6, 16,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(0, 1)),
	NPCM8XX_PINCFG(200,	r2, MFSEL1, 14,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(201,	r1, MFSEL3, 9,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		GPO),
	NPCM8XX_PINCFG(202,	smb0c, I2CSEGSEL, 1,	fm0, MFSEL6, 16,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(0, 1)),
	NPCM8XX_PINCFG(203,	faninx, MFSEL3, 3,	spi1, MFSEL3, 4,	fm1, MFSEL6, 17,	none, NONE, 0,		none, NONE, 0,		DS(8, 12)),
	NPCM8XX_PINCFG(208,	rg2, MFSEL4, 24,	ddr, MFSEL3, 26,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(209,	rg2, MFSEL4, 24,	ddr, MFSEL3, 26,	rmii3, MFSEL5, 11,	none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(210,	rg2, MFSEL4, 24,	ddr, MFSEL3, 26,	rmii3, MFSEL5, 11,	none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(211,	rg2, MFSEL4, 24,	ddr, MFSEL3, 26,	rmii3, MFSEL5, 11,	none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(212,	rg2, MFSEL4, 24,	ddr, MFSEL3, 26,	r3rxer, MFSEL6, 30,	none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(213,	rg2, MFSEL4, 24,	ddr, MFSEL3, 26,	rmii3, MFSEL5, 14,	none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(214,	rg2, MFSEL4, 24,	ddr, MFSEL3, 26,	rmii3, MFSEL5, 11,	none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(215,	rg2, MFSEL4, 24,	ddr, MFSEL3, 26,	rmii3, MFSEL5, 11,	none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(216,	rg2mdio, MFSEL4, 23,	ddr, MFSEL3, 26,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(217,	rg2mdio, MFSEL4, 23,	ddr, MFSEL3, 26,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(218,	wdog1, MFSEL3, 19,	smb16, MFSEL7, 30,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(219,	wdog2, MFSEL3, 20,	smb16, MFSEL7, 30,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(220,	smb12, MFSEL3, 5,	pwm8, MFSEL6, 11,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(221,	smb12, MFSEL3, 5,	pwm9, MFSEL6, 12,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(222,	smb13, MFSEL3, 6,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(223,	smb13, MFSEL3, 6,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(224,	spix, MFSEL4, 27,	fm2, MFSEL6, 18,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(225,	spix, MFSEL4, 27,	fm2, MFSEL6, 18,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(226,	spix, MFSEL4, 27,	fm2, MFSEL6, 18,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(227,	spix, MFSEL4, 27,	fm2, MFSEL6, 18,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(228,	spixcs1, MFSEL4, 28,	fm2, MFSEL6, 18,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(229,	spix, MFSEL4, 27,	fm2, MFSEL6, 18,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(230,	spix, MFSEL4, 27,	fm2, MFSEL6, 18,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
	NPCM8XX_PINCFG(231,	clkreq, MFSEL4, 9,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		DS(4, 12) | SLEW),
	NPCM8XX_PINCFG(233,	spi1cs1, MFSEL5, 0,	fm1, MFSEL6, 17,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEWLPC),
	NPCM8XX_PINCFG(234,	pwm10, MFSEL6, 13,	smb20, MFSEL5, 28,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		0),
	NPCM8XX_PINCFG(235,	pwm11, MFSEL6, 14,	smb20, MFSEL5, 28,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(240,	i3c0, MFSEL5, 17,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(241,	i3c0, MFSEL5, 17,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(242,	i3c1, MFSEL5, 19,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(243,	i3c1, MFSEL5, 19,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(244,	i3c2, MFSEL5, 21,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(245,	i3c2, MFSEL5, 21,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(246,	i3c3, MFSEL5, 23,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(247,	i3c3, MFSEL5, 23,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(249,	r2oen, INTCR4, 13,	none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		none, NONE, 0,		SLEW),
	NPCM8XX_PINCFG(250,	rg2refck, INTCR4, 6,	ddr, MFSEL3, 26,	rmii3, MFSEL5, 14,	none, NONE, 0,		none, NONE, 0,		DS(8, 12) | SLEW),
};

#define NPCM8XX_PIN(a, b) { .number = a, .name = b }
struct npcm8xx_pin_desc {
	unsigned int number;
	const char *name;
};

/* number, name, drv_data */
static const struct npcm8xx_pin_desc npcm8xx_pins[] = {
	NPCM8XX_PIN(0,	"GPIO0/IOX1_DI/SMB6C_SDA/SMB18_SDA"),
	NPCM8XX_PIN(1,	"GPIO1/IOX1_LD/SMB6C_SCL/SMB18_SCL"),
	NPCM8XX_PIN(2,	"GPIO2/IOX1_CK/SMB6B_SDA/SMB17_SDA"),
	NPCM8XX_PIN(3,	"GPIO3/IOX1_DO/SMB6B_SCL/SMB17_SCL"),
	NPCM8XX_PIN(4,	"GPIO4/IOX2_DI/SMB1D_SDA"),
	NPCM8XX_PIN(5,	"GPIO5/IOX2_LD/SMB1D_SCL"),
	NPCM8XX_PIN(6,	"GPIO6/IOX2_CK/SMB2D_SDA"),
	NPCM8XX_PIN(7,	"GPIO7/IOX2_D0/SMB2D_SCL"),
	NPCM8XX_PIN(8,	"GPIO8/LKGPO1/TP_GPIO0"),
	NPCM8XX_PIN(9,	"GPIO9/LKGPO2/TP_GPIO1"),
	NPCM8XX_PIN(10, "GPIO10/IOXH_LD/SMB6D_SCL/SMB16_SCL"),
	NPCM8XX_PIN(11, "GPIO11/IOXH_CK/SMB6D_SDA/SMB16_SDA"),
	NPCM8XX_PIN(12, "GPIO12/GSPI_CK/SMB5B_SCL"),
	NPCM8XX_PIN(13, "GPIO13/GSPI_DO/SMB5B_SDA"),
	NPCM8XX_PIN(14, "GPIO14/GSPI_DI/SMB5C_SCL"),
	NPCM8XX_PIN(15, "GPIO15/GSPI_CS/SMB5C_SDA"),
	NPCM8XX_PIN(16, "GPIO16/SMB7B_SDA/LKGPO0/TP_GPIO2"),
	NPCM8XX_PIN(17, "GPIO17/PSPI_DI/CP1_GPIO5"),
	NPCM8XX_PIN(18, "GPIO18/PSPI_D0/SMB4B_SDA"),
	NPCM8XX_PIN(19, "GPIO19/PSPI_CK/SMB4B_SCL"),
	NPCM8XX_PIN(20, "GPIO20/H_GPIO0/SMB4C_SDA/SMB15_SDA"),
	NPCM8XX_PIN(21, "GPIO21/H_GPIO1/SMB4C_SCL/SMB15_SCL"),
	NPCM8XX_PIN(22, "GPIO22/H_GPIO2/SMB4D_SDA/SMB14_SDA"),
	NPCM8XX_PIN(23, "GPIO23/H_GPIO3/SMB4D_SCL/SMB14_SCL"),
	NPCM8XX_PIN(24, "GPIO24/IOXH_DO/H_GPIO4/SMB7C_SCL/TP_SMB2_SCL"),
	NPCM8XX_PIN(25, "GPIO25/IOXH_DI/H_GPIO4/SMB7C_SDA/TP_SMB2_SDA"),
	NPCM8XX_PIN(26, "GPIO26/SMB5_SDA"),
	NPCM8XX_PIN(27, "GPIO27/SMB5_SCL"),
	NPCM8XX_PIN(28, "GPIO28/SMB4_SDA"),
	NPCM8XX_PIN(29, "GPIO29/SMB4_SCL"),
	NPCM8XX_PIN(30, "GPIO30/SMB3_SDA"),
	NPCM8XX_PIN(31, "GPIO31/SMB3_SCL"),
	NPCM8XX_PIN(32, "GPIO32/SMB14_SCL/SPI0_nCS1"),
	NPCM8XX_PIN(33, "I3C4_SCL"),
	NPCM8XX_PIN(34, "I3C4_SDA"),
	NPCM8XX_PIN(35, "NA"),
	NPCM8XX_PIN(36, "NA"),
	NPCM8XX_PIN(37, "GPIO37/SMB3C_SDA/SMB23_SDA"),
	NPCM8XX_PIN(38, "GPIO38/SMB3C_SCL/SMB23_SCL"),
	NPCM8XX_PIN(39, "GPIO39/SMB3B_SDA/SMB22_SDA"),
	NPCM8XX_PIN(40, "GPIO40/SMB3B_SCL/SMB22_SCL"),
	NPCM8XX_PIN(41, "GPIO41/BU0_RXD/CP1U_RXD"),
	NPCM8XX_PIN(42, "GPIO42/BU0_TXD/CP1U_TXD"),
	NPCM8XX_PIN(43, "GPIO43/SI1_RXD/BU1_RXD"),
	NPCM8XX_PIN(44, "GPIO44/SI1_nCTS/BU1_nCTS/CP_TDI/TP_TDI/CP_TP_TDI"),
	NPCM8XX_PIN(45, "GPIO45/SI1_nDCD/CP_TMS_SWIO/TP_TMS_SWIO/CP_TP_TMS_SWIO"),
	NPCM8XX_PIN(46, "GPIO46/SI1_nDSR/CP_TCK_SWCLK/TP_TCK_SWCLK/CP_TP_TCK_SWCLK"),
	NPCM8XX_PIN(47, "GPIO47/SI1n_RI1"),
	NPCM8XX_PIN(48, "GPIO48/SI2_TXD/BU0_TXD/STRAP5"),
	NPCM8XX_PIN(49, "GPIO49/SI2_RXD/BU0_RXD"),
	NPCM8XX_PIN(50, "GPIO50/SI2_nCTS/BU6_TXD/TPU_TXD"),
	NPCM8XX_PIN(51, "GPIO51/SI2_nRTS/BU6_RXD/TPU_RXD"),
	NPCM8XX_PIN(52, "GPIO52/SI2_nDCD/BU5_RXD"),
	NPCM8XX_PIN(53, "GPIO53/SI2_nDTR_BOUT2/BU5_TXD"),
	NPCM8XX_PIN(54, "GPIO54/SI2_nDSR/BU4_TXD"),
	NPCM8XX_PIN(55, "GPIO55/SI2_RI2/BU4_RXD"),
	NPCM8XX_PIN(56, "GPIO56/R1_RXERR/R1_OEN"),
	NPCM8XX_PIN(57, "GPIO57/R1_MDC/TP_GPIO4"),
	NPCM8XX_PIN(58, "GPIO58/R1_MDIO/TP_GPIO5"),
	NPCM8XX_PIN(59, "GPIO59/H_GPIO06/SMB3D_SDA/SMB19_SDA"),
	NPCM8XX_PIN(60, "GPIO60/H_GPIO07/SMB3D_SCL/SMB19_SCL"),
	NPCM8XX_PIN(61, "GPIO61/SI1_nDTR_BOUT"),
	NPCM8XX_PIN(62, "GPIO62/SI1_nRTS/BU1_nRTS/CP_TDO_SWO/TP_TDO_SWO/CP_TP_TDO_SWO"),
	NPCM8XX_PIN(63, "GPIO63/BU1_TXD1/SI1_TXD"),
	NPCM8XX_PIN(64, "GPIO64/FANIN0"),
	NPCM8XX_PIN(65, "GPIO65/FANIN1"),
	NPCM8XX_PIN(66, "GPIO66/FANIN2"),
	NPCM8XX_PIN(67, "GPIO67/FANIN3"),
	NPCM8XX_PIN(68, "GPIO68/FANIN4"),
	NPCM8XX_PIN(69, "GPIO69/FANIN5"),
	NPCM8XX_PIN(70, "GPIO70/FANIN6"),
	NPCM8XX_PIN(71, "GPIO71/FANIN7"),
	NPCM8XX_PIN(72, "GPIO72/FANIN8"),
	NPCM8XX_PIN(73, "GPIO73/FANIN9"),
	NPCM8XX_PIN(74, "GPIO74/FANIN10"),
	NPCM8XX_PIN(75, "GPIO75/FANIN11"),
	NPCM8XX_PIN(76, "GPIO76/FANIN12"),
	NPCM8XX_PIN(77, "GPIO77/FANIN13"),
	NPCM8XX_PIN(78, "GPIO78/FANIN14"),
	NPCM8XX_PIN(79, "GPIO79/FANIN15"),
	NPCM8XX_PIN(80, "GPIO80/PWM0"),
	NPCM8XX_PIN(81, "GPIO81/PWM1"),
	NPCM8XX_PIN(82, "GPIO82/PWM2"),
	NPCM8XX_PIN(83, "GPIO83/PWM3"),
	NPCM8XX_PIN(84, "GPIO84/R2_TXD0"),
	NPCM8XX_PIN(85, "GPIO85/R2_TXD1"),
	NPCM8XX_PIN(86, "GPIO86/R2_TXEN"),
	NPCM8XX_PIN(87, "GPIO87/R2_RXD0"),
	NPCM8XX_PIN(88, "GPIO88/R2_RXD1"),
	NPCM8XX_PIN(89, "GPIO89/R2_CRSDV"),
	NPCM8XX_PIN(90, "GPIO90/R2_RXERR/R2_OEN"),
	NPCM8XX_PIN(91, "GPIO91/R2_MDC/CP1_GPIO6/TP_GPIO0"),
	NPCM8XX_PIN(92, "GPIO92/R2_MDIO/CP1_GPIO7/TP_GPIO1"),
	NPCM8XX_PIN(93, "GPIO93/GA20/SMB5D_SCL"),
	NPCM8XX_PIN(94, "GPIO94/nKBRST/SMB5D_SDA"),
	NPCM8XX_PIN(95, "GPIO95/nESPIRST/LPC_nLRESET"),
	NPCM8XX_PIN(96, "GPIO96/CP1_GPIO7/BU2_TXD/TP_GPIO7"),
	NPCM8XX_PIN(97, "GPIO97/CP1_GPIO6/BU2_RXD/TP_GPIO6"),
	NPCM8XX_PIN(98, "GPIO98/CP1_GPIO5/BU4_TXD/TP_GPIO5"),
	NPCM8XX_PIN(99, "GPIO99/CP1_GPIO4/BU4_RXD/TP_GPIO4"),
	NPCM8XX_PIN(100, "GPIO100/CP1_GPIO3/BU5_TXD/TP_GPIO3"),
	NPCM8XX_PIN(101, "GPIO101/CP1_GPIO2/BU5_RXD/TP_GPIO2"),
	NPCM8XX_PIN(102, "GPIO102/HSYNC"),
	NPCM8XX_PIN(103, "GPIO103/VSYNC"),
	NPCM8XX_PIN(104, "GPIO104/DDC_SCL"),
	NPCM8XX_PIN(105, "GPIO105/DDC_SDA"),
	NPCM8XX_PIN(106, "GPIO106/I3C5_SCL"),
	NPCM8XX_PIN(107, "GPIO107/I3C5_SDA"),
	NPCM8XX_PIN(108, "GPIO108/SG1_MDC"),
	NPCM8XX_PIN(109, "GPIO109/SG1_MDIO"),
	NPCM8XX_PIN(110, "GPIO110/RG2_TXD0/DDRV0/R3_TXD0"),
	NPCM8XX_PIN(111, "GPIO111/RG2_TXD1/DDRV1/R3_TXD1"),
	NPCM8XX_PIN(112, "GPIO112/RG2_TXD2/DDRV2"),
	NPCM8XX_PIN(113, "GPIO113/RG2_TXD3/DDRV3"),
	NPCM8XX_PIN(114, "GPIO114/SMB0_SCL"),
	NPCM8XX_PIN(115, "GPIO115/SMB0_SDA"),
	NPCM8XX_PIN(116, "GPIO116/SMB1_SCL"),
	NPCM8XX_PIN(117, "GPIO117/SMB1_SDA"),
	NPCM8XX_PIN(118, "GPIO118/SMB2_SCL"),
	NPCM8XX_PIN(119, "GPIO119/SMB2_SDA"),
	NPCM8XX_PIN(120, "GPIO120/SMB2C_SDA"),
	NPCM8XX_PIN(121, "GPIO121/SMB2C_SCL"),
	NPCM8XX_PIN(122, "GPIO122/SMB2B_SDA"),
	NPCM8XX_PIN(123, "GPIO123/SMB2B_SCL"),
	NPCM8XX_PIN(124, "GPIO124/SMB1C_SDA/CP1_GPIO3"),
	NPCM8XX_PIN(125, "GPIO125/SMB1C_SCL/CP1_GPIO2"),
	NPCM8XX_PIN(126, "GPIO126/SMB1B_SDA/CP1_GPIO1"),
	NPCM8XX_PIN(127, "GPIO127/SMB1B_SCL/CP1_GPIO0"),
	NPCM8XX_PIN(128, "GPIO128/SMB824_SCL"),
	NPCM8XX_PIN(129, "GPIO129/SMB824_SDA"),
	NPCM8XX_PIN(130, "GPIO130/SMB925_SCL"),
	NPCM8XX_PIN(131, "GPIO131/SMB925_SDA"),
	NPCM8XX_PIN(132, "GPIO132/SMB1026_SCL"),
	NPCM8XX_PIN(133, "GPIO133/SMB1026_SDA"),
	NPCM8XX_PIN(134, "GPIO134/SMB11_SCL"),
	NPCM8XX_PIN(135, "GPIO135/SMB11_SDA"),
	NPCM8XX_PIN(136, "GPIO136/JM1_TCK"),
	NPCM8XX_PIN(137, "GPIO137/JM1_TDO"),
	NPCM8XX_PIN(138, "GPIO138/JM1_TMS"),
	NPCM8XX_PIN(139, "GPIO139/JM1_TDI"),
	NPCM8XX_PIN(140, "GPIO140/JM1_nTRST"),
	NPCM8XX_PIN(141, "GPIO141/SMB7B_SCL"),
	NPCM8XX_PIN(142, "GPIO142/SMB7D_SCL/TPSMB1_SCL"),
	NPCM8XX_PIN(143, "GPIO143/SMB7D_SDA/TPSMB1_SDA"),
	NPCM8XX_PIN(144, "GPIO144/PWM4"),
	NPCM8XX_PIN(145, "GPIO145/PWM5"),
	NPCM8XX_PIN(146, "GPIO146/PWM6"),
	NPCM8XX_PIN(147, "GPIO147/PWM7"),
	NPCM8XX_PIN(148, "GPIO148/MMC_DT4"),
	NPCM8XX_PIN(149, "GPIO149/MMC_DT5"),
	NPCM8XX_PIN(150, "GPIO150/MMC_DT6"),
	NPCM8XX_PIN(151, "GPIO151/MMC_DT7"),
	NPCM8XX_PIN(152, "GPIO152/MMC_CLK"),
	NPCM8XX_PIN(153, "GPIO153/MMC_WP"),
	NPCM8XX_PIN(154, "GPIO154/MMC_CMD"),
	NPCM8XX_PIN(155, "GPIO155/MMC_nCD/MMC_nRSTLK"),
	NPCM8XX_PIN(156, "GPIO156/MMC_DT0"),
	NPCM8XX_PIN(157, "GPIO157/MMC_DT1"),
	NPCM8XX_PIN(158, "GPIO158/MMC_DT2"),
	NPCM8XX_PIN(159, "GPIO159/MMC_DT3"),
	NPCM8XX_PIN(160, "GPIO160/CLKOUT/RNGOSCOUT/GFXBYPCK"),
	NPCM8XX_PIN(161, "GPIO161/ESPI_nCS/LPC_nLFRAME"),
	NPCM8XX_PIN(162, "GPIO162/LPC_nCLKRUN"),
	NPCM8XX_PIN(163, "GPIO163/ESPI_CK/LPC_LCLK"),
	NPCM8XX_PIN(164, "GPIO164/ESPI_IO0/LPC_LAD0"),
	NPCM8XX_PIN(165, "GPIO165/ESPI_IO1/LPC_LAD1"),
	NPCM8XX_PIN(166, "GPIO166/ESPI_IO2/LPC_LAD2"),
	NPCM8XX_PIN(167, "GPIO167/ESPI_IO3/LPC_LAD3"),
	NPCM8XX_PIN(168, "GPIO168/ESPI_nALERT/SERIRQ"),
	NPCM8XX_PIN(169, "GPIO169/nSCIPME/SMB21_SCL"),
	NPCM8XX_PIN(170, "GPIO170/nSMI/SMB21_SDA"),
	NPCM8XX_PIN(171, "GPIO171/SMB6_SCL"),
	NPCM8XX_PIN(172, "GPIO172/SMB6_SDA"),
	NPCM8XX_PIN(173, "GPIO173/SMB7_SCL"),
	NPCM8XX_PIN(174, "GPIO174/SMB7_SDA"),
	NPCM8XX_PIN(175, "GPIO175/SPI1_CK/FANIN19/FM1_CK"),
	NPCM8XX_PIN(176, "GPIO176/SPI1_DO/FANIN18/FM1_DO/STRAP9"),
	NPCM8XX_PIN(177, "GPIO177/SPI1_DI/FANIN17/FM1_D1/STRAP10"),
	NPCM8XX_PIN(178, "GPIO178/R1_TXD0"),
	NPCM8XX_PIN(179, "GPIO179/R1_TXD1"),
	NPCM8XX_PIN(180, "GPIO180/R1_TXEN"),
	NPCM8XX_PIN(181, "GPIO181/R1_RXD0"),
	NPCM8XX_PIN(182, "GPIO182/R1_RXD1"),
	NPCM8XX_PIN(183, "GPIO183/SPI3_SEL"),
	NPCM8XX_PIN(184, "GPIO184/SPI3_D0/STRAP13"),
	NPCM8XX_PIN(185, "GPIO185/SPI3_D1"),
	NPCM8XX_PIN(186, "GPIO186/SPI3_nCS0"),
	NPCM8XX_PIN(187, "GPIO187/SPI3_nCS1_SMB14_SDA"),
	NPCM8XX_PIN(188, "GPIO188/SPI3_D2/SPI3_nCS2"),
	NPCM8XX_PIN(189, "GPIO189/SPI3_D3/SPI3_nCS3"),
	NPCM8XX_PIN(190, "GPIO190/nPRD_SMI"),
	NPCM8XX_PIN(191, "GPIO191/SPI1_D1/FANIN17/FM1_D1/STRAP10"),
	NPCM8XX_PIN(192, "GPIO192/SPI1_D3/SPI_nCS3/FM1_D3/SMB15_SCL"),
	NPCM8XX_PIN(193, "GPIO193/R1_CRSDV"),
	NPCM8XX_PIN(194, "GPIO194/SMB0B_SCL/FM0_CK"),
	NPCM8XX_PIN(195, "GPIO195/SMB0B_SDA/FM0_D0"),
	NPCM8XX_PIN(196, "GPIO196/SMB0C_SCL/FM0_D1"),
	NPCM8XX_PIN(197, "GPIO197/SMB0DEN/FM0_D3"),
	NPCM8XX_PIN(198, "GPIO198/SMB0D_SDA/FM0_D2"),
	NPCM8XX_PIN(199, "GPIO199/SMB0D_SCL/FM0_CSO"),
	NPCM8XX_PIN(200, "GPIO200/R2_CK"),
	NPCM8XX_PIN(201, "GPIO201/R1_CK"),
	NPCM8XX_PIN(202, "GPIO202/SMB0C_SDA/FM0_CSI"),
	NPCM8XX_PIN(203, "GPIO203/SPI1_nCS0/FANIN16/FM1_CSI"),
	NPCM8XX_PIN(204, "NA"),
	NPCM8XX_PIN(205, "NA"),
	NPCM8XX_PIN(206, "NA"),
	NPCM8XX_PIN(207, "NA"),
	NPCM8XX_PIN(208, "GPIO208/RG2_TXC/DVCK"),
	NPCM8XX_PIN(209, "GPIO209/RG2_TXCTL/DDRV4/R3_TXEN"),
	NPCM8XX_PIN(210, "GPIO210/RG2_RXD0/DDRV5/R3_RXD0"),
	NPCM8XX_PIN(211, "GPIO211/RG2_RXD1/DDRV6/R3_RXD1"),
	NPCM8XX_PIN(212, "GPIO212/RG2_RXD2/DDRV7/R3_RXD2"),
	NPCM8XX_PIN(213, "GPIO213/RG2_RXD3/DDRV8/R3_OEN"),
	NPCM8XX_PIN(214, "GPIO214/RG2_RXC/DDRV9/R3_CK"),
	NPCM8XX_PIN(215, "GPIO215/RG2_RXCTL/DDRV10/R3_CRSDV"),
	NPCM8XX_PIN(216, "GPIO216/RG2_MDC/DDRV11"),
	NPCM8XX_PIN(217, "GPIO217/RG2_MDIO/DVHSYNC"),
	NPCM8XX_PIN(218, "GPIO218/nWDO1/SMB16_SCL"),
	NPCM8XX_PIN(219, "GPIO219/nWDO2/SMB16_SDA"),
	NPCM8XX_PIN(220, "GPIO220/SMB12_SCL/PWM8"),
	NPCM8XX_PIN(221, "GPIO221/SMB12_SDA/PWM9"),
	NPCM8XX_PIN(222, "GPIO222/SMB13_SCL"),
	NPCM8XX_PIN(223, "GPIO223/SMB13_SDA"),
	NPCM8XX_PIN(224, "GPIO224/SPIX_CK/FM2_CK"),
	NPCM8XX_PIN(225, "GPO225/SPIX_D0/FM2_D0/STRAP1"),
	NPCM8XX_PIN(226, "GPO226/SPIX_D1/FM2_D1/STRAP2"),
	NPCM8XX_PIN(227, "GPIO227/SPIX_nCS0/FM2_CSI"),
	NPCM8XX_PIN(228, "GPIO228/SPIX_nCS1/FM2_CSO"),
	NPCM8XX_PIN(229, "GPO229/SPIX_D2/FM2_D2/STRAP3"),
	NPCM8XX_PIN(230, "GPO230/SPIX_D3/FM2_D3/STRAP6"),
	NPCM8XX_PIN(231, "GPIO231/EP_nCLKREQ"),
	NPCM8XX_PIN(232, "NA"),
	NPCM8XX_PIN(233, "GPIO233/SPI1_nCS1/FM1_CSO"),
	NPCM8XX_PIN(234, "GPIO234/PWM10/SMB20_SCL"),
	NPCM8XX_PIN(235, "GPIO235/PWM11/SMB20_SDA"),
	NPCM8XX_PIN(236, "NA"),
	NPCM8XX_PIN(237, "NA"),
	NPCM8XX_PIN(238, "NA"),
	NPCM8XX_PIN(239, "NA"),
	NPCM8XX_PIN(240, "GPIO240/I3C0_SCL"),
	NPCM8XX_PIN(241, "GPIO241/I3C0_SDA"),
	NPCM8XX_PIN(242, "GPIO242/I3C1_SCL"),
	NPCM8XX_PIN(243, "GPIO243/I3C1_SDA"),
	NPCM8XX_PIN(244, "GPIO244/I3C2_SCL"),
	NPCM8XX_PIN(245, "GPIO245/I3C2_SDA"),
	NPCM8XX_PIN(246, "GPIO246/I3C3_SCL"),
	NPCM8XX_PIN(247, "GPIO247/I3C3_SDA"),
	NPCM8XX_PIN(248, "NA"),
	NPCM8XX_PIN(249, "R2_OEN"),
	NPCM8XX_PIN(250, "GPIO250/RG2_REFCK/DVVSYNC"),
};

struct npcm8xx_pinctrl_priv {
	u32 gcr_base;
	u32 clk_base;
	u32 gpio_base;
};

static int npcm8xx_pinctrl_probe(struct udevice *dev)
{
	struct npcm8xx_pinctrl_priv *priv = dev_get_priv(dev);

	priv->gcr_base = NPCM_GCR_BA;
	priv->clk_base = NPCM_CLK_BA;
	priv->gpio_base = NPCM_GPIO_BA;

	return 0;
}

/* Enable mode in pin group */
static void npcm8xx_setfunc(struct udevice *dev, const unsigned int *pin,
			    unsigned int pin_number, unsigned int mode)
{
	struct npcm8xx_pinctrl_priv *priv = dev_get_priv(dev);
	const struct npcm8xx_pincfg *cfg;
	int i;
	u32 gcr_reg = priv->gcr_base;

	for (i = 0 ; i < pin_number ; i++) {
		cfg = &pincfgs[pin[i]];
		if (mode == fn_gpio || cfg->fn0 == mode || cfg->fn1 == mode ||
		    cfg->fn2 == mode || cfg->fn3 == mode || cfg->fn4 == mode) {
			if (cfg->reg0) {
				if (cfg->fn0 == mode)
					setbits_le32((uintptr_t)(gcr_reg + cfg->reg0),
						     BIT(cfg->bit0));
				else
					clrbits_le32((uintptr_t)(gcr_reg + cfg->reg0),
						     BIT(cfg->bit0));
			}
			if (cfg->reg1) {
				if (cfg->fn1 == mode)
					setbits_le32((uintptr_t)(gcr_reg + cfg->reg1),
						     BIT(cfg->bit1));
				else
					clrbits_le32((uintptr_t)(gcr_reg + cfg->reg1),
						     BIT(cfg->bit1));
			}
			if (cfg->reg2) {
				if (cfg->fn2 == mode)
					setbits_le32((uintptr_t)(gcr_reg + cfg->reg2),
						     BIT(cfg->bit2));
				else
					clrbits_le32((uintptr_t)(gcr_reg + cfg->reg2),
						     BIT(cfg->bit2));
			}
			if (cfg->reg3) {
				if (cfg->fn3 == mode)
					setbits_le32((uintptr_t)(gcr_reg + cfg->reg3),
						     BIT(cfg->bit3));
				else
					clrbits_le32((uintptr_t)(gcr_reg + cfg->reg3),
						     BIT(cfg->bit3));
			}
			if (cfg->reg4) {
				if (cfg->fn4 == mode)
					setbits_le32((uintptr_t)(gcr_reg + cfg->reg4),
						     BIT(cfg->bit4));
				else
					clrbits_le32((uintptr_t)(gcr_reg + cfg->reg4),
						     BIT(cfg->bit4));
			}
		}
	}
}

static int npcm8xx_get_pins_count(struct udevice *dev)
{
	return ARRAY_SIZE(npcm8xx_pins);
}

static const char *npcm8xx_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	return npcm8xx_pins[selector].name;
}

static int npcm8xx_get_groups_count(struct udevice *dev)
{
	return ARRAY_SIZE(npcm8xx_groups);
}

static const char *npcm8xx_get_group_name(struct udevice *dev,
					  unsigned int selector)
{
	return npcm8xx_groups[selector].name;
}

static int npcm8xx_get_functions_count(struct udevice *dev)
{
	return ARRAY_SIZE(npcm8xx_funcs);
}

static const char *npcm8xx_get_function_name(struct udevice *dev,
					     unsigned int selector)
{
	return npcm8xx_funcs[selector].name;
}

static int npcm8xx_pinmux_set(struct udevice *dev, unsigned int group,
			      unsigned int function)
{
	dev_dbg(dev, "set_mux: %d, %d[%s]\n", function, group,
		npcm8xx_groups[group].name);

	npcm8xx_setfunc(dev, npcm8xx_groups[group].pins,
			npcm8xx_groups[group].npins, group);

	return 0;
}

#define PIN_CONFIG_PERSIST_STATE (PIN_CONFIG_END + 1)
#define PIN_CONFIG_POLARITY_STATE (PIN_CONFIG_END + 2)
#define PIN_CONFIG_EVENT_CLEAR (PIN_CONFIG_END + 3)

static const struct pinconf_param npcm8xx_conf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 1 },
	{ "input-enable", PIN_CONFIG_INPUT_ENABLE, 1 },
	{ "output-enable", PIN_CONFIG_OUTPUT_ENABLE, 1 },
	{ "output-high", PIN_CONFIG_OUTPUT, 1, },
	{ "output-low", PIN_CONFIG_OUTPUT, 0, },
	{ "drive-open-drain", PIN_CONFIG_DRIVE_OPEN_DRAIN, 1 },
	{ "drive-push-pull", PIN_CONFIG_DRIVE_PUSH_PULL, 1 },
	{ "persist-enable", PIN_CONFIG_PERSIST_STATE, 1 },
	{ "persist-disable", PIN_CONFIG_PERSIST_STATE, 0 },
	{ "input-debounce", PIN_CONFIG_INPUT_DEBOUNCE, 0 },
	{ "active-high", PIN_CONFIG_POLARITY_STATE, 0 },
	{ "active-low", PIN_CONFIG_POLARITY_STATE, 1 },
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 0 },
	{ "slew-rate", PIN_CONFIG_SLEW_RATE, 0},
	{ "event-clear", PIN_CONFIG_EVENT_CLEAR, 0},
};

static bool is_gpio_persist(struct udevice *dev, enum reset_type type, unsigned int bank)
{
	struct npcm8xx_pinctrl_priv *priv = dev_get_priv(dev);
	u32 base = priv->clk_base;

	u8 offset = bank + GPIOX_MODULE_RESET;
	u32 mask = 1 << offset;

	dev_dbg(dev, "reboot reason: 0x%x\n", type);

	switch (type) {
	case (PORST_TYPE):
		return false;
	case (CORST_TYPE):
		return !((readl((uintptr_t)(base + NPCM8XX_RST_CORSTC)) & mask) >> offset);
	case (WD0RST_TYPE):
		return !((readl((uintptr_t)(base + NPCM8XX_RST_WD0RCR)) & mask) >> offset);
	case (WD1RST_TYPE):
		return !((readl((uintptr_t)(base + NPCM8XX_RST_WD1RCR)) & mask) >> offset);
	case (WD2RST_TYPE):
		return !((readl((uintptr_t)(base + NPCM8XX_RST_WD2RCR)) & mask) >> offset);
	default:
		return false;
	}
}

static int npcm8xx_gpio_reset_persist(struct udevice *dev, unsigned int banknum,
				      unsigned int enable)
{
	struct npcm8xx_pinctrl_priv *priv = dev_get_priv(dev);
	u32 base = priv->clk_base;

	dev_dbg(dev, "set gpio persist, bank %d, enable %d\n", banknum, enable);

	if (enable) {
		clrbits_le32((uintptr_t)(base + NPCM8XX_RST_WD0RCR),
			     BIT(GPIOX_MODULE_RESET + banknum));
		clrbits_le32((uintptr_t)(base + NPCM8XX_RST_WD1RCR),
			     BIT(GPIOX_MODULE_RESET + banknum));
		clrbits_le32((uintptr_t)(base + NPCM8XX_RST_WD2RCR),
			     BIT(GPIOX_MODULE_RESET + banknum));
		clrbits_le32((uintptr_t)(base + NPCM8XX_RST_CORSTC),
			     BIT(GPIOX_MODULE_RESET + banknum));
	} else {
		setbits_le32((uintptr_t)(base + NPCM8XX_RST_WD0RCR),
			     BIT(GPIOX_MODULE_RESET + banknum) | CA9C_MODULE_RESET);
		setbits_le32((uintptr_t)(base + NPCM8XX_RST_WD1RCR),
			     BIT(GPIOX_MODULE_RESET + banknum) | CA9C_MODULE_RESET);
		setbits_le32((uintptr_t)(base + NPCM8XX_RST_WD2RCR),
			     BIT(GPIOX_MODULE_RESET + banknum) | CA9C_MODULE_RESET);
		setbits_le32((uintptr_t)(base + NPCM8XX_RST_CORSTC),
			     BIT(GPIOX_MODULE_RESET + banknum) | CA9C_MODULE_RESET);
	}

	return 0;
}

/* Set drive strength for a pin, if supported */
static int npcm8xx_set_drive_strength(struct udevice *dev,
				      unsigned int pin, unsigned int nval)
{
	struct npcm8xx_pinctrl_priv *priv = dev_get_priv(dev);
	unsigned int bank = pin / NPCM8XX_GPIO_PER_BANK;
	unsigned int gpio = (pin % NPCM8XX_GPIO_PER_BITS);
	u32 base = priv->gpio_base + (NPCM8XX_GPIO_BANK_OFFSET * bank);
	unsigned int v;

	v = (pincfgs[pin].flag & DRIVE_STRENGTH_MASK);
	if (!nval || !v)
		return -EOPNOTSUPP;

	if (DSLO(v) == nval) {
		dev_dbg(dev,
			"setting pin %d to low strength [%d]\n", pin, nval);
		clrbits_le32((uintptr_t)(base + NPCM8XX_GP_N_ODSC), BIT(gpio));
		return 0;
	} else if (DSHI(v) == nval) {
		dev_dbg(dev,
			"setting pin %d to high strength [%d]\n", pin, nval);
		setbits_le32((uintptr_t)(base + NPCM8XX_GP_N_ODSC), BIT(gpio));
		return 0;
	}

	return -EOPNOTSUPP;
}

/* Set slew rate of pin (high/low) */
static int npcm8xx_set_slew_rate(struct udevice *dev, unsigned int pin, unsigned int arg)
{
	struct npcm8xx_pinctrl_priv *priv = dev_get_priv(dev);
	unsigned int bank = pin / NPCM8XX_GPIO_PER_BANK;
	unsigned int gpio = (pin % NPCM8XX_GPIO_PER_BITS);
	u32 base = priv->gpio_base + (NPCM8XX_GPIO_BANK_OFFSET * bank);

	if (pincfgs[pin].flag & SLEW) {
		switch (arg) {
		case 0:
			dev_dbg(dev,
				"setting pin %d slew rate to low\n", pin);
			clrbits_le32((uintptr_t)(base + NPCM8XX_GP_N_OSRC), BIT(gpio));
			return 0;
		case 1:
			dev_dbg(dev,
				"setting pin %d slew rate to high\n", pin);
			setbits_le32((uintptr_t)(base + NPCM8XX_GP_N_OSRC), BIT(gpio));
			return 0;
		default:
			return -EOPNOTSUPP;
		}
	}

	/* LPC Slew rate in SRCNT register */
	if (pincfgs[pin].flag & SLEWLPC) {
		switch (arg) {
		case 0:
			dev_dbg(dev,
				"setting LPC/ESPI(%d) slew rate to low\n", pin);
			clrbits_le32((uintptr_t)(priv->gcr_base + NPCM8XX_GCR_SRCNT), SRCNT_ESPI);
			return 0;
		case 1:
			dev_dbg(dev,
				"setting LPC/ESPI(%d) slew rate to high\n", pin);
			setbits_le32((uintptr_t)(priv->gcr_base + NPCM8XX_GCR_SRCNT), SRCNT_ESPI);
			return 0;
		default:
			return -EOPNOTSUPP;
		}
	}

	return -EOPNOTSUPP;
}

static int npcm8xx_pinconf_set(struct udevice *dev, unsigned int pin,
			       unsigned int param, unsigned int arg)
{
	struct npcm8xx_pinctrl_priv *priv = dev_get_priv(dev);
	int err = 0;
	unsigned int bank = pin / NPCM8XX_GPIO_PER_BANK;
	unsigned int gpio = (pin % NPCM8XX_GPIO_PER_BITS);
	u32 base = priv->gpio_base + (0x1000 * bank);

	npcm8xx_setfunc(dev, (const unsigned int *)&pin, 1, fn_gpio);

	/* To prevent unexpected IRQ trap at verctor 00 in linux kernel */
	if (param == PIN_CONFIG_EVENT_CLEAR) {
		dev_dbg(dev, "set pin %d event clear\n", pin);
		clrbits_le32((uintptr_t)(base + NPCM8XX_GP_N_EVEN), BIT(gpio));
		setbits_le32((uintptr_t)(base + NPCM8XX_GP_N_EVST), BIT(gpio));
		return err;
	}

	// allow set persist state disable
	if (param == PIN_CONFIG_PERSIST_STATE) {
		npcm8xx_gpio_reset_persist(dev, bank, arg);
		return err;
	}

	if (is_gpio_persist(dev, npcm8xx_reset_reason(), bank))
		return err;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		dev_dbg(dev, "set pin %d bias dsiable\n", pin);
		clrbits_le32((uintptr_t)(base + NPCM8XX_GP_N_PU), BIT(gpio));
		clrbits_le32((uintptr_t)(base + NPCM8XX_GP_N_PD), BIT(gpio));
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		dev_dbg(dev, "set pin %d bias pull down\n", pin);
		clrbits_le32((uintptr_t)(base + NPCM8XX_GP_N_PU), BIT(gpio));
		setbits_le32((uintptr_t)(base + NPCM8XX_GP_N_PD), BIT(gpio));
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		dev_dbg(dev, "set pin %d bias pull up\n", pin);
		setbits_le32((uintptr_t)(base + NPCM8XX_GP_N_PU), BIT(gpio));
		clrbits_le32((uintptr_t)(base + NPCM8XX_GP_N_PD), BIT(gpio));
		break;
	case PIN_CONFIG_INPUT_ENABLE:
		dev_dbg(dev, "set pin %d input enable\n", pin);
		setbits_le32((uintptr_t)(base + NPCM8XX_GP_N_OEC), BIT(gpio));
		setbits_le32((uintptr_t)(base + NPCM8XX_GP_N_IEM), BIT(gpio));
		break;
	case PIN_CONFIG_OUTPUT_ENABLE:
		dev_dbg(dev, "set pin %d output enable\n", pin);
		clrbits_le32((uintptr_t)(base + NPCM8XX_GP_N_IEM), BIT(gpio));
		setbits_le32((uintptr_t)(base + NPCM8XX_GP_N_OES), BIT(gpio));
	case PIN_CONFIG_OUTPUT:
		dev_dbg(dev, "set pin %d output %d\n", pin, arg);
		clrbits_le32((uintptr_t)(base + NPCM8XX_GP_N_IEM), BIT(gpio));
		setbits_le32((uintptr_t)(base + NPCM8XX_GP_N_OES), BIT(gpio));
		if (arg)
			setbits_le32((uintptr_t)(base + NPCM8XX_GP_N_DOUT), BIT(gpio));
		else
			clrbits_le32((uintptr_t)(base + NPCM8XX_GP_N_DOUT), BIT(gpio));
		break;
	case PIN_CONFIG_DRIVE_PUSH_PULL:
		dev_dbg(dev, "set pin %d push pull\n", pin);
		clrbits_le32((uintptr_t)(base + NPCM8XX_GP_N_OTYP), BIT(gpio));
		break;
	case PIN_CONFIG_DRIVE_OPEN_DRAIN:
		dev_dbg(dev, "set pin %d open drain\n", pin);
		setbits_le32((uintptr_t)(base + NPCM8XX_GP_N_OTYP), BIT(gpio));
		break;
	case PIN_CONFIG_INPUT_DEBOUNCE:
		dev_dbg(dev, "set pin %d input debounce\n", pin);
		setbits_le32((uintptr_t)(base + NPCM8XX_GP_N_DBNC), BIT(gpio));
		break;
	case PIN_CONFIG_POLARITY_STATE:
		dev_dbg(dev, "set pin %d active %d\n", pin, arg);
		if (arg)
			setbits_le32((uintptr_t)(base + NPCM8XX_GP_N_POL), BIT(gpio));
		else
			clrbits_le32((uintptr_t)(base + NPCM8XX_GP_N_POL), BIT(gpio));
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		dev_dbg(dev, "set pin %d driver strength %d\n", pin, arg);
		err = npcm8xx_set_drive_strength(dev, pin, arg);
		break;
	case PIN_CONFIG_SLEW_RATE:
		dev_dbg(dev, "set pin %d slew rate %d\n", pin, arg);
		err = npcm8xx_set_slew_rate(dev, pin, arg);
		break;
	default:
		err = -EOPNOTSUPP;
	}
	return err;
}

static struct pinctrl_ops npcm8xx_pinctrl_ops = {
	.set_state	= pinctrl_generic_set_state,
	.get_pins_count = npcm8xx_get_pins_count,
	.get_pin_name = npcm8xx_get_pin_name,
	.get_groups_count = npcm8xx_get_groups_count,
	.get_group_name = npcm8xx_get_group_name,
	.get_functions_count = npcm8xx_get_functions_count,
	.get_function_name = npcm8xx_get_function_name,
	.pinmux_set = npcm8xx_pinmux_set,
	.pinmux_group_set = npcm8xx_pinmux_set,
	.pinconf_num_params = ARRAY_SIZE(npcm8xx_conf_params),
	.pinconf_params = npcm8xx_conf_params,
	.pinconf_set = npcm8xx_pinconf_set,
	.pinconf_group_set = npcm8xx_pinconf_set,
};

static const struct udevice_id npcm8xx_pinctrl_ids[] = {
	{ .compatible = "nuvoton,npcm845-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_npcm8xx) = {
	.name = "nuvoton_npcm8xx_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = npcm8xx_pinctrl_ids,
	.priv_auto = sizeof(struct npcm8xx_pinctrl_priv),
	.ops = &npcm8xx_pinctrl_ops,
	.probe = npcm8xx_pinctrl_probe,
};
