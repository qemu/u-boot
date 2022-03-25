.. SPDX-License-Identifier: GPL-2.0+

pre-load-verify command
=======================

Synopsis
--------

::

    pre_load_verify <addr>

Description
-----------

The pre-load-verify command verify the signature of the binary at address addr
using the pre-load header that should be at the beginning of the binary.

addr
    Address of the binary to verify


Examples
--------


::

    => pre_load_verify 100
    INFO: signature check has succeed

If succeed, the u-boot env variable loadaddr_verified is set to the address
if the binary after the pre-load header

::

    => printenv loadaddr_verified
    loadaddr_verified=1100


Return value
------------

The return value $? is 0 is the signature check succeed, 1 otherwise
