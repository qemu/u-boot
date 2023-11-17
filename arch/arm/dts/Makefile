# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_ARCH_ZYNQMP) += \
	zynqmp-p-a2197-00-revA-x-prc-01-revA.dtbo	\
	zynqmp-p-a2197-00-revA-x-prc-02-revA.dtbo	\
	zynqmp-p-a2197-00-revA-x-prc-03-revA.dtbo	\
	zynqmp-p-a2197-00-revA-x-prc-04-revA.dtbo	\
	zynqmp-p-a2197-00-revA-x-prc-05-revA.dtbo	\
	zynqmp-sc-vek280-revA.dtbo		\
	zynqmp-sc-vek280-revB.dtbo		\
	zynqmp-sc-vhk158-revA.dtbo		\
	zynqmp-sc-vpk120-revB.dtbo		\
	zynqmp-sc-vpk180-revA.dtbo		\
	zynqmp-sc-vpk180-revB.dtbo		\
	zynqmp-sc-vn-p-b2197-00-revA.dtbo	\
	zynqmp-sck-kd-g-revA.dtbo		\
	zynqmp-sck-kr-g-revA.dtbo		\
	zynqmp-sck-kr-g-revB.dtbo		\
	zynqmp-sck-kv-g-revA.dtbo		\
	zynqmp-sck-kv-g-revB.dtbo

dtb-$(CONFIG_ARCH_IMX8M) += \
	imx8mp-dhcom-som-overlay-rev100.dtbo \
	imx8mp-dhcom-som-overlay-eth1xfast.dtbo \
	imx8mp-dhcom-som-overlay-eth2xfast.dtbo \
	imx8mp-dhcom-pdk-overlay-eth2xfast.dtbo \
	imx8mp-dhcom-pdk3-overlay-rev100.dtbo

dtb-$(CONFIG_SOC_K3_AM654) += \
	k3-am6548-iot2050-advanced-m2-bkey-usb3-overlay.dtbo \
	k3-am6548-iot2050-advanced-m2-bkey-ekey-pcie-overlay.dtbo

dtb-$(CONFIG_TARGET_IMX8MM_CL_IOT_GATE) += \
					imx8mm-cl-iot-gate-ied.dtbo \
					imx8mm-cl-iot-gate-ied-adc0.dtbo \
					imx8mm-cl-iot-gate-ied-adc1.dtbo \
					imx8mm-cl-iot-gate-ied-can0.dtbo \
					imx8mm-cl-iot-gate-ied-can1.dtbo \
					imx8mm-cl-iot-gate-ied-tpm0.dtbo \
					imx8mm-cl-iot-gate-ied-tpm1.dtbo

dtb-$(CONFIG_TARGET_IMX8MM_CL_IOT_GATE_OPTEE) += \
					imx8mm-cl-iot-gate-ied.dtbo \
					imx8mm-cl-iot-gate-ied-adc0.dtbo \
					imx8mm-cl-iot-gate-ied-adc1.dtbo \
					imx8mm-cl-iot-gate-ied-can0.dtbo \
					imx8mm-cl-iot-gate-ied-can1.dtbo \
					imx8mm-cl-iot-gate-ied-tpm0.dtbo \
					imx8mm-cl-iot-gate-ied-tpm1.dtbo

include $(srctree)/scripts/Makefile.dts

targets += $(dtb-y)

# Add any required device tree compiler flags here
DTC_FLAGS += -a 0x8

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb *.dtbo *_HS
