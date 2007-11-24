/*
 * globbing.c: interface to the POSIX glob routines
 *  
 * Copyright (C) 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2001, 2002, 2003, 2006, 2007 Colin Watson.
 *
 * This file is part of man-db.
 *
 * man-db is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * man-db is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with man-db; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>

#include "argp.h"
#include "dirname.h"

#include "gettext.h"
#define _(String) gettext (String)
#define N_(String) gettext_noop (String)

#include "manconfig.h"
#include "globbing.h"

char *program_name;

extern const char *extension;
static int match_case = 0;
static char **remaining_args;

const char *argp_program_version = "globbing " PACKAGE_VERSION;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;
error_t argp_err_exit_status = FAIL;

static const char args_doc[] = N_("path section name");

static struct argp_option options[] = {
	{ "debug",		'd',	0,			0,	N_("emit debugging messages") },
	{ "extension",		'e',	N_("EXTENSION"),	0,	N_("limit search to extension type EXTENSION") },
	{ "ignore-case",	'i',	0,			0,	N_("look for pages case-insensitively (default)") },
	{ "match-case",		'I',	0,			0,	N_("look for pages case-sensitively") },
	{ 0, 'h', 0, OPTION_HIDDEN, 0 }, /* compatibility for --help */
	{ 0 }
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
		case 'd':
			debug_level = 1;
			return 0;
		case 'e':
			extension = arg;
			return 0;
		case 'i':
			match_case = 0;
			return 0;
		case 'I':
			match_case = 1;
			return 0;
		case 'h':
			argp_state_help (state, state->out_stream,
					 ARGP_HELP_STD_HELP);
			break;
		case ARGP_KEY_ARGS:
			if (state->argc - state->next != 3)
				argp_usage (state);
			remaining_args = state->argv + state->next;
			return 0;
	}
	return ARGP_ERR_UNKNOWN;
}

static struct argp argp = { options, parse_opt, args_doc };

int main (int argc, char **argv)
{
	int i;

	program_name = base_name (argv[0]);

	if (argp_parse (&argp, argc, argv, 0, 0, 0))
		exit (FAIL);

	for (i = 0; i <= 1; i++) {
		char **files;

		files = look_for_file (remaining_args[0], remaining_args[1],
				       remaining_args[2], i, match_case);
		if (files)
			while (*files)
				printf ("%s\n", *files++);
	}
	return 0;
}
