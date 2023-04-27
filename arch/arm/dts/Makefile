# SPDX-License-Identifier: GPL-2.0+

ifdef CONFIG_RCAR_64
DTC_FLAGS += -R 4 -p 0x1000
endif

include $(srctree)/scripts/Makefile.dts

targets += $(dtb-y)

# Add any required device tree compiler flags here
DTC_FLAGS += -a 0x8

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb *.dtbo *_HS
