// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2021
 * Francis Laniel, Amarula Solutions, francis.laniel@amarulasolutions.com
 */

#include <common.h>
#include <command.h>
#include <env_attr.h>
#include <test/hush.h>
#include <test/ut.h>

static int hush_test_simple_dollar(struct unit_test_state *uts)
{
	console_record_reset_enable();
	ut_assertok(run_command("echo $dollar_foo", 0));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-zero-length"
	/*
	 * Next line is empty as $dollar_foo was not set before, but compiler
	 * complains about the format being empty, so we disable this warning
	 * only for this line.
	 */
	ut_assert_nextline("");
#pragma GCC diagnostic pop
	ut_assert_console_end();

	ut_assertok(run_command("echo ${dollar_foo}", 0));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-zero-length"
	/*
	 * Next line is empty as $dollar_foo was not set before, but compiler
	 * complains about the format being empty, so we disable this warning
	 * only for this line.
	 */
	ut_assert_nextline("");
#pragma GCC diagnostic pop
	ut_assert_console_end();

	ut_assertok(run_command("dollar_foo=bar", 0));

	ut_assertok(run_command("echo $dollar_foo", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	ut_assertok(run_command("echo ${dollar_foo}", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	/*
	 * R is way of giving raw string literals in C++.
	 * It means a "\n" is a R string literal will not be interpreted as line
	 * feed but printed as "\n".
	 * GCC provides it for C as an extension.
	 */
	ut_assertok(run_command(R"(dollar_foo=\$bar)", 0));

	ut_assertok(run_command("echo $dollar_foo", 0));
	ut_assert_nextline("$bar");
	ut_assert_console_end();

	ut_assertok(run_command("dollar_foo='$bar'", 0));

	ut_assertok(run_command("echo $dollar_foo", 0));
	ut_assert_nextline("$bar");
	ut_assert_console_end();

	ut_asserteq(1, run_command("dollar_foo=bar quux", 0));
	/* Next line contains error message. */
	ut_assert_skipline();
	ut_assert_console_end();

	ut_asserteq(1, run_command("dollar_foo='bar quux", 0));
	/* Next line contains error message. */
	ut_assert_skipline();
#if CONFIG_IS_ENABLED(HUSH_2021_PARSER)
	/*
	 * For some strange reasons, the console is not empty after running
	 * above command.
	 * So, we reset it to not have side effects for other tests.
	 */
	console_record_reset_enable();
#else /* HUSH_OLD_PARSER */
	ut_assert_console_end();
#endif /* HUSH_OLD_PARSER */

	ut_asserteq(1, run_command(R"(dollar_foo=bar quux")", 0));
	/* Two next lines contain error message. */
	ut_assert_skipline();
	ut_assert_skipline();
#if CONFIG_IS_ENABLED(HUSH_2021_PARSER)
	/* See above comments. */
	console_record_reset_enable();
#else /* HUSH_OLD_PARSER */
	ut_assert_console_end();
#endif /* HUSH_OLD_PARSER */

	ut_assertok(run_command(R"(dollar_foo='bar "quux')", 0));

	ut_assertok(run_command("echo $dollar_foo", 0));
	/*
	 * This one is buggy.
	 * ut_assert_nextline(R"(bar "quux)");
	 * ut_assert_console_end();
	 *
	 * So, let's reset output:
	 */
	console_record_reset_enable();

#if CONFIG_IS_ENABLED(HUSH_2021_PARSER)
	/*
	 * Old parser returns an error because it waits for closing '\'', but
	 * this behavior is wrong as the '\'' is surrounded by '"', so no need
	 * to wait for a closing one.
	 */
	ut_assertok(run_command(R"(dollar_foo="bar 'quux")", 0));

	ut_assertok(run_command("echo $dollar_foo", 0));
	ut_assert_nextline("bar 'quux");
	ut_assert_console_end();
#else /* HUSH_OLD_PARSER */
	ut_asserteq(1, run_command(R"(dollar_foo="bar 'quux")", 0));
	/* Next line contains error message. */
	ut_assert_skipline();
	ut_assert_console_end();
#endif /* HUSH_OLD_PARSER */

	ut_assertok(run_command("dollar_foo='bar quux'", 0));
	ut_assertok(run_command("echo $dollar_foo", 0));
	ut_assert_nextline("bar quux");
	ut_assert_console_end();

#if CONFIG_IS_ENABLED(HUSH_2021_PARSER)
	/* Reset local variable. */
	ut_assertok(run_command("dollar_foo=", 0));
#else /* HUSH_OLD_PARSER */
	puts("Beware: this test set local variable dollar_foo and it cannot be unset!");
#endif /* HUSH_OLD_PARSER */

	return 0;
}
HUSH_TEST(hush_test_simple_dollar, 0);

static int hush_test_env_dollar(struct unit_test_state *uts)
{
	env_set("env_foo", "bar");
	console_record_reset_enable();

	ut_assertok(run_command("echo $env_foo", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	ut_assertok(run_command("echo ${env_foo}", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	/* Environment variables have precedence over local variable. */
	ut_assertok(run_command("env_foo=quux", 0));
	ut_assertok(run_command("echo ${env_foo}", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	/* Clean up setting the variable. */
	env_set("env_foo", NULL);

#if CONFIG_IS_ENABLED(HUSH_2021_PARSER)
	/* Reset local variable. */
	ut_assertok(run_command("env_foo=", 0));
#else /* HUSH_OLD_PARSER */
	puts("Beware: this test set local variable env_foo and it cannot be unset!");
#endif /* HUSH_OLD_PARSER */

	return 0;
}
HUSH_TEST(hush_test_env_dollar, 0);

static int hush_test_command_dollar(struct unit_test_state *uts)
{
	console_record_reset_enable();

	ut_assertok(run_command(R"(dollar_bar="echo bar")", 0));

	ut_assertok(run_command("$dollar_bar", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	ut_assertok(run_command("${dollar_bar}", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	ut_assertok(run_command(R"(dollar_bar="echo
	bar")", 0));

	ut_assertok(run_command("$dollar_bar", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	ut_assertok(run_command(R"(dollar_bar='echo bar
	')", 0));

	ut_assertok(run_command("$dollar_bar", 0));
	ut_assert_nextline("bar");
	ut_assert_console_end();

	ut_assertok(run_command(R"(dollar_bar='echo bar\n')", 0));

	ut_assertok(run_command("$dollar_bar", 0));
#if CONFIG_IS_ENABLED(HUSH_2021_PARSER)
	/*
	 * This difference seems to come from a bug solved in Busybox hush.
	 * Behavior of hush 2021 is coherent with bash and other shells.
	 */
	ut_assert_nextline(R"(bar\n)");
#else /* HUSH_OLD_PARSER */
	ut_assert_nextline("barn");
#endif /* HUSH_OLD_PARSER */
	ut_assert_console_end();

	ut_assertok(run_command("dollar_bar='echo $bar'", 0));

	ut_assertok(run_command("$dollar_bar", 0));
	ut_assert_nextline("$bar");
	ut_assert_console_end();

	ut_assertok(run_command("dollar_quux=quux", 0));
	ut_assertok(run_command(R"(dollar_bar="echo $dollar_quux")", 0));

	ut_assertok(run_command("$dollar_bar", 0));
	ut_assert_nextline("quux");
	ut_assert_console_end();

#if CONFIG_IS_ENABLED(HUSH_2021_PARSER)
	/* Reset local variables. */
	ut_assertok(run_command("dollar_bar=", 0));
	ut_assertok(run_command("dollar_quux=", 0));
#else /* HUSH_OLD_PARSER */
	puts("Beware: this test sets local variable dollar_bar and dollar_quux and they cannot be unset!");
#endif /* HUSH_OLD_PARSER */

	return 0;
}
HUSH_TEST(hush_test_command_dollar, 0);
