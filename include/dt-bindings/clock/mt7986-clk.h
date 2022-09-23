/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#ifndef _DT_BINDINGS_CLK_MT7986_H
#define _DT_BINDINGS_CLK_MT7986_H

/* INFRACFG */

#define CK_INFRA_CK_F26M		0
#define CK_INFRA_UART			1
#define CK_INFRA_ISPI0			2
#define CK_INFRA_I2C			3
#define CK_INFRA_ISPI1			4
#define CK_INFRA_PWM			5
#define CK_INFRA_66M_MCK		6
#define CK_INFRA_CK_F32K		7
#define CK_INFRA_PCIE_CK		8
#define CK_INFRA_PWM_BCK		9
#define CK_INFRA_PWM_CK1		10
#define CK_INFRA_PWM_CK2		11
#define CK_INFRA_133M_HCK		12
#define CK_INFRA_EIP_CK			13
#define CK_INFRA_66M_PHCK		14
#define CK_INFRA_FAUD_L_CK		15
#define CK_INFRA_FAUD_AUD_CK		17
#define CK_INFRA_FAUD_EG2_CK		17
#define CK_INFRA_I2CS_CK		18
#define CK_INFRA_MUX_UART0		19
#define CK_INFRA_MUX_UART1		20
#define CK_INFRA_MUX_UART2		21
#define CK_INFRA_NFI_CK			22
#define CK_INFRA_SPINFI_CK		23
#define CK_INFRA_MUX_SPI0		24
#define CK_INFRA_MUX_SPI1		25
#define CK_INFRA_RTC_32K		26
#define CK_INFRA_FMSDC_CK		27
#define CK_INFRA_FMSDC_HCK_CK		28
#define CK_INFRA_PERI_133M		29
#define CK_INFRA_133M_PHCK		30
#define CK_INFRA_USB_SYS_CK		31
#define CK_INFRA_USB_CK			32
#define CK_INFRA_USB_XHCI_CK		33
#define CK_INFRA_PCIE_GFMUX_TL_O_PRE	34
#define CK_INFRA_F26M_CK0		35
#define CK_INFRA_HD_133M		36
#define CLK_INFRA_NR_CLK		37

/* TOPCKGEN */

#define CK_TOP_CB_CKSQ_40M		0
#define CK_TOP_CB_M_416M		1
#define CK_TOP_CB_M_D2			2
#define CK_TOP_CB_M_D4			3
#define CK_TOP_CB_M_D8			4
#define CK_TOP_M_D8_D2			5
#define CK_TOP_M_D3_D2			6
#define CK_TOP_CB_MM_D2			7
#define CK_TOP_CB_MM_D4			8
#define CK_TOP_CB_MM_D8			9
#define CK_TOP_MM_D8_D2			10
#define CK_TOP_MM_D3_D8			11
#define CK_TOP_CB_U2_PHYD_CK		12
#define CK_TOP_CB_APLL2_196M		13
#define CK_TOP_APLL2_D4			14
#define CK_TOP_CB_NET1_D4		15
#define CK_TOP_CB_NET1_D5		16
#define CK_TOP_NET1_D5_D2		17
#define CK_TOP_NET1_D5_D4		18
#define CK_TOP_NET1_D8_D2		19
#define CK_TOP_NET1_D8_D4		20
#define CK_TOP_CB_NET2_800M		21
#define CK_TOP_CB_NET2_D4		22
#define CK_TOP_NET2_D4_D2		23
#define CK_TOP_NET2_D3_D2		24
#define CK_TOP_CB_WEDMCU_760M		25
#define CK_TOP_WEDMCU_D5_D2		26
#define CK_TOP_CB_SGM_325M		27
#define CK_TOP_CB_CKSQ_40M_D2		28
#define CK_TOP_CB_RTC_32K		29
#define CK_TOP_CB_RTC_32P7K		30
#define CK_TOP_NFI1X			31
#define CK_TOP_USB_EQ_RX250M		32
#define CK_TOP_USB_TX250M		33
#define CK_TOP_USB_LN0_CK		34
#define CK_TOP_USB_CDR_CK		35
#define CK_TOP_SPINFI_BCK		36
#define CK_TOP_I2C_BCK			37
#define CK_TOP_PEXTP_TL			38
#define CK_TOP_EMMC_250M		39
#define CK_TOP_EMMC_416M		40
#define CK_TOP_F_26M_ADC_CK		41
#define CK_TOP_SYSAXI			42
#define CK_TOP_NETSYS_WED_MCU		43
#define CK_TOP_NETSYS_2X		44
#define CK_TOP_SGM_325M			45
#define CK_TOP_A1SYS			46
#define CK_TOP_EIP_B			47
#define CK_TOP_F26M			48
#define CK_TOP_AUD_L			49
#define CK_TOP_A_TUNER			50
#define CK_TOP_U2U3_REF			51
#define CK_TOP_U2U3_SYS			52
#define CK_TOP_U2U3_XHCI		53
#define CK_TOP_AP2CNN_HOST		54
#define CK_TOP_NFI1X_SEL		55
#define CK_TOP_SPINFI_SEL		56
#define CK_TOP_SPI_SEL			57
#define CK_TOP_SPIM_MST_SEL		58
#define CK_TOP_UART_SEL			59
#define CK_TOP_PWM_SEL			60
#define CK_TOP_I2C_SEL			61
#define CK_TOP_PEXTP_TL_SEL		62
#define CK_TOP_EMMC_250M_SEL		63
#define CK_TOP_EMMC_416M_SEL		64
#define CK_TOP_F_26M_ADC_SEL		65
#define CK_TOP_DRAMC_SEL		66
#define CK_TOP_DRAMC_MD32_SEL		67
#define CK_TOP_SYSAXI_SEL		68
#define CK_TOP_SYSAPB_SEL		69
#define CK_TOP_ARM_DB_MAIN_SEL		70
#define CK_TOP_ARM_DB_JTSEL		71
#define CK_TOP_NETSYS_SEL		72
#define CK_TOP_NETSYS_500M_SEL		73
#define CK_TOP_NETSYS_MCU_SEL		74
#define CK_TOP_NETSYS_2X_SEL		75
#define CK_TOP_SGM_325M_SEL		76
#define CK_TOP_SGM_REG_SEL		77
#define CK_TOP_A1SYS_SEL		78
#define CK_TOP_CONN_MCUSYS_SEL		79
#define CK_TOP_EIP_B_SEL		80
#define CK_TOP_PCIE_PHY_SEL		81
#define CK_TOP_USB3_PHY_SEL		82
#define CK_TOP_F26M_SEL			83
#define CK_TOP_AUD_L_SEL		84
#define CK_TOP_A_TUNER_SEL		85
#define CK_TOP_U2U3_SEL			86
#define CK_TOP_U2U3_SYS_SEL		87
#define CK_TOP_U2U3_XHCI_SEL		88
#define CK_TOP_DA_U2_REFSEL		89
#define CK_TOP_DA_U2_CK_1P_SEL		90
#define CK_TOP_AP2CNN_HOST_SEL		91
#define CLK_TOP_NR_CLK			92

/*
 * INFRACFG_AO
 * clock muxes need to be append to infracfg domain, and clock gates
 * need to be keep in infracgh_ao domain
 */

#define CK_INFRA_UART0_SEL		(0 + CLK_INFRA_NR_CLK)
#define CK_INFRA_UART1_SEL		(1 + CLK_INFRA_NR_CLK)
#define CK_INFRA_UART2_SEL		(2 + CLK_INFRA_NR_CLK)
#define CK_INFRA_SPI0_SEL		(3 + CLK_INFRA_NR_CLK)
#define CK_INFRA_SPI1_SEL		(4 + CLK_INFRA_NR_CLK)
#define CK_INFRA_PWM1_SEL		(5 + CLK_INFRA_NR_CLK)
#define CK_INFRA_PWM2_SEL		(6 + CLK_INFRA_NR_CLK)
#define CK_INFRA_PWM_BSEL		(7 + CLK_INFRA_NR_CLK)
#define CK_INFRA_PCIE_SEL		(8 + CLK_INFRA_NR_CLK)
#define CK_INFRA_GPT_STA		0
#define CK_INFRA_PWM_HCK		1
#define CK_INFRA_PWM_STA		2
#define CK_INFRA_PWM1_CK		3
#define CK_INFRA_PWM2_CK		4
#define CK_INFRA_CQ_DMA_CK		5
#define CK_INFRA_EIP97_CK		6
#define CK_INFRA_AUD_BUS_CK		7
#define CK_INFRA_AUD_26M_CK		8
#define CK_INFRA_AUD_L_CK		9
#define CK_INFRA_AUD_AUD_CK		10
#define CK_INFRA_AUD_EG2_CK		11
#define CK_INFRA_DRAMC_26M_CK		12
#define CK_INFRA_DBG_CK			13
#define CK_INFRA_AP_DMA_CK		14
#define CK_INFRA_SEJ_CK			15
#define CK_INFRA_SEJ_13M_CK		16
#define CK_INFRA_THERM_CK		17
#define CK_INFRA_I2CO_CK		18
#define CK_INFRA_TRNG_CK		19
#define CK_INFRA_UART0_CK		20
#define CK_INFRA_UART1_CK		21
#define CK_INFRA_UART2_CK		22
#define CK_INFRA_NFI1_CK		23
#define CK_INFRA_SPINFI1_CK		24
#define CK_INFRA_NFI_HCK_CK		25
#define CK_INFRA_SPI0_CK		26
#define CK_INFRA_SPI1_CK		27
#define CK_INFRA_SPI0_HCK_CK		28
#define CK_INFRA_SPI1_HCK_CK		29
#define CK_INFRA_FRTC_CK		30
#define CK_INFRA_MSDC_CK		31
#define CK_INFRA_MSDC_HCK_CK		32
#define CK_INFRA_MSDC_133M_CK		33
#define CK_INFRA_MSDC_66M_CK		34
#define CK_INFRA_ADC_26M_CK		35
#define CK_INFRA_ADC_FRC_CK		36
#define CK_INFRA_FBIST2FPC_CK		37
#define CK_INFRA_IUSB_133_CK		38
#define CK_INFRA_IUSB_66M_CK		39
#define CK_INFRA_IUSB_SYS_CK		40
#define CK_INFRA_IUSB_CK		41
#define CK_INFRA_IPCIE_CK		42
#define CK_INFRA_IPCIER_CK		43
#define CK_INFRA_IPCIEB_CK		44
#define CLK_INFRA_AO_NR_CLK		45

/* APMIXEDSYS */

#define CK_APMIXED_ARMPLL		0
#define CK_APMIXED_NET2PLL		1
#define CK_APMIXED_MMPLL		2
#define CK_APMIXED_SGMPLL		3
#define CK_APMIXED_WEDMCUPLL		4
#define CK_APMIXED_NET1PLL		5
#define CK_APMIXED_MPLL			6
#define CK_APMIXED_APLL2		7
#define CLK_APMIXED_NR_CLK		8

/* SGMIISYS_0 */

#define CK_SGM0_TX_EN			0
#define CK_SGM0_RX_EN			1
#define CK_SGM0_CK0_EN			2
#define CK_SGM0_CDR_CK0_EN		3
#define CLK_SGMII0_NR_CLK		4

/* SGMIISYS_1 */

#define CK_SGM1_TX_EN			0
#define CK_SGM1_RX_EN			1
#define CK_SGM1_CK1_EN			2
#define CK_SGM1_CDR_CK1_EN		3
#define CLK_SGMII1_NR_CLK		4

/* ETHSYS */

#define CK_ETH_FE_EN			0
#define CK_ETH_GP2_EN			1
#define CK_ETH_GP1_EN			2
#define CK_ETH_WOCPU1_EN		3
#define CK_ETH_WOCPU0_EN		4
#define CLK_ETH_NR_CLK			5

#endif

/* _DT_BINDINGS_CLK_MT7986_H */
