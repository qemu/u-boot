// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek clock driver for MT7986 SoC
 *
 * Copyright (C) 2022 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <dm.h>
#include <log.h>
#include <asm/arch-mediatek/reset.h>
#include <asm/io.h>
#include <dt-bindings/clock/mt7986-clk.h>
#include <linux/bitops.h>

#include "clk-mtk.h"

#define MT7986_CLK_PDN 0x250
#define MT7986_CLK_PDN_EN_WRITE BIT(31)

#define PLL_FACTOR(_id, _name, _parent, _mult, _div)                           \
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_APMIXED)

#define TOP_FACTOR(_id, _name, _parent, _mult, _div)                           \
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_TOPCKGEN)

#define INFRA_FACTOR(_id, _name, _parent, _mult, _div)                         \
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_INFRASYS)

/* FIXED PLLS */
static const struct mtk_fixed_clk fixed_pll_clks[] = {
	FIXED_CLK(CK_APMIXED_ARMPLL, CLK_XTAL, 2000000000),
	FIXED_CLK(CK_APMIXED_NET2PLL, CLK_XTAL, 800000000),
	FIXED_CLK(CK_APMIXED_MMPLL, CLK_XTAL, 1440000000),
	FIXED_CLK(CK_APMIXED_SGMPLL, CLK_XTAL, 325000000),
	FIXED_CLK(CK_APMIXED_WEDMCUPLL, CLK_XTAL, 760000000),
	FIXED_CLK(CK_APMIXED_NET1PLL, CLK_XTAL, 2500000000),
	FIXED_CLK(CK_APMIXED_MPLL, CLK_XTAL, 416000000),
	FIXED_CLK(CK_APMIXED_APLL2, CLK_XTAL, 196608000),
};

/* TOPCKGEN FIXED CLK */
static const struct mtk_fixed_clk top_fixed_clks[] = {
	FIXED_CLK(CK_TOP_CB_CKSQ_40M, CLK_XTAL, 40000000),
};

/* TOPCKGEN FIXED DIV */
static const struct mtk_fixed_factor top_fixed_divs[] = {
	PLL_FACTOR(CK_TOP_CB_M_416M, "cb_m_416m", CK_APMIXED_MPLL, 1, 1),
	PLL_FACTOR(CK_TOP_CB_M_D2, "cb_m_d2", CK_APMIXED_MPLL, 1, 2),
	PLL_FACTOR(CK_TOP_CB_M_D4, "cb_m_d4", CK_APMIXED_MPLL, 1, 4),
	PLL_FACTOR(CK_TOP_CB_M_D8, "cb_m_d8", CK_APMIXED_MPLL, 1, 8),
	PLL_FACTOR(CK_TOP_M_D8_D2, "m_d8_d2", CK_APMIXED_MPLL, 1, 16),
	PLL_FACTOR(CK_TOP_M_D3_D2, "m_d3_d2", CK_APMIXED_MPLL, 1, 2),
	PLL_FACTOR(CK_TOP_CB_MM_D2, "cb_mm_d2", CK_APMIXED_MMPLL, 1, 2),
	PLL_FACTOR(CK_TOP_CB_MM_D4, "cb_mm_d4", CK_APMIXED_MMPLL, 1, 4),
	PLL_FACTOR(CK_TOP_CB_MM_D8, "cb_mm_d8", CK_APMIXED_MMPLL, 1, 8),
	PLL_FACTOR(CK_TOP_MM_D8_D2, "mm_d8_d2", CK_APMIXED_MMPLL, 1, 16),
	PLL_FACTOR(CK_TOP_MM_D3_D8, "mm_d3_d8", CK_APMIXED_MMPLL, 1, 8),
	PLL_FACTOR(CK_TOP_CB_U2_PHYD_CK, "cb_u2_phyd", CK_APMIXED_MMPLL, 1, 30),
	PLL_FACTOR(CK_TOP_CB_APLL2_196M, "cb_apll2_196m", CK_APMIXED_APLL2, 1,
		   1),
	PLL_FACTOR(CK_TOP_APLL2_D4, "apll2_d4", CK_APMIXED_APLL2, 1, 4),
	PLL_FACTOR(CK_TOP_CB_NET1_D4, "cb_net1_d4", CK_APMIXED_NET1PLL, 1, 4),
	PLL_FACTOR(CK_TOP_CB_NET1_D5, "cb_net1_d5", CK_APMIXED_NET1PLL, 1, 5),
	PLL_FACTOR(CK_TOP_NET1_D5_D2, "net1_d5_d2", CK_APMIXED_NET1PLL, 1, 10),
	PLL_FACTOR(CK_TOP_NET1_D5_D4, "net1_d5_d4", CK_APMIXED_NET1PLL, 1, 20),
	PLL_FACTOR(CK_TOP_NET1_D8_D2, "net1_d8_d2", CK_APMIXED_NET1PLL, 1, 16),
	PLL_FACTOR(CK_TOP_NET1_D8_D4, "net1_d8_d4", CK_APMIXED_NET1PLL, 1, 32),
	PLL_FACTOR(CK_TOP_CB_NET2_800M, "cb_net2_800m", CK_APMIXED_NET2PLL, 1,
		   1),
	PLL_FACTOR(CK_TOP_CB_NET2_D4, "cb_net2_d4", CK_APMIXED_NET2PLL, 1, 4),
	PLL_FACTOR(CK_TOP_NET2_D4_D2, "net2_d4_d2", CK_APMIXED_NET2PLL, 1, 8),
	PLL_FACTOR(CK_TOP_NET2_D3_D2, "net2_d3_d2", CK_APMIXED_NET2PLL, 1, 2),
	PLL_FACTOR(CK_TOP_CB_WEDMCU_760M, "cb_wedmcu_760m",
		   CK_APMIXED_WEDMCUPLL, 1, 1),
	PLL_FACTOR(CK_TOP_WEDMCU_D5_D2, "wedmcu_d5_d2", CK_APMIXED_WEDMCUPLL, 1,
		   10),
	PLL_FACTOR(CK_TOP_CB_SGM_325M, "cb_sgm_325m", CK_APMIXED_SGMPLL, 1, 1),
	TOP_FACTOR(CK_TOP_CB_CKSQ_40M_D2, "cb_cksq_40m_d2", CK_TOP_CB_CKSQ_40M,
		   1, 2),
	TOP_FACTOR(CK_TOP_CB_RTC_32K, "cb_rtc_32k", CK_TOP_CB_CKSQ_40M, 1,
		   1250),
	TOP_FACTOR(CK_TOP_CB_RTC_32P7K, "cb_rtc_32p7k", CK_TOP_CB_CKSQ_40M, 1,
		   1220),
	TOP_FACTOR(CK_TOP_NFI1X, "nfi1x", CK_TOP_NFI1X_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_USB_EQ_RX250M, "usb_eq_rx250m", CK_TOP_CB_CKSQ_40M, 1,
		   1),
	TOP_FACTOR(CK_TOP_USB_TX250M, "usb_tx250m", CK_TOP_CB_CKSQ_40M, 1, 1),
	TOP_FACTOR(CK_TOP_USB_LN0_CK, "usb_ln0", CK_TOP_CB_CKSQ_40M, 1, 1),
	TOP_FACTOR(CK_TOP_USB_CDR_CK, "usb_cdr", CK_TOP_CB_CKSQ_40M, 1, 1),
	TOP_FACTOR(CK_TOP_SPINFI_BCK, "spinfi_bck", CK_TOP_SPINFI_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_I2C_BCK, "i2c_bck", CK_TOP_I2C_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_PEXTP_TL, "pextp_tl", CK_TOP_PEXTP_TL_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_EMMC_250M, "emmc_250m", CK_TOP_EMMC_250M_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_EMMC_416M, "emmc_416m", CK_TOP_EMMC_416M_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_F_26M_ADC_CK, "f_26m_adc", CK_TOP_F_26M_ADC_SEL, 1,
		   1),
	TOP_FACTOR(CK_TOP_SYSAXI, "sysaxi", CK_TOP_SYSAXI_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_NETSYS_WED_MCU, "netsys_wed_mcu",
		   CK_TOP_NETSYS_MCU_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_NETSYS_2X, "netsys_2x", CK_TOP_NETSYS_2X_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_SGM_325M, "sgm_325m", CK_TOP_SGM_325M_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_A1SYS, "a1sys", CK_TOP_A1SYS_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_EIP_B, "eip_b", CK_TOP_EIP_B_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_F26M, "csw_f26m", CK_TOP_F26M_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_AUD_L, "aud_l", CK_TOP_AUD_L_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_A_TUNER, "a_tuner", CK_TOP_A_TUNER_SEL, 2, 1),
	TOP_FACTOR(CK_TOP_U2U3_REF, "u2u3_ref", CK_TOP_U2U3_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_U2U3_SYS, "u2u3_sys", CK_TOP_U2U3_SYS_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_U2U3_XHCI, "u2u3_xhci", CK_TOP_U2U3_XHCI_SEL, 1, 1),
	TOP_FACTOR(CK_TOP_AP2CNN_HOST, "ap2cnn_host", CK_TOP_AP2CNN_HOST_SEL, 1,
		   1),
};

/* TOPCKGEN MUX PARENTS */
static const int nfi1x_parents[] = { CK_TOP_CB_CKSQ_40M,  CK_TOP_CB_MM_D8,
				     CK_TOP_NET1_D8_D2,   CK_TOP_NET2_D3_D2,
				     CK_TOP_CB_M_D4,      CK_TOP_MM_D8_D2,
				     CK_TOP_WEDMCU_D5_D2, CK_TOP_CB_M_D8 };

static const int spinfi_parents[] = {
	CK_TOP_CB_CKSQ_40M_D2, CK_TOP_CB_CKSQ_40M, CK_TOP_NET1_D5_D4,
	CK_TOP_CB_M_D4,	CK_TOP_MM_D8_D2,    CK_TOP_WEDMCU_D5_D2,
	CK_TOP_MM_D3_D8,       CK_TOP_CB_M_D8
};

static const int spi_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_CB_M_D2,
				   CK_TOP_CB_MM_D8,    CK_TOP_NET1_D8_D2,
				   CK_TOP_NET2_D3_D2,  CK_TOP_NET1_D5_D4,
				   CK_TOP_CB_M_D4,     CK_TOP_WEDMCU_D5_D2 };

static const int uart_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_CB_M_D8,
				    CK_TOP_M_D8_D2 };

static const int pwm_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_NET1_D8_D2,
				   CK_TOP_NET1_D5_D4, CK_TOP_CB_M_D4 };

static const int i2c_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_NET1_D5_D4,
				   CK_TOP_CB_M_D4, CK_TOP_NET1_D8_D4 };

static const int pextp_tl_ck_parents[] = { CK_TOP_CB_CKSQ_40M,
					   CK_TOP_NET1_D5_D4, CK_TOP_NET2_D4_D2,
					   CK_TOP_CB_RTC_32K };

static const int emmc_250m_parents[] = { CK_TOP_CB_CKSQ_40M,
					 CK_TOP_NET1_D5_D2 };

static const int emmc_416m_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_CB_M_416M };

static const int f_26m_adc_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_M_D8_D2 };

static const int dramc_md32_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_CB_M_D2 };

static const int sysaxi_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_NET1_D8_D2,
				      CK_TOP_CB_NET2_D4 };

static const int sysapb_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_M_D3_D2,
				      CK_TOP_NET2_D4_D2 };

static const int arm_db_main_parents[] = { CK_TOP_CB_CKSQ_40M,
					   CK_TOP_NET2_D3_D2 };

static const int arm_db_jtsel_parents[] = { -1, CK_TOP_CB_CKSQ_40M };

static const int netsys_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_CB_MM_D4 };

static const int netsys_500m_parents[] = { CK_TOP_CB_CKSQ_40M,
					   CK_TOP_CB_NET1_D5 };

static const int netsys_mcu_parents[] = { CK_TOP_CB_CKSQ_40M,
					  CK_TOP_CB_WEDMCU_760M,
					  CK_TOP_CB_MM_D2, CK_TOP_CB_NET1_D4,
					  CK_TOP_CB_NET1_D5 };

static const int netsys_2x_parents[] = { CK_TOP_CB_CKSQ_40M,
					 CK_TOP_CB_NET2_800M,
					 CK_TOP_CB_WEDMCU_760M,
					 CK_TOP_CB_MM_D2 };

static const int sgm_325m_parents[] = { CK_TOP_CB_CKSQ_40M,
					CK_TOP_CB_SGM_325M };

static const int sgm_reg_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_NET1_D8_D4 };

static const int a1sys_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_APLL2_D4 };

static const int conn_mcusys_parents[] = { CK_TOP_CB_CKSQ_40M,
					   CK_TOP_CB_MM_D2 };

static const int eip_b_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_CB_NET2_800M };

static const int aud_l_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_CB_APLL2_196M,
				     CK_TOP_M_D8_D2 };

static const int a_tuner_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_APLL2_D4,
				       CK_TOP_M_D8_D2 };

static const int u2u3_sys_parents[] = { CK_TOP_CB_CKSQ_40M, CK_TOP_NET1_D5_D4 };

static const int da_u2_refsel_parents[] = { CK_TOP_CB_CKSQ_40M,
					    CK_TOP_CB_U2_PHYD_CK };

#define TOP_MUX(_id, _name, _parents, _mux_ofs, _mux_set_ofs, _mux_clr_ofs,    \
		_shift, _width, _gate, _upd_ofs, _upd)                         \
	{                                                                      \
		.id = _id, .mux_reg = _mux_ofs, .mux_set_reg = _mux_set_ofs,   \
		.mux_clr_reg = _mux_clr_ofs, .upd_reg = _upd_ofs,              \
		.upd_shift = _upd, .mux_shift = _shift,                        \
		.mux_mask = BIT(_width) - 1, .gate_reg = _mux_ofs,             \
		.gate_shift = _gate, .parent = _parents,                       \
		.num_parents = ARRAY_SIZE(_parents),                           \
		.flags = CLK_MUX_SETCLR_UPD,                                   \
	}

/* TOPCKGEN MUX_GATE */
static const struct mtk_composite top_muxes[] = {
	/* CLK_CFG_0 */
	TOP_MUX(CK_TOP_NFI1X_SEL, "nfi1x_sel", nfi1x_parents, 0x000, 0x004,
		0x008, 0, 3, 7, 0x1C0, 0),
	TOP_MUX(CK_TOP_SPINFI_SEL, "spinfi_sel", spinfi_parents, 0x000, 0x004,
		0x008, 8, 3, 15, 0x1C0, 1),
	TOP_MUX(CK_TOP_SPI_SEL, "spi_sel", spi_parents, 0x000, 0x004, 0x008, 16,
		3, 23, 0x1C0, 2),
	TOP_MUX(CK_TOP_SPIM_MST_SEL, "spim_mst_sel", spi_parents, 0x000, 0x004,
		0x008, 24, 3, 31, 0x1C0, 3),
	/* CLK_CFG_1 */
	TOP_MUX(CK_TOP_UART_SEL, "uart_sel", uart_parents, 0x010, 0x014, 0x018,
		0, 2, 7, 0x1C0, 4),
	TOP_MUX(CK_TOP_PWM_SEL, "pwm_sel", pwm_parents, 0x010, 0x014, 0x018, 8,
		2, 15, 0x1C0, 5),
	TOP_MUX(CK_TOP_I2C_SEL, "i2c_sel", i2c_parents, 0x010, 0x014, 0x018, 16,
		2, 23, 0x1C0, 6),
	TOP_MUX(CK_TOP_PEXTP_TL_SEL, "pextp_tl_ck_sel", pextp_tl_ck_parents,
		0x010, 0x014, 0x018, 24, 2, 31, 0x1C0, 7),
	/* CLK_CFG_2 */
	TOP_MUX(CK_TOP_EMMC_250M_SEL, "emmc_250m_sel", emmc_250m_parents, 0x020,
		0x024, 0x028, 0, 1, 7, 0x1C0, 8),
	TOP_MUX(CK_TOP_EMMC_416M_SEL, "emmc_416m_sel", emmc_416m_parents, 0x020,
		0x024, 0x028, 8, 1, 15, 0x1C0, 9),
	TOP_MUX(CK_TOP_F_26M_ADC_SEL, "f_26m_adc_sel", f_26m_adc_parents, 0x020,
		0x024, 0x028, 16, 1, 23, 0x1C0, 10),
	TOP_MUX(CK_TOP_DRAMC_SEL, "dramc_sel", f_26m_adc_parents, 0x020, 0x024,
		0x028, 24, 1, 31, 0x1C0, 11),
	/* CLK_CFG_3 */
	TOP_MUX(CK_TOP_DRAMC_MD32_SEL, "dramc_md32_sel", dramc_md32_parents,
		0x030, 0x034, 0x038, 0, 1, 7, 0x1C0, 12),
	TOP_MUX(CK_TOP_SYSAXI_SEL, "sysaxi_sel", sysaxi_parents, 0x030, 0x034,
		0x038, 8, 2, 15, 0x1C0, 13),
	TOP_MUX(CK_TOP_SYSAPB_SEL, "sysapb_sel", sysapb_parents, 0x030, 0x034,
		0x038, 16, 2, 23, 0x1C0, 14),
	TOP_MUX(CK_TOP_ARM_DB_MAIN_SEL, "arm_db_main_sel", arm_db_main_parents,
		0x030, 0x034, 0x038, 24, 1, 31, 0x1C0, 15),
	/* CLK_CFG_4 */
	TOP_MUX(CK_TOP_ARM_DB_JTSEL, "arm_db_jtsel", arm_db_jtsel_parents,
		0x040, 0x044, 0x048, 0, 1, 7, 0x1C0, 16),
	TOP_MUX(CK_TOP_NETSYS_SEL, "netsys_sel", netsys_parents, 0x040, 0x044,
		0x048, 8, 1, 15, 0x1C0, 17),
	TOP_MUX(CK_TOP_NETSYS_500M_SEL, "netsys_500m_sel", netsys_500m_parents,
		0x040, 0x044, 0x048, 16, 1, 23, 0x1C0, 18),
	TOP_MUX(CK_TOP_NETSYS_MCU_SEL, "netsys_mcu_sel", netsys_mcu_parents,
		0x040, 0x044, 0x048, 24, 3, 31, 0x1C0, 19),
	/* CLK_CFG_5 */
	TOP_MUX(CK_TOP_NETSYS_2X_SEL, "netsys_2x_sel", netsys_2x_parents, 0x050,
		0x054, 0x058, 0, 2, 7, 0x1C0, 20),
	TOP_MUX(CK_TOP_SGM_325M_SEL, "sgm_325m_sel", sgm_325m_parents, 0x050,
		0x054, 0x058, 8, 1, 15, 0x1C0, 21),
	TOP_MUX(CK_TOP_SGM_REG_SEL, "sgm_reg_sel", sgm_reg_parents, 0x050,
		0x054, 0x058, 16, 1, 23, 0x1C0, 22),
	TOP_MUX(CK_TOP_A1SYS_SEL, "a1sys_sel", a1sys_parents, 0x050, 0x054,
		0x058, 24, 1, 31, 0x1C0, 23),
	/* CLK_CFG_6 */
	TOP_MUX(CK_TOP_CONN_MCUSYS_SEL, "conn_mcusys_sel", conn_mcusys_parents,
		0x060, 0x064, 0x068, 0, 1, 7, 0x1C0, 24),
	TOP_MUX(CK_TOP_EIP_B_SEL, "eip_b_sel", eip_b_parents, 0x060, 0x064,
		0x068, 8, 1, 15, 0x1C0, 25),
	TOP_MUX(CK_TOP_PCIE_PHY_SEL, "pcie_phy_sel", f_26m_adc_parents, 0x060,
		0x064, 0x068, 16, 1, 23, 0x1C0, 26),
	TOP_MUX(CK_TOP_USB3_PHY_SEL, "usb3_phy_sel", f_26m_adc_parents, 0x060,
		0x064, 0x068, 24, 1, 31, 0x1C0, 27),
	/* CLK_CFG_7 */
	TOP_MUX(CK_TOP_F26M_SEL, "csw_f26m_sel", f_26m_adc_parents, 0x070,
		0x074, 0x078, 0, 1, 7, 0x1C0, 28),
	TOP_MUX(CK_TOP_AUD_L_SEL, "aud_l_sel", aud_l_parents, 0x070, 0x074,
		0x078, 8, 2, 15, 0x1C0, 29),
	TOP_MUX(CK_TOP_A_TUNER_SEL, "a_tuner_sel", a_tuner_parents, 0x070,
		0x074, 0x078, 16, 2, 23, 0x1C0, 30),
	TOP_MUX(CK_TOP_U2U3_SEL, "u2u3_sel", f_26m_adc_parents, 0x070, 0x074,
		0x078, 24, 1, 31, 0x1C4, 0),
	/* CLK_CFG_8 */
	TOP_MUX(CK_TOP_U2U3_SYS_SEL, "u2u3_sys_sel", u2u3_sys_parents, 0x080,
		0x084, 0x088, 0, 1, 7, 0x1C4, 1),
	TOP_MUX(CK_TOP_U2U3_XHCI_SEL, "u2u3_xhci_sel", u2u3_sys_parents, 0x080,
		0x084, 0x088, 8, 1, 15, 0x1C4, 2),
	TOP_MUX(CK_TOP_DA_U2_REFSEL, "da_u2_refsel", da_u2_refsel_parents,
		0x080, 0x084, 0x088, 16, 1, 23, 0x1C4, 3),
	TOP_MUX(CK_TOP_DA_U2_CK_1P_SEL, "da_u2_ck_1p_sel", da_u2_refsel_parents,
		0x080, 0x084, 0x088, 24, 1, 31, 0x1C4, 4),
	/* CLK_CFG_9 */
	TOP_MUX(CK_TOP_AP2CNN_HOST_SEL, "ap2cnn_host_sel", sgm_reg_parents,
		0x090, 0x094, 0x098, 0, 1, 7, 0x1C4, 5),
};

/* INFRA FIXED DIV */
static const struct mtk_fixed_factor infra_fixed_divs[] = {
	TOP_FACTOR(CK_INFRA_CK_F26M, "infra_ck_f26m", CK_TOP_F26M_SEL, 1, 1),
	TOP_FACTOR(CK_INFRA_UART, "infra_uart", CK_TOP_UART_SEL, 1, 1),
	TOP_FACTOR(CK_INFRA_ISPI0, "infra_ispi0", CK_TOP_SPI_SEL, 1, 1),
	TOP_FACTOR(CK_INFRA_I2C, "infra_i2c", CK_TOP_I2C_SEL, 1, 1),
	TOP_FACTOR(CK_INFRA_ISPI1, "infra_ispi1", CK_TOP_SPINFI_SEL, 1, 1),
	TOP_FACTOR(CK_INFRA_PWM, "infra_pwm", CK_TOP_PWM_SEL, 1, 1),
	TOP_FACTOR(CK_INFRA_66M_MCK, "infra_66m_mck", CK_TOP_SYSAXI_SEL, 1, 2),
	TOP_FACTOR(CK_INFRA_CK_F32K, "infra_ck_f32k", CK_TOP_CB_RTC_32P7K, 1,
		   1),
	TOP_FACTOR(CK_INFRA_PCIE_CK, "infra_pcie", CK_TOP_PEXTP_TL_SEL, 1, 1),
	INFRA_FACTOR(CK_INFRA_PWM_BCK, "infra_pwm_bck", CK_INFRA_PWM_BSEL, 1,
		     1),
	INFRA_FACTOR(CK_INFRA_PWM_CK1, "infra_pwm_ck1", CK_INFRA_PWM1_SEL, 1,
		     1),
	INFRA_FACTOR(CK_INFRA_PWM_CK2, "infra_pwm_ck2", CK_INFRA_PWM2_SEL, 1,
		     1),
	TOP_FACTOR(CK_INFRA_133M_HCK, "infra_133m_hck", CK_TOP_SYSAXI, 1, 1),
	TOP_FACTOR(CK_INFRA_EIP_CK, "infra_eip", CK_TOP_EIP_B, 1, 1),
	INFRA_FACTOR(CK_INFRA_66M_PHCK, "infra_66m_phck", CK_INFRA_133M_HCK, 1,
		     1),
	TOP_FACTOR(CK_INFRA_FAUD_L_CK, "infra_faud_l", CK_TOP_AUD_L, 1, 1),
	TOP_FACTOR(CK_INFRA_FAUD_AUD_CK, "infra_faud_aud", CK_TOP_A1SYS, 1, 1),
	TOP_FACTOR(CK_INFRA_FAUD_EG2_CK, "infra_faud_eg2", CK_TOP_A_TUNER, 1,
		   1),
	TOP_FACTOR(CK_INFRA_I2CS_CK, "infra_i2cs", CK_TOP_I2C_BCK, 1, 1),
	INFRA_FACTOR(CK_INFRA_MUX_UART0, "infra_mux_uart0", CK_INFRA_UART0_SEL,
		     1, 1),
	INFRA_FACTOR(CK_INFRA_MUX_UART1, "infra_mux_uart1", CK_INFRA_UART1_SEL,
		     1, 1),
	INFRA_FACTOR(CK_INFRA_MUX_UART2, "infra_mux_uart2", CK_INFRA_UART2_SEL,
		     1, 1),
	TOP_FACTOR(CK_INFRA_NFI_CK, "infra_nfi", CK_TOP_NFI1X, 1, 1),
	TOP_FACTOR(CK_INFRA_SPINFI_CK, "infra_spinfi", CK_TOP_SPINFI_BCK, 1, 1),
	INFRA_FACTOR(CK_INFRA_MUX_SPI0, "infra_mux_spi0", CK_INFRA_SPI0_SEL, 1,
		     1),
	INFRA_FACTOR(CK_INFRA_MUX_SPI1, "infra_mux_spi1", CK_INFRA_SPI1_SEL, 1,
		     1),
	TOP_FACTOR(CK_INFRA_RTC_32K, "infra_rtc_32k", CK_TOP_CB_RTC_32K, 1, 1),
	TOP_FACTOR(CK_INFRA_FMSDC_CK, "infra_fmsdc", CK_TOP_EMMC_416M, 1, 1),
	TOP_FACTOR(CK_INFRA_FMSDC_HCK_CK, "infra_fmsdc_hck", CK_TOP_EMMC_250M,
		   1, 1),
	TOP_FACTOR(CK_INFRA_PERI_133M, "infra_peri_133m", CK_TOP_SYSAXI, 1, 1),
	TOP_FACTOR(CK_INFRA_133M_PHCK, "infra_133m_phck", CK_TOP_SYSAXI, 1, 1),
	TOP_FACTOR(CK_INFRA_USB_SYS_CK, "infra_usb_sys", CK_TOP_U2U3_SYS, 1, 1),
	TOP_FACTOR(CK_INFRA_USB_CK, "infra_usb", CK_TOP_U2U3_REF, 1, 1),
	TOP_FACTOR(CK_INFRA_USB_XHCI_CK, "infra_usb_xhci", CK_TOP_U2U3_XHCI, 1,
		   1),
	TOP_FACTOR(CK_INFRA_PCIE_GFMUX_TL_O_PRE, "infra_pcie_mux",
		   CK_TOP_PEXTP_TL, 1, 1),
	TOP_FACTOR(CK_INFRA_F26M_CK0, "infra_f26m_ck0", CK_TOP_F26M, 1, 1),
	TOP_FACTOR(CK_INFRA_HD_133M, "infra_hd_133m", CK_TOP_SYSAXI, 1, 1),
};

/* INFRASYS MUX PARENTS */
static const int infra_uart0_parents[] = { CK_INFRA_CK_F26M, CK_INFRA_UART };

static const int infra_spi0_parents[] = { CK_INFRA_I2C, CK_INFRA_ISPI0 };

static const int infra_spi1_parents[] = { CK_INFRA_I2C, CK_INFRA_ISPI1 };

static const int infra_pwm_bsel_parents[] = { CK_INFRA_CK_F32K,
					      CK_INFRA_CK_F26M,
					      CK_INFRA_66M_MCK, CK_INFRA_PWM };

static const int infra_pcie_parents[] = { CK_INFRA_CK_F32K, CK_INFRA_CK_F26M,
					  -1, CK_INFRA_PCIE_CK };

#define INFRA_MUX(_id, _name, _parents, _reg, _shift, _width)                  \
	{                                                                      \
		.id = _id, .mux_reg = (_reg) + 0x8,                            \
		.mux_set_reg = (_reg) + 0x0, .mux_clr_reg = (_reg) + 0x4,      \
		.mux_shift = _shift, .mux_mask = BIT(_width) - 1,              \
		.parent = _parents, .num_parents = ARRAY_SIZE(_parents),       \
		.flags = CLK_MUX_SETCLR_UPD | CLK_PARENT_INFRASYS,             \
	}

/* INFRA MUX */

static const struct mtk_composite infra_muxes[] = {
	/* MODULE_CLK_SEL_0 */
	INFRA_MUX(CK_INFRA_UART0_SEL, "infra_uart0_sel", infra_uart0_parents,
		  0x10, 0, 1),
	INFRA_MUX(CK_INFRA_UART1_SEL, "infra_uart1_sel", infra_uart0_parents,
		  0x10, 1, 1),
	INFRA_MUX(CK_INFRA_UART2_SEL, "infra_uart2_sel", infra_uart0_parents,
		  0x10, 2, 1),
	INFRA_MUX(CK_INFRA_SPI0_SEL, "infra_spi0_sel", infra_spi0_parents, 0x10,
		  4, 1),
	INFRA_MUX(CK_INFRA_SPI1_SEL, "infra_spi1_sel", infra_spi1_parents, 0x10,
		  5, 1),
	INFRA_MUX(CK_INFRA_PWM1_SEL, "infra_pwm1_sel", infra_pwm_bsel_parents,
		  0x10, 9, 2),
	INFRA_MUX(CK_INFRA_PWM2_SEL, "infra_pwm2_sel", infra_pwm_bsel_parents,
		  0x10, 11, 2),
	INFRA_MUX(CK_INFRA_PWM_BSEL, "infra_pwm_bsel", infra_pwm_bsel_parents,
		  0x10, 13, 2),
	/* MODULE_CLK_SEL_1 */
	INFRA_MUX(CK_INFRA_PCIE_SEL, "infra_pcie_sel", infra_pcie_parents, 0x20,
		  0, 2),
};

static const struct mtk_gate_regs infra_0_cg_regs = {
	.set_ofs = 0x40,
	.clr_ofs = 0x44,
	.sta_ofs = 0x48,
};

static const struct mtk_gate_regs infra_1_cg_regs = {
	.set_ofs = 0x50,
	.clr_ofs = 0x54,
	.sta_ofs = 0x58,
};

static const struct mtk_gate_regs infra_2_cg_regs = {
	.set_ofs = 0x60,
	.clr_ofs = 0x64,
	.sta_ofs = 0x68,
};

#define GATE_INFRA0(_id, _name, _parent, _shift)                               \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &infra_0_cg_regs,        \
		.shift = _shift,                                               \
		.flags = CLK_GATE_SETCLR | CLK_PARENT_INFRASYS,                \
	}

#define GATE_INFRA1(_id, _name, _parent, _shift)                               \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &infra_1_cg_regs,        \
		.shift = _shift,                                               \
		.flags = CLK_GATE_SETCLR | CLK_PARENT_INFRASYS,                \
	}

#define GATE_INFRA2(_id, _name, _parent, _shift)                               \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &infra_2_cg_regs,        \
		.shift = _shift,                                               \
		.flags = CLK_GATE_SETCLR | CLK_PARENT_INFRASYS,                \
	}

/* INFRA GATE */

static const struct mtk_gate infracfg_ao_gates[] = {
	/* INFRA0 */
	GATE_INFRA0(CK_INFRA_GPT_STA, "infra_gpt_sta", CK_INFRA_66M_MCK, 0),
	GATE_INFRA0(CK_INFRA_PWM_HCK, "infra_pwm_hck", CK_INFRA_66M_MCK, 1),
	GATE_INFRA0(CK_INFRA_PWM_STA, "infra_pwm_sta", CK_INFRA_PWM_BCK, 2),
	GATE_INFRA0(CK_INFRA_PWM1_CK, "infra_pwm1", CK_INFRA_PWM_CK1, 3),
	GATE_INFRA0(CK_INFRA_PWM2_CK, "infra_pwm2", CK_INFRA_PWM_CK2, 4),
	GATE_INFRA0(CK_INFRA_CQ_DMA_CK, "infra_cq_dma", CK_INFRA_133M_HCK, 6),
	GATE_INFRA0(CK_INFRA_EIP97_CK, "infra_eip97", CK_INFRA_EIP_CK, 7),
	GATE_INFRA0(CK_INFRA_AUD_BUS_CK, "infra_aud_bus", CK_INFRA_66M_PHCK, 8),
	GATE_INFRA0(CK_INFRA_AUD_26M_CK, "infra_aud_26m", CK_INFRA_CK_F26M, 9),
	GATE_INFRA0(CK_INFRA_AUD_L_CK, "infra_aud_l", CK_INFRA_FAUD_L_CK, 10),
	GATE_INFRA0(CK_INFRA_AUD_AUD_CK, "infra_aud_aud", CK_INFRA_FAUD_AUD_CK,
		    11),
	GATE_INFRA0(CK_INFRA_AUD_EG2_CK, "infra_aud_eg2", CK_INFRA_FAUD_EG2_CK,
		    13),
	GATE_INFRA0(CK_INFRA_DRAMC_26M_CK, "infra_dramc_26m", CK_INFRA_CK_F26M,
		    14),
	GATE_INFRA0(CK_INFRA_DBG_CK, "infra_dbg", CK_INFRA_66M_MCK, 15),
	GATE_INFRA0(CK_INFRA_AP_DMA_CK, "infra_ap_dma", CK_INFRA_66M_MCK, 16),
	GATE_INFRA0(CK_INFRA_SEJ_CK, "infra_sej", CK_INFRA_66M_MCK, 24),
	GATE_INFRA0(CK_INFRA_SEJ_13M_CK, "infra_sej_13m", CK_INFRA_CK_F26M, 25),
	GATE_INFRA0(CK_INFRA_TRNG_CK, "infra_trng", CK_INFRA_HD_133M, 26),
	/* INFRA1 */
	GATE_INFRA1(CK_INFRA_THERM_CK, "infra_therm", CK_INFRA_CK_F26M, 0),
	GATE_INFRA1(CK_INFRA_I2CO_CK, "infra_i2co", CK_INFRA_I2CS_CK, 1),
	GATE_INFRA1(CK_INFRA_UART0_CK, "infra_uart0", CK_INFRA_MUX_UART0, 2),
	GATE_INFRA1(CK_INFRA_UART1_CK, "infra_uart1", CK_INFRA_MUX_UART1, 3),
	GATE_INFRA1(CK_INFRA_UART2_CK, "infra_uart2", CK_INFRA_MUX_UART2, 4),
	GATE_INFRA1(CK_INFRA_NFI1_CK, "infra_nfi1", CK_INFRA_NFI_CK, 8),
	GATE_INFRA1(CK_INFRA_SPINFI1_CK, "infra_spinfi1", CK_INFRA_SPINFI_CK,
		    9),
	GATE_INFRA1(CK_INFRA_NFI_HCK_CK, "infra_nfi_hck", CK_INFRA_66M_MCK, 10),
	GATE_INFRA1(CK_INFRA_SPI0_CK, "infra_spi0", CK_INFRA_MUX_SPI0, 11),
	GATE_INFRA1(CK_INFRA_SPI1_CK, "infra_spi1", CK_INFRA_MUX_SPI1, 12),
	GATE_INFRA1(CK_INFRA_SPI0_HCK_CK, "infra_spi0_hck", CK_INFRA_66M_MCK,
		    13),
	GATE_INFRA1(CK_INFRA_SPI1_HCK_CK, "infra_spi1_hck", CK_INFRA_66M_MCK,
		    14),
	GATE_INFRA1(CK_INFRA_FRTC_CK, "infra_frtc", CK_INFRA_RTC_32K, 15),
	GATE_INFRA1(CK_INFRA_MSDC_CK, "infra_msdc", CK_INFRA_FMSDC_CK, 16),
	GATE_INFRA1(CK_INFRA_MSDC_HCK_CK, "infra_msdc_hck",
		    CK_INFRA_FMSDC_HCK_CK, 17),
	GATE_INFRA1(CK_INFRA_MSDC_133M_CK, "infra_msdc_133m",
		    CK_INFRA_PERI_133M, 18),
	GATE_INFRA1(CK_INFRA_MSDC_66M_CK, "infra_msdc_66m", CK_INFRA_66M_PHCK,
		    19),
	GATE_INFRA1(CK_INFRA_ADC_26M_CK, "infra_adc_26m", CK_INFRA_CK_F26M, 20),
	GATE_INFRA1(CK_INFRA_ADC_FRC_CK, "infra_adc_frc", CK_INFRA_CK_F26M, 21),
	GATE_INFRA1(CK_INFRA_FBIST2FPC_CK, "infra_fbist2fpc", CK_INFRA_NFI_CK,
		    23),
	/* INFRA2 */
	GATE_INFRA2(CK_INFRA_IUSB_133_CK, "infra_iusb_133", CK_INFRA_133M_PHCK,
		    0),
	GATE_INFRA2(CK_INFRA_IUSB_66M_CK, "infra_iusb_66m", CK_INFRA_66M_PHCK,
		    1),
	GATE_INFRA2(CK_INFRA_IUSB_SYS_CK, "infra_iusb_sys", CK_INFRA_USB_SYS_CK,
		    2),
	GATE_INFRA2(CK_INFRA_IUSB_CK, "infra_iusb", CK_INFRA_USB_CK, 3),
	GATE_INFRA2(CK_INFRA_IPCIE_CK, "infra_ipcie", CK_INFRA_PCIE_CK, 13),
	GATE_INFRA2(CK_INFRA_IPCIER_CK, "infra_ipcier", CK_INFRA_F26M_CK0, 15),
	GATE_INFRA2(CK_INFRA_IPCIEB_CK, "infra_ipcieb", CK_INFRA_133M_PHCK, 15),
};

static const struct mtk_clk_tree mt7986_fixed_pll_clk_tree = {
	.fdivs_offs = CLK_APMIXED_NR_CLK,
	.xtal_rate = 40 * MHZ,
	.fclks = fixed_pll_clks,
};

static const struct mtk_clk_tree mt7986_topckgen_clk_tree = {
	.fdivs_offs = CK_TOP_CB_M_416M,
	.muxes_offs = CK_TOP_NFI1X_SEL,
	.fclks = top_fixed_clks,
	.fdivs = top_fixed_divs,
	.muxes = top_muxes,
	.flags = CLK_BYPASS_XTAL,
};

static const struct mtk_clk_tree mt7986_infracfg_clk_tree = {
	.fdivs_offs = CK_INFRA_CK_F26M,
	.muxes_offs = CK_INFRA_UART0_SEL,
	.fdivs = infra_fixed_divs,
	.muxes = infra_muxes,
};

static const struct udevice_id mt7986_fixed_pll_compat[] = {
	{ .compatible = "mediatek,mt7986-fixed-plls" },
	{}
};

static const struct udevice_id mt7986_topckgen_compat[] = {
	{ .compatible = "mediatek,mt7986-topckgen" },
	{}
};

static int mt7986_fixed_pll_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt7986_fixed_pll_clk_tree);
}

static int mt7986_topckgen_probe(struct udevice *dev)
{
	struct mtk_clk_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	writel(MT7986_CLK_PDN_EN_WRITE, priv->base + MT7986_CLK_PDN);

	return mtk_common_clk_init(dev, &mt7986_topckgen_clk_tree);
}

U_BOOT_DRIVER(mtk_clk_apmixedsys) = {
	.name = "mt7986-clock-fixed-pll",
	.id = UCLASS_CLK,
	.of_match = mt7986_fixed_pll_compat,
	.probe = mt7986_fixed_pll_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen) = {
	.name = "mt7986-clock-topckgen",
	.id = UCLASS_CLK,
	.of_match = mt7986_topckgen_compat,
	.probe = mt7986_topckgen_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

static const struct udevice_id mt7986_infracfg_compat[] = {
	{ .compatible = "mediatek,mt7986-infracfg" },
	{}
};

static const struct udevice_id mt7986_infracfg_ao_compat[] = {
	{ .compatible = "mediatek,mt7986-infracfg_ao" },
	{}
};

static int mt7986_infracfg_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt7986_infracfg_clk_tree);
}

static int mt7986_infracfg_ao_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7986_infracfg_clk_tree,
					infracfg_ao_gates);
}

U_BOOT_DRIVER(mtk_clk_infracfg) = {
	.name = "mt7986-clock-infracfg",
	.id = UCLASS_CLK,
	.of_match = mt7986_infracfg_compat,
	.probe = mt7986_infracfg_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_infrasys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_infracfg_ao) = {
	.name = "mt7986-clock-infracfg-ao",
	.id = UCLASS_CLK,
	.of_match = mt7986_infracfg_ao_compat,
	.probe = mt7986_infracfg_ao_probe,
	.priv_auto = sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* ethsys */
static const struct mtk_gate_regs eth_cg_regs = {
	.sta_ofs = 0x30,
};

#define GATE_ETH(_id, _name, _parent, _shift)                                  \
	{                                                                      \
		.id = _id, .parent = _parent, .regs = &eth_cg_regs,            \
		.shift = _shift,                                               \
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_TOPCKGEN,         \
	}

static const struct mtk_gate eth_cgs[] = {
	GATE_ETH(CK_ETH_FE_EN, "eth_fe_en", CK_TOP_NETSYS_2X, 7),
	GATE_ETH(CK_ETH_GP2_EN, "eth_gp2_en", CK_TOP_SGM_325M, 8),
	GATE_ETH(CK_ETH_GP1_EN, "eth_gp1_en", CK_TOP_SGM_325M, 8),
	GATE_ETH(CK_ETH_WOCPU1_EN, "eth_wocpu1_en", CK_TOP_NETSYS_WED_MCU, 14),
	GATE_ETH(CK_ETH_WOCPU0_EN, "eth_wocpu0_en", CK_TOP_NETSYS_WED_MCU, 15),
};

static int mt7986_ethsys_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt7986_topckgen_clk_tree,
					eth_cgs);
}

static int mt7986_ethsys_bind(struct udevice *dev)
{
	int ret = 0;

	if (CONFIG_IS_ENABLED(RESET_MEDIATEK)) {
		ret = mediatek_reset_bind(dev, ETHSYS_HIFSYS_RST_CTRL_OFS, 1);
		if (ret)
			debug("Warning: failed to bind reset controller\n");
	}

	return ret;
}

static const struct udevice_id mt7986_ethsys_compat[] = {
	{ .compatible = "mediatek,mt7986-ethsys" },
	{ }
};

U_BOOT_DRIVER(mtk_clk_ethsys) = {
	.name = "mt7986-clock-ethsys",
	.id = UCLASS_CLK,
	.of_match = mt7986_ethsys_compat,
	.probe = mt7986_ethsys_probe,
	.bind = mt7986_ethsys_bind,
	.priv_auto = sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
};
