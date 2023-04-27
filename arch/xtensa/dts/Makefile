# SPDX-License-Identifier: GPL-2.0+

include $(srctree)/scripts/Makefile.dts

targets += $(dtb-y)

DTC_FLAGS +=

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
