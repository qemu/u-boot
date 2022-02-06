/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _CLI_HUSH_H_
#define _CLI_HUSH_H_

#define FLAG_EXIT_FROM_LOOP 1
#define FLAG_PARSE_SEMICOLON (1 << 1)	  /* symbol ';' is special for parser */
#define FLAG_REPARSING       (1 << 2)	  /* >=2nd pass */
#define FLAG_CONT_ON_NEWLINE (1 << 3)	  /* continue when we see \n */

extern int u_boot_hush_start(void);
extern int parse_string_outer(const char *, int);
#if CONFIG_IS_ENABLED(HUSH_OLD_PARSER)
extern int parse_file_outer(void);
#elif CONFIG_IS_ENABLED(HUSH_2021_PARSER)
extern void parse_and_run_file(void);
#endif

#if CONFIG_IS_ENABLED(HUSH_OLD_PARSER)
int set_local_var(const char *s, int flg_export);
#elif CONFIG_IS_ENABLED(HUSH_2021_PARSER)
int set_local_var(char *s, int flg_export);
#endif
void unset_local_var(const char *name);
char *get_local_var(const char *s);

#if defined(CONFIG_HUSH_INIT_VAR)
extern int hush_init_var (void);
#endif
#endif
