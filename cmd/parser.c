// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <cli.h>
#include <command.h>
#include <string.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static const char *gd_flags_to_parser(void)
{
	if (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		return "old";
	if (gd->flags & GD_FLG_HUSH_2021_PARSER)
		return "2021";
	return NULL;
}

static int do_parser_print(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	const char *current = gd_flags_to_parser();

	if (!current) {
		printf("current parser value is not valid, this should not happen!\n");
		return CMD_RET_FAILURE;
	}

	printf("%s\n", current);

	return CMD_RET_SUCCESS;
}

static int parser_string_to_gd_flags(const char *parser)
{
	if (!strcmp(parser, "old"))
		return GD_FLG_HUSH_OLD_PARSER;
	if (!strcmp(parser, "2021"))
		return GD_FLG_HUSH_2021_PARSER;
	return -1;
}

static void reset_parser_gd_flags(void)
{
	gd->flags &= ~GD_FLG_HUSH_OLD_PARSER;
	gd->flags &= ~GD_FLG_HUSH_2021_PARSER;
}

static int do_parser_set(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	char *parser_name;
	int parser_flag;

	if (argc < 2)
		return CMD_RET_USAGE;

	parser_name = argv[1];

	parser_flag = parser_string_to_gd_flags(parser_name);
	if (parser_flag == -1) {
		printf("Bad value for parser name: %s\n", parser_name);
		return CMD_RET_USAGE;
	}

	if (parser_flag == GD_FLG_HUSH_OLD_PARSER
		&& !CONFIG_IS_ENABLED(HUSH_OLD_PARSER)) {
		printf("Want to set current parser to old, but its code was not compiled!\n");
		return CMD_RET_FAILURE;
	}

	if (parser_flag == GD_FLG_HUSH_2021_PARSER
		&& !CONFIG_IS_ENABLED(HUSH_2021_PARSER)) {
		printf("Want to set current parser to 2021, but its code was not compiled!\n");
		return CMD_RET_FAILURE;
	}

	reset_parser_gd_flags();
	gd->flags |= parser_flag;

	cli_init();
	cli_loop();

	/* cli_loop() should never return. */
	return CMD_RET_FAILURE;
}

static struct cmd_tbl parser_sub[] = {
	U_BOOT_CMD_MKENT(print, 1, 1, do_parser_print, "", ""),
	U_BOOT_CMD_MKENT(set, 2, 1, do_parser_set, "", ""),
};

static int do_parser(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct cmd_tbl *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* drop initial "parser" arg */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], parser_sub, ARRAY_SIZE(parser_sub));
	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

#if CONFIG_IS_ENABLED(SYS_LONGHELP)
static char parser_help_text[] =
	"print - print current parser\n"
	"set - set the current parser, possible values are: old, 2021"
	;
#endif

U_BOOT_CMD(parser, 3, 1, do_parser,
	   "parser",
#if CONFIG_IS_ENABLED(SYS_LONGHELP)
	   parser_help_text
#endif
);
