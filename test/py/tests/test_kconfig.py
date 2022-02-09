# SPDX-License-Identifier: GPL-2.0
# Copyright 2022 Google LLC
# Written by Simon Glass <sjg@chromium.org>

import pytest

import u_boot_utils as util

@pytest.mark.slow
def test_kconfig(u_boot_console):
    """Test build failures when IF_ENABLED_INT() option is not enabled"""
    cons = u_boot_console

    # This detects build errors in test/lib/kconfig.c
    out = util.run_and_log(
        cons, ['./tools/buildman/buildman', '--board', 'sandbox',
               '-a', 'TEST_KCONFIG'], ignore_errors=True)
    assert 'invalid_use_of_IF_ENABLED_INT' in out
    assert 'invalid_use_of_CONFIG_IF_ENABLED_INT' in out

@pytest.mark.slow
def test_kconfig_spl(u_boot_console):
    """Test build failures when IF_ENABLED_INT() option is not enabled"""
    cons = u_boot_console

    # This detects build errors in test/lib/kconfig_spl.c
    out = util.run_and_log(
        cons, ['./tools/buildman/buildman', '--board', 'sandbox_spl',
               '-a', 'TEST_KCONFIG'], ignore_errors=True)
    assert 'invalid_use_of_IF_ENABLED_INT' in out

    # There is no CONFIG_SPL_TEST_KCONFIG, so the CONFIG_IF_ENABLED_INT()
    # line should not generate an error
    assert 'invalid_use_of_CONFIG_IF_ENABLED_INT' not in out
