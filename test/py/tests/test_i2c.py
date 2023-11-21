# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

import pytest
import random
import re

"""
Note: This test doesn't rely on boardenv_* configuration value but they can
change test behavior.

For example:

# Setup env__i2c_device_test_skip to True if tests with i2c devices should be
# skipped. For example: Missing QEMU model or broken i2c device
env__i2c_device_test_skip = True

# Setup env__i2c_eeprom_device_test to set the i2c bus number, eeprom address
# and configured value for i2c_eeprom test case. Test will be skipped if
# env__i2c_eeprom_device_test is not set
env__i2c_eeprom_device_test = {
    "bus": 3,
    "eeprom_addr": 0x54,
    "eeprom_val": "30 31",
}

# Setup env__i2c_device_test to provide the i2c bus list to test against it
# for i2c_probe_all_buses case instead of probing all the buses available. If
# it is not set, it list down all the buses and probes it
env__i2c_device_test = {
    "bus_list": [0, 2, 5, 12, 16, 18]
}
"""

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_bus(u_boot_console):
    if u_boot_console.config.env.get("env__i2c_device_test_skip", False):
        pytest.skip("I2C device test is not enabled!")
    expected_response = "Bus"
    response = u_boot_console.run_command("i2c bus")
    assert expected_response in response

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_dev(u_boot_console):
    if u_boot_console.config.env.get("env__i2c_device_test_skip", False):
        pytest.skip("I2C device test is not enabled!")
    expected_response = "Current bus"
    response = u_boot_console.run_command("i2c dev")
    assert expected_response in response

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_probe(u_boot_console):
    if u_boot_console.config.env.get("env__i2c_device_test_skip", False):
        pytest.skip("I2C device test is not enabled!")
    expected_response = "Setting bus to 0"
    response = u_boot_console.run_command("i2c dev 0")
    assert expected_response in response
    expected_response = "Valid chip addresses:"
    response = u_boot_console.run_command("i2c probe")
    assert expected_response in response

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_eeprom(u_boot_console):
    f = u_boot_console.config.env.get("env__i2c_eeprom_device_test", None)
    if not f:
        pytest.skip("No I2C eeprom to test!")

    bus = f.get("bus", 0)
    if bus < 0:
        pytest.fail("No bus specified via env__i2c_eeprom_device_test!")

    addr = f.get("eeprom_addr", -1)
    if addr < 0:
        pytest.fail("No eeprom address specified via env__i2c_eeprom_device_test!")

    value = f.get("eeprom_val")
    if not value:
        pytest.fail("No eeprom configured value provided via env__i2c_eeprom_device_test!")

    # Enable i2c mux bridge
    u_boot_console.run_command("i2c dev %x" % bus)
    u_boot_console.run_command("i2c probe")
    output = u_boot_console.run_command("i2c md %x 0 5" % addr)
    assert value in output

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_probe_all_buses(u_boot_console):
    if u_boot_console.config.env.get("env__i2c_device_test_skip", False):
        pytest.skip("I2C device test is not enabled!")
    expected_response = "Bus"
    response = u_boot_console.run_command("i2c bus")
    assert expected_response in response

    # Get all the bus list
    f = u_boot_console.config.env.get("env__i2c_device_test", None)
    if f:
        bus_list = f.get("bus_list")
    else:
        buses = re.findall("Bus (.+?):", response)
        bus_list = [int(x) for x in buses]

    for dev in bus_list:
        expected_response = f"Setting bus to {dev}"
        response = u_boot_console.run_command(f"i2c dev {dev}")
        assert expected_response in response
        expected_response = "Valid chip addresses:"
        response = u_boot_console.run_command("i2c probe")
        assert expected_response in response
