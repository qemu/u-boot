#!/bin/sh

# 0) Generate keys
#
# WARNING: ECDSA keys are only supported by HAB 4.5 and newer (i.e. i.MX8M Plus)
#
# cd /path/to/cst-3.3.1/keys/
#    ./hab4_pki_tree.sh -existing-ca n -use-ecc n -kl 4096 -duration 10 -num-srk 4 -srk-ca y
# cd /path/to/cst-3.3.1/crts/
#   ../linux64/bin/srktool -h 4 -t SRK_1_2_3_4_table.bin -e SRK_1_2_3_4_fuse.bin -d sha256 -c ./SRK1_sha256_4096_65537_v3_ca_crt.pem,./SRK2_sha256_4096_65537_v3_ca_crt.pem,./SRK3_sha256_4096_65537_v3_ca_crt.pem,./SRK4_sha256_4096_65537_v3_ca_crt.pem -f 1

# 1) Build U-Boot (e.g. for i.MX8MM)
#
# export ATF_LOAD_ADDR=0x920000
# cp -Lv /path/to/arm-trusted-firmware/build/imx8mm/release/bl31.bin .
# cp -Lv /path/to/firmware-imx-8.14/firmware/ddr/synopsys/ddr3* .
# make -j imx8mm_board_defconfig
# make -j`nproc` flash.bin

# 2) Sign SPL and DRAM blobs

cp doc/imx/habv4/csf_examples/mx8m/csf_spl.txt csf_spl.tmp
cp doc/imx/habv4/csf_examples/mx8m/csf_fit.txt csf_fit.tmp

spl_block_base=$(printf "0x%x" $(( $(sed -n "/CONFIG_SPL_TEXT_BASE=/ s@.*=@@p" .config) - 0x40)) )
spl_block_size=$(printf "0x%x" $(stat -tc %s u-boot-spl-ddr.bin))
sed -i "/Blocks = / s@.*@  Blocks = $spl_block_base 0x0 $spl_block_size \"flash.bin\"@" csf_spl.tmp

# Generate CSF blob
cst -i csf_spl.tmp -o csf_spl.bin

# Patch CSF blob into flash.bin
spl_csf_offset=$(xxd -s 24 -l 4 -e flash.bin | cut -d " " -f 2 | sed "s@^@0x@")
spl_bin_offset=$(xxd -s 4 -l 4 -e flash.bin | cut -d " " -f 2 | sed "s@^@0x@")
spl_dd_offset=$((${spl_csf_offset} - ${spl_bin_offset} + 0x40))
dd if=csf_spl.bin of=flash.bin bs=1 seek=${spl_dd_offset} conv=notrunc

# 3) Sign u-boot.itb

# fitImage
fit_block_base=$(printf "0x%x" $(sed -n "/CONFIG_SPL_LOAD_FIT_ADDRESS=/ s@.*=@@p" .config) )
fit_block_offset=$(printf "0x%s" $(fdtget -t x u-boot.dtb /binman/imx-boot/uboot offset))
fit_block_size=$(printf "0x%x" $(( ( ( $(stat -tc %s u-boot.itb) + 0x1000 - 0x1 ) & ~(0x1000 - 0x1)) + 0x20 )) )
sed -i "/Blocks = / s@.*@  Blocks = $fit_block_base $fit_block_offset $fit_block_size \"flash.bin\"@" csf_fit.tmp

# IVT
ivt_ptr_base=$(printf "%08x" ${fit_block_base} | sed "s@\(..\)\(..\)\(..\)\(..\)@0x\4\3\2\1@")
ivt_block_base=$(printf "%08x" $(( ${fit_block_base} + ${fit_block_size} - 0x20 )) | sed "s@\(..\)\(..\)\(..\)\(..\)@0x\4\3\2\1@")
csf_block_base=$(printf "%08x" $(( ${fit_block_base} + ${fit_block_size} )) | sed "s@\(..\)\(..\)\(..\)\(..\)@0x\4\3\2\1@")
ivt_block_offset=$((${fit_block_offset} + ${fit_block_size} - 0x20))
csf_block_offset=$((${ivt_block_offset} + 0x20))

echo "0xd1002041 ${ivt_block_base} 0x00000000 0x00000000 0x00000000 ${ivt_block_base} ${csf_block_base} 0x00000000" | xxd -r -p > ivt.bin
dd if=ivt.bin of=flash.bin bs=1 seek=${ivt_block_offset} conv=notrunc

# Generate CSF blob
cst -i csf_fit.tmp -o csf_fit.bin
# Patch CSF blob into flash.bin
dd if=csf_fit.bin of=flash.bin bs=1 seek=${csf_block_offset} conv=notrunc
