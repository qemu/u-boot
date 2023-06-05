.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the HTC One X (endeavoru)
======================================================

``DISCLAMER!`` Moving your HTC ONe X to use U-Boot assumes
replacement of the vendor hboot. Vendor android firmwares
will no longer be able to run on the device.
This replacement IS reversible.

Quick Start
-----------

- Build U-Boot
- Pack U-Boot into repart-block
- Flash repart-block into the eMMC
- Boot
- Self Upgrading

Build U-Boot
------------

.. code-block:: bash

    $ export CROSS_COMPILE=arm-linux-gnueabi-
    $ make endeavoru_defconfig
    $ make

After the build succeeds, you will obtain the final ``u-boot-dtb-tegra.bin``
image, ready for flashing (but check the next section for additional
adjustments).

Pack U-Boot into repar-block
---------------------------

``DISCLAMER!`` All questions related to re-crypt work should be
asked in re-crypt repo issues. NOT HERE!

re-crypt is a small script which packs ``u-boot-dtb-tegra.bin`` in
form usable by device. This process is required only on the first
installation or to recover the device in case of a failed update.

.. code-block:: bash

    $ git clone https://github.com/clamor-s/re-crypt.git
    $ cd re-crypt # place your u-boot-dtb-regra.bin here
    $ ./re-crypt.sh -d endeavoru

Script will produce you a `repart-block.bin` ready to flash.

Flash repart-block into the eMMC
---------------------------

``DISCLAMER!`` All questions related to NvFlash should be asked
in the proper place. NOT HERE! Flashing repart-block will erase
all your eMMC, so make a backup before!

`repart-block.bin` contains BCT and bootloader in encrypted state
in form which can just be written RAW at the start of eMMC.

.. code-block:: bash

    $ wheelie --blob blob.bin
    $ nvflash --resume --rawdevicewrite 0 1024 repart-block.bin

Boot
----

After flashing ``repart-block.bin`` the device should reboot and turn
itself off. This is normal behavior if no boot configuration is
found.

To boot Linux, U-Boot will look for an ``extlinux.conf`` on MicroSD
and then on eMMC. Additionally if Volume Down button is pressed
while booting device will enter bootmenu. Bootmenu contains entries
to mount MicroSD and eMMC as mass storage, fastboot, reboot, reboot
RCM, poweroff, enter U-Boot console and update bootloader (check next
chapter).

FLashing ``repart-block.bin`` eliminates vendor restriction on eMMC
and allows the user to use/partition it in any way the user desires.

Self Upgrading
--------------------------------------

Place your ``u-boot-dtb-tegra.bin`` on the first partition of the
MicroSD card and insert it into the tablet. Enter bootmenu, choose
update bootloader option with Power button and U-Boot should update
itself. Once the process is completed, U-Boot will ask to press any
button to reboot.
