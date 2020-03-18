# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (c) 2017 Microsemi Corporation.
# Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
#
# Copyright (C) 2017 Andes Technology Corporation
# Rick Chen, Andes Technology Corporation <rick@andestech.com>
#

ldflags-$(CONFIG_32BIT) := -m elf32lriscv
ldflags-$(CONFIG_64BIT) := -m elf64lriscv

ifdef CONFIG_32BIT
EFI_LDS			:= elf_riscv32_efi.lds
endif

ifdef CONFIG_64BIT
EFI_LDS			:= elf_riscv64_efi.lds
endif

CONFIG_STANDALONE_LOAD_ADDR ?= 0x00000000

PLATFORM_CPPFLAGS	+= -ffixed-gp -fpic
PLATFORM_RELFLAGS	+= -fno-common -gdwarf-2 -ffunction-sections \
			   -fdata-sections
LDFLAGS_u-boot		+= --gc-sections -static -pie
KBUILD_LDFLAGS		+= $(ldflags-y)

EFI_CRT0		:= crt0_riscv_efi.o
EFI_RELOC		:= reloc_riscv_efi.o
