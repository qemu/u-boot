Building with Clang
===================

The biggest problem when trying to compile U-Boot with Clang is that almost all
archs rely on storing gd in a global register and the Clang 3.5 user manual
states: "Clang does not support global register variables; this is unlikely to
be implemented soon because it requires additional LLVM backend support."

The ARM backend can be instructed not to use the r9 and x18 registers using
-ffixed-r9 or -ffixed-x18 respectively. As global registers themselves are not
supported inline assembly is needed to get and set the r9 or x18 value. This
leads to larger code then strictly necessary, but at least works.

Debian based
------------

Required packages can be installed via apt, e.g.

.. code-block:: bash

    sudo apt-get install clang

Note that we make use of the CROSS_COMPILE variable to determine what to tell
clang to use as the build target.
To compile U-Boot with Clang on Linux without IAS use e.g.

.. code-block:: bash

    make HOSTCC=clang rpi_2_defconfig
    make HOSTCC=clang CROSS_COMPILE=arm-linux-gnueabi- CC=clang -j8

It can also be used to compile sandbox:

.. code-block:: bash

    make HOSTCC=clang sandbox_defconfig
    make HOSTCC=clang CC=clang -j8


FreeBSD 11
----------

Since llvm 3.4 is currently in the base system, the integrated assembler as
is incapable of building U-Boot. Therefore gas from devel/arm-gnueabi-binutils
is used instead. It needs a symlink to be picked up correctly though:

.. code-block:: bash

    ln -s /usr/local/bin/arm-gnueabi-freebsd-as /usr/bin/arm-freebsd-eabi-as

The following commands compile U-Boot using the Clang xdev toolchain.

**NOTE:** CROSS_COMPILE and target differ on purpose!

.. code-block:: bash

    export CROSS_COMPILE=arm-gnueabi-freebsd-
    gmake rpi_2_defconfig
    gmake CC="clang -target arm-freebsd-eabi --sysroot /usr/arm-freebsd" -j8

Given that U-Boot will default to gcc, above commands can be
simplified with a simple wrapper script - saved as
/usr/local/bin/arm-gnueabi-freebsd-gcc - listed below:

.. code-block:: bash

    #!/bin/sh
    exec clang -target arm-freebsd-eabi --sysroot /usr/arm-freebsd "$@"
