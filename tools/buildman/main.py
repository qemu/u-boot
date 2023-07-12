#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2012 The Chromium OS Authors.
#

"""See README for more information"""

import os
import sys

# Bring in the patman libraries
# pylint: disable=C0413
our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, os.path.join(our_path, '..'))

# Our modules
from buildman import bsettings
from buildman import cmdline
from buildman import control
from u_boot_pylib import test_util

def run_tests(skip_net_tests, verbose, args):
    """Run the buildman tests

    Args:
        skip_net_tests (bool): True to skip tests which need the network
        verbosity (int): Verbosity level to use (0-4)
        args (list of str): List of tests to run, empty to run all
    """
    # These imports are here since tests are not available when buildman is
    # installed as a Python module
    # pylint: disable=C0415
    from buildman import func_test
    from buildman import test

    test_name = args and args[0] or None
    if skip_net_tests:
        test.use_network = False

    # Run the entry tests first ,since these need to be the first to import the
    # 'entry' module.
    result = test_util.run_test_suites(
        'buildman', False, verbose, False, None, test_name, [],
        [test.TestBuild, func_test.TestFunctional,
         'buildman.toolchain', 'patman.gitutil'])

    return (0 if result.wasSuccessful() else 1)

def run_buildman():
    """Run bulidman

    This is the main program. It collects arguments and runs either the tests or
    the control module.
    """
    options, args = cmdline.ParseArgs()

    if not options.debug:
        sys.tracebacklimit = 0

    # Run our meagre tests
    if cmdline.HAS_TESTS and options.test:
        run_tests(options.skip_net_tests, options.verbose, args)

    # Build selected commits for selected boards
    else:
        bsettings.Setup(options.config_file)
        ret_code = control.do_buildman(options, args)
        sys.exit(ret_code)


if __name__ == "__main__":
    run_buildman()
