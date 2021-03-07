# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2021, Asherah Connor <ashe@kivikakk.ee>

# Test qfw command implementation

import pytest

@pytest.mark.buildconfigspec('cmd_qfw')
def test_qfw_cpus(u_boot_console):
    "Test QEMU firmware config reports the CPU count correctly."

    output = u_boot_console.run_command('qfw cpus')
    assert '1 cpu(s) online' in output

@pytest.mark.buildconfigspec('cmd_qfw')
def test_qfw_list(u_boot_console):
    "Test QEMU firmware config lists devices."

    output = u_boot_console.run_command('qfw list')
    assert 'bootorder' in output
    assert 'etc/table-loader' in output
