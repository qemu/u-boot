# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2013 The Chromium OS Authors.
#

"""Control module for buildman

This holds the main control logic for buildman, when not running tests.
"""

import multiprocessing
import os
import shutil
import sys

from buildman import boards
from buildman import bsettings
from buildman import cfgutil
from buildman import toolchain
from buildman.builder import Builder
from patman import gitutil
from patman import patchstream
from u_boot_pylib import command
from u_boot_pylib import terminal
from u_boot_pylib.terminal import tprint

TEST_BUILDER = None

def get_plural(count):
    """Returns a plural 's' if count is not 1"""
    return 's' if count != 1 else ''

def get_action_summary(is_summary, commits, selected, step, threads, jobs):
    """Return a string summarising the intended action.

    Args:
        is_summary (bool): True if this is a summary (otherwise it is building)
        commits (list): List of commits being built
        selected (list of Board): List of Board objects that are marked
        step (int): Step increment through commits
        threads (int): Number of processor threads being used
        jobs (int): Number of jobs to build at once

    Returns:
        Summary string.
    """
    if commits:
        count = len(commits)
        count = (count + step - 1) // step
        commit_str = f'{count} commit{get_plural(count)}'
    else:
        commit_str = 'current source'
    msg = (f"{'Summary of' if is_summary else 'Building'} "
           f'{commit_str} for {len(selected)} boards')
    msg += (f' ({threads} thread{get_plural(threads)}, '
            f'{jobs} job{get_plural(jobs)} per thread)')
    return msg

# pylint: disable=R0913
def show_actions(series, why_selected, boards_selected, output_dir,
                 board_warnings, step, threads, jobs, verbose):
    """Display a list of actions that we would take, if not a dry run.

    Args:
        series: Series object
        why_selected: Dictionary where each key is a buildman argument
                provided by the user, and the value is the list of boards
                brought in by that argument. For example, 'arm' might bring
                in 400 boards, so in this case the key would be 'arm' and
                the value would be a list of board names.
        boards_selected: Dict of selected boards, key is target name,
                value is Board object
        output_dir (str): Output directory for builder
        board_warnings: List of warnings obtained from board selected
        step (int): Step increment through commits
        threads (int): Number of processor threads being used
        jobs (int): Number of jobs to build at once
        verbose (bool): True to indicate why each board was selected
    """
    col = terminal.Color()
    print('Dry run, so not doing much. But I would do this:')
    print()
    if series:
        commits = series.commits
    else:
        commits = None
    print(get_action_summary(False, commits, boards_selected, step, threads,
                             jobs))
    print(f'Build directory: {output_dir}')
    if commits:
        for upto in range(0, len(series.commits), step):
            commit = series.commits[upto]
            print('   ', col.build(col.YELLOW, commit.hash[:8], bright=False), end=' ')
            print(commit.subject)
    print()
    for arg in why_selected:
        if arg != 'all':
            print(arg, f': {len(why_selected[arg])} boards')
            if verbose:
                print(f"   {' '.join(why_selected[arg])}")
    print('Total boards to build for each '
          f"commit: {len(why_selected['all'])}\n")
    if board_warnings:
        for warning in board_warnings:
            print(col.build(col.YELLOW, warning))

def show_toolchain_prefix(brds, toolchains):
    """Show information about a the tool chain used by one or more boards

    The function checks that all boards use the same toolchain, then prints
    the correct value for CROSS_COMPILE.

    Args:
        boards: Boards object containing selected boards
        toolchains: Toolchains object containing available toolchains

    Return:
        None on success, string error message otherwise
    """
    board_selected = brds.get_selected_dict()
    tc_set = set()
    for brd in board_selected.values():
        tc_set.add(toolchains.Select(brd.arch))
    if len(tc_set) != 1:
        sys.exit('Supplied boards must share one toolchain')
    tchain = tc_set.pop()
    print(tchain.GetEnvArgs(toolchain.VAR_CROSS_COMPILE))

def get_allow_missing(opt_allow, opt_no_allow, num_selected, has_branch):
    """Figure out whether to allow external blobs

    Uses the allow-missing setting and the provided arguments to decide whether
    missing external blobs should be allowed

    Args:
        opt_allow (bool): True if --allow-missing flag is set
        opt_no_allow (bool): True if --no-allow-missing flag is set
        num_selected (int): Number of selected board
        has_branch (bool): True if a git branch (to build) has been provided

    Returns:
        bool: True to allow missing external blobs, False to produce an error if
            external blobs are used
    """
    allow_missing = False
    am_setting = bsettings.GetGlobalItemValue('allow-missing')
    if am_setting:
        if am_setting == 'always':
            allow_missing = True
        if 'multiple' in am_setting and num_selected > 1:
            allow_missing = True
        if 'branch' in am_setting and has_branch:
            allow_missing = True

    if opt_allow:
        allow_missing = True
    if opt_no_allow:
        allow_missing = False
    return allow_missing


def count_commits(branch, count, col, git_dir):
    """Could the number of commits in the branch/ranch being built

    Args:
        branch (str): Name of branch to build, or None if none
        count (int): Number of commits to build, or -1 for all
        col (Terminal.Color): Color object to use
        git_dir (str): Git directory to use, e.g. './.git'

    Returns:
        tuple:
            Number of commits being built
            True if the 'branch' string contains a range rather than a simple
                name
    """
    has_range = branch and '..' in branch
    if count == -1:
        if not branch:
            count = 1
        else:
            if has_range:
                count, msg = gitutil.count_commits_in_range(git_dir, branch)
            else:
                count, msg = gitutil.count_commits_in_branch(git_dir, branch)
            if count is None:
                sys.exit(col.build(col.RED, msg))
            elif count == 0:
                sys.exit(col.build(col.RED,
                                   f"Range '{branch}' has no commits"))
            if msg:
                print(col.build(col.YELLOW, msg))
            count += 1   # Build upstream commit also

    if not count:
        msg = (f"No commits found to process in branch '{branch}': "
               "set branch's upstream or use -c flag")
        sys.exit(col.build(col.RED, msg))
    return count, has_range


def determine_series(selected, col, git_dir, count, branch, work_in_output):
    """Determine the series which is to be built, if any

    If there is a series, the commits in that series are numbered by setting
    their sequence value (starting from 0). This is used by tests.

    Args:
        selected (list of Board): List of Board objects that are marked
            selected
        col (Terminal.Color): Color object to use
        git_dir (str): Git directory to use, e.g. './.git'
        count (int): Number of commits in branch
        branch (str): Name of branch to build, or None if none
        work_in_output (bool): True to work in the output directory

    Returns:
        Series: Series to build, or None for none

    Read the metadata from the commits. First look at the upstream commit,
    then the ones in the branch. We would like to do something like
    upstream/master~..branch but that isn't possible if upstream/master is
    a merge commit (it will list all the commits that form part of the
    merge)

    Conflicting tags are not a problem for buildman, since it does not use
    them. For example, Series-version is not useful for buildman. On the
    other hand conflicting tags will cause an error. So allow later tags
    to overwrite earlier ones by setting allow_overwrite=True
    """

    # Work out how many commits to build. We want to build everything on the
    # branch. We also build the upstream commit as a control so we can see
    # problems introduced by the first commit on the branch.
    count, has_range = count_commits(branch, count, col, git_dir)
    if work_in_output:
        if len(selected) != 1:
            sys.exit(col.build(col.RED,
                               '-w can only be used with a single board'))
        if count != 1:
            sys.exit(col.build(col.RED,
                               '-w can only be used with a single commit'))

    if branch:
        if count == -1:
            if has_range:
                range_expr = branch
            else:
                range_expr = gitutil.get_range_in_branch(git_dir, branch)
            upstream_commit = gitutil.get_upstream(git_dir, branch)
            series = patchstream.get_metadata_for_list(upstream_commit,
                git_dir, 1, series=None, allow_overwrite=True)

            series = patchstream.get_metadata_for_list(range_expr,
                    git_dir, None, series, allow_overwrite=True)
        else:
            # Honour the count
            series = patchstream.get_metadata_for_list(branch,
                    git_dir, count, series=None, allow_overwrite=True)

        # Number the commits for test purposes
        for i, commit in enumerate(series.commits):
            commit.sequence = i
    else:
        series = None
    return series


def do_fetch_arch(toolchains, col, fetch_arch):
    """Handle the --fetch-arch option

    Args:
        toolchains (Toolchains): Tool chains to use
        col (terminal.Color): Color object to build
        fetch_arch (str): Argument passed to the --fetch-arch option

    Returns:
        int: Return code for buildman
    """
    if fetch_arch == 'list':
        sorted_list = toolchains.ListArchs()
        print(col.build(
            col.BLUE,
            f"Available architectures: {' '.join(sorted_list)}\n"))
        return 0

    if fetch_arch == 'all':
        fetch_arch = ','.join(toolchains.ListArchs())
        print(col.build(col.CYAN,
                        f'\nDownloading toolchains: {fetch_arch}'))
    for arch in fetch_arch.split(','):
        print()
        ret = toolchains.FetchAndInstall(arch)
        if ret:
            return ret
    return 0


def get_toolchains(toolchains, col, override_toolchain, fetch_arch,
                   list_tool_chains, verbose):
    """Get toolchains object to use

    Args:
        toolchains (Toolchains or None): Toolchains to use. If None, then a
            Toolchains object will be created and scanned
        col (Terminal.Color): Color object
        override_toolchain (str or None): Override value for toolchain, or None
        fetch_arch (bool): True to fetch the toolchain for the architectures
        list_tool_chains (bool): True to list all tool chains
        verbose (bool): True for verbose output when listing toolchains

Returns:
        Either:
            int: Operation completed and buildman should exit with exit code
            Toolchains: Toolchains object to use
    """
    no_toolchains = toolchains is None
    if no_toolchains:
        toolchains = toolchain.Toolchains(override_toolchain)

    if fetch_arch:
        return do_fetch_arch(toolchains, col, fetch_arch)

    if no_toolchains:
        toolchains.GetSettings()
        toolchains.Scan(list_tool_chains and verbose)
    if list_tool_chains:
        toolchains.List()
        print()
        return 0
    return toolchains


def get_boards_obj(output_dir, regen_board_list, threads, verbose):
    """Object the Boards object to use

    Creates the output directory and ensures there is a boards.cfg file, then
    read it in.

    Args:
        output_dir (str): Output directory to use
        regen_board_list (bool): True to just regenerate the board list
        threads (int or None): Number of threads to use to create boards file
        verbose (bool): False to suppress output from boards-file generation

    Returns:
        Either:
            int: Operation completed and buildman should exit with exit code
            Boards: Boards object to use
    """
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    board_file = os.path.join(output_dir, 'boards.cfg')
    if regen_board_list and regen_board_list != '-':
        board_file = regen_board_list

    brds = boards.Boards()
    okay = brds.ensure_board_list(
        board_file,
        threads or multiprocessing.cpu_count(),
        force=regen_board_list,
        quiet=not verbose)
    if regen_board_list:
        return 0 if okay else 2
    brds.read_boards(board_file)
    return brds


def determine_boards(brds, args, col, opt_boards, exclude):
    """Determine which boards to build

    Each element of args and exclude can refer to a board name, arch or SoC

    Args:
        brds (Boards): Boards object
        args (list of str): Arguments describing boards to build
        col (Terminal.Color): Color object
        opt_boards (list of str): Specific boards to build, or None for all
        exclude (list of str): Arguments describing boards to exclude

    Returns:
        tuple:
            list of Board: List of Board objects that are marked selected
            why_selected: Dictionary where each key is a buildman argument
                    provided by the user, and the value is the list of boards
                    brought in by that argument. For example, 'arm' might bring
                    in 400 boards, so in this case the key would be 'arm' and
                    the value would be a list of board names.
            board_warnings: List of warnings obtained from board selected
    """
    exclude = []
    if exclude:
        for arg in exclude:
            exclude += arg.split(',')

    if opt_boards:
        requested_boards = []
        for brd in opt_boards:
            requested_boards += brd.split(',')
    else:
        requested_boards = None
    why_selected, board_warnings = brds.select_boards(args, exclude,
                                                      requested_boards)
    selected = brds.get_selected()
    if not selected:
        sys.exit(col.build(col.RED, 'No matching boards found'))
    return selected, why_selected, board_warnings


def adjust_options(options, series, selected):
    """Adjust options according to various constraints

    Updates verbose, show_errors, threads, jobs and step

    Args:
        options (Options): Options object to adjust
        series (Series): Series being built / summarised
        selected (list of Board): List of Board objects that are marked
    """
    if not series and not options.dry_run:
        options.verbose = True
        if not options.summary:
            options.show_errors = True

    # By default we have one thread per CPU. But if there are not enough jobs
    # we can have fewer threads and use a high '-j' value for make.
    if options.threads is None:
        options.threads = min(multiprocessing.cpu_count(), len(selected))
    if not options.jobs:
        options.jobs = max(1, (multiprocessing.cpu_count() +
                len(selected) - 1) // len(selected))

    if not options.step:
        options.step = len(series.commits) - 1

    # We can't show function sizes without board details at present
    if options.show_bloat:
        options.show_detail = True


def setup_output_dir(output_dir, work_in_output, branch, no_subdirs, col,
                     clean_dir):
    """Set up the output directory

    Args:
        output_dir (str): Output directory provided by the user, or None if none
        work_in_output (bool): True to work in the output directory
        branch (str): Name of branch to build, or None if none
        no_subdirs (bool): True to put the output in the top-level output dir
        clean_dir: Used for tests only, indicates that the existing output_dir
            should be removed before starting the build

    Returns:
        str: Updated output directory pathname
    """
    if not output_dir:
        if work_in_output:
            sys.exit(col.build(col.RED, '-w requires that you specify -o'))
        output_dir = '..'
    if branch and not no_subdirs:
        # As a special case allow the board directory to be placed in the
        # output directory itself rather than any subdirectory.
        dirname = branch.replace('/', '_')
        output_dir = os.path.join(output_dir, dirname)
        if clean_dir and os.path.exists(output_dir):
            shutil.rmtree(output_dir)
    return output_dir


def run_builder(builder, commits, board_selected, options):
    """Run the builder or show the summary

    Args:
        commits (list of Commit): List of commits being built, None if no branch
        boards_selected (dict): Dict of selected boards:
            key: target name
            value: Board object
        options (Options): Options to use

    Returns:
        int: Return code for buildman
    """
    gnu_make = command.output(os.path.join(options.git,
            'scripts/show-gnu-make'), raise_on_error=False).rstrip()
    if not gnu_make:
        sys.exit('GNU Make not found')
    builder.gnu_make = gnu_make

    if not options.ide:
        tprint(get_action_summary(options.summary, commits, board_selected,
                                  options.step, options.threads, options.jobs))

    builder.SetDisplayOptions(
        options.show_errors, options.show_sizes, options.show_detail,
        options.show_bloat, options.list_error_boards, options.show_config,
        options.show_environment, options.filter_dtb_warnings,
        options.filter_migration_warnings, options.ide)
    if options.summary:
        builder.ShowSummary(commits, board_selected)
    else:
        fail, warned, excs = builder.BuildBoards(
            commits, board_selected, options.keep_outputs, options.verbose)
        if excs:
            return 102
        if fail:
            return 100
        if warned and not options.ignore_warnings:
            return 101
    return 0


def calc_adjust_cfg(adjust_cfg, reproducible_builds):
    """Calculate the value to use for adjust_cfg

    Args:
        adjust_cfg (list of str): List of configuration changes. See cfgutil for
            details
        reproducible_builds (bool): True to adjust the configuration to get
            reproduceable builds

    Returns:
        adjust_cfg (list of str): List of configuration changes
    """
    adjust_cfg = cfgutil.convert_list_to_dict(adjust_cfg)

    # Drop LOCALVERSION_AUTO since it changes the version string on every commit
    if reproducible_builds:
        # If these are mentioned, leave the local version alone
        if 'LOCALVERSION' in adjust_cfg or 'LOCALVERSION_AUTO' in adjust_cfg:
            print('Not dropping LOCALVERSION_AUTO for reproducible build')
        else:
            adjust_cfg['LOCALVERSION_AUTO'] = '~'
    return adjust_cfg


def do_buildman(options, args, toolchains=None, make_func=None, brds=None,
                clean_dir=False, test_thread_exceptions=False):
    """The main control code for buildman

    Args:
        options: Command line options object
        args: Command line arguments (list of strings)
        toolchains: Toolchains to use - this should be a Toolchains()
                object. If None, then it will be created and scanned
        make_func: Make function to use for the builder. This is called
                to execute 'make'. If this is None, the normal function
                will be used, which calls the 'make' tool with suitable
                arguments. This setting is useful for tests.
        brds: Boards() object to use, containing a list of available
                boards. If this is None it will be created and scanned.
        clean_dir: Used for tests only, indicates that the existing output_dir
            should be removed before starting the build
        test_thread_exceptions: Uses for tests only, True to make the threads
            raise an exception instead of reporting their result. This simulates
            a failure in the code somewhere
    """
    # Used so testing can obtain the builder: pylint: disable=W0603
    global TEST_BUILDER

    gitutil.setup()
    col = terminal.Color()

    git_dir = os.path.join(options.git, '.git')

    toolchains = get_toolchains(toolchains, col, options.override_toolchain,
                                options.fetch_arch, options.list_tool_chains,
                                options.verbose)
    output_dir = setup_output_dir(
        options.output_dir, options.work_in_output, options.branch,
        options.no_subdirs, col, clean_dir)

    # Work out what subset of the boards we are building
    if not brds:
        brds = get_boards_obj(output_dir, options.regen_board_list,
                              options.threads, options.verbose)
        if isinstance(brds, int):
            return brds

    selected, why_selected, board_warnings = determine_boards(
        brds, args, col, options.boards, options.exclude)

    if options.print_prefix:
        show_toolchain_prefix(brds, toolchains)
        return 0

    series = determine_series(selected, col, git_dir, options.count,
                              options.branch, options.work_in_output)

    adjust_options(options, series, selected)

    # For a dry run, just show our actions as a sanity check
    if options.dry_run:
        show_actions(series, why_selected, selected, output_dir, board_warnings,
                     options.step, options.threads, options.jobs,
                     options.verbose)
        return 0

    # Create a new builder with the selected options
    builder = Builder(toolchains, output_dir, git_dir,
            options.threads, options.jobs, checkout=True,
            show_unknown=options.show_unknown, step=options.step,
            no_subdirs=options.no_subdirs, full_path=options.full_path,
            verbose_build=options.verbose_build,
            mrproper=options.mrproper,
            per_board_out_dir=options.per_board_out_dir,
            config_only=options.config_only,
            squash_config_y=not options.preserve_config_y,
            warnings_as_errors=options.warnings_as_errors,
            work_in_output=options.work_in_output,
            test_thread_exceptions=test_thread_exceptions,
            adjust_cfg=calc_adjust_cfg(options.adjust_cfg,
                                       options.reproducible_builds),
            allow_missing=get_allow_missing(options.allow_missing,
                                            options.no_allow_missing,
                                            len(selected), options.branch),
            no_lto=options.no_lto,
            reproducible_builds=options.reproducible_builds,
            force_build = options.force_build,
            force_build_failures = options.force_build_failures,
            force_reconfig = options.force_reconfig, in_tree = options.in_tree,
            force_config_on_failure=not options.quick, make_func=make_func)

    TEST_BUILDER = builder

    return run_builder(builder, series.commits if series else None,
                       brds.get_selected_dict(), options)
