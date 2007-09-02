/*
 * man.c: The manual pager
 *
 * Copyright (C) 1990, 1991 John W. Eaton.
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2001, 2002, 2003 Colin Watson.
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
 *
 * John W. Eaton
 * jwe@che.utexas.edu
 * Department of Chemical Engineering
 * The University of Texas at Austin
 * Austin, Texas  78712
 *
 * Mostly written/re-written by Wilf, some routines by Markus Armbruster.
 *
 * CJW: Various robustness, security, and internationalization fixes.
 * Improved HTML support (originally written by Fabrizio Polacco).
 * Rewrite of page location routines for improved maintainability and
 * accuracy.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <termios.h>

#if defined(STDC_HEADERS)
#  include <string.h>
#  include <stdlib.h>
#elif defined(HAVE_STRING_H)
#  include <string.h>
#elif defined(HAVE_STRINGS_H)
#  include <strings.h>
#else /* no string(s) header */
extern char *strchr(), *strcat();
#endif /* STDC_HEADERS */

#if defined(HAVE_UNISTD_H)
#  include <unistd.h>
#else
extern pid_t vfork();
#  define R_OK		4
#  define STDOUT_FILENO	1
#  define STDIN_FILENO	0
#endif /* HAVE_UNISTD_H */

#if defined(HAVE_LIMITS_H)
#  include <limits.h>
#elif defined(HAVE_SYS_PARAM_H)
#  include <sys/param.h>
#endif

static char *cwd;

#ifndef PIPE_BUF
#  if defined(_POSIX_VERSION) && defined(_POSIX_PIPE_MAX)
#    define PIPE_MAX _POSIX_PIPE_MAX
#  else /* !_POSIX_PIPE_MAX */
#    if defined(PIPE_MAX) && (PIPE_MAX != INT_MAX)
#      define PIPE_BUF PIPE_MAX
#    else /* !PIPE_MAX */
#      define PIPE_BUF 512
#    endif /* PIPE_MAX */
#  endif /* _POSIX_PIPE_MAX */
#endif /* PIPE_BUF */

#ifdef HAVE_SYS_FILE
#  include <sys/file.h>
#endif /* HAVE_SYS_FILE */

#if HAVE_FCNTL_H
#  include <fcntl.h>
#endif

#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/stat.h>

#if HAVE_SYS_WAIT_H
#  include <sys/wait.h>
#endif

#ifndef STDC_HEADERS
extern char *getenv();
extern int errno;
#endif

#ifdef HAVE_LIBGEN_H
#  include <libgen.h>
#endif /* HAVE_LIBGEN_H */

#ifdef HAVE_GETOPT_H
#  include <getopt.h>
#else /* !HAVE_GETOPT_H */
#  include "lib/getopt.h"
#endif /* HAVE_GETOPT_H */

#include "lib/gettext.h"
#include <locale.h>
#define _(String) gettext (String)

#include "manconfig.h"
#include "libdb/mydbm.h"
#include "libdb/db_storage.h"
#include "lib/error.h"
#include "lib/cleanup.h"
#include "lib/setenv.h"
#include "lib/hashtable.h"
#include "lib/pipeline.h"
#include "lib/getcwdalloc.h"
#include "lib/pathsearch.h"
#include "lib/linelength.h"
#include "lib/decompress.h"
#include "check_mandirs.h"
#include "filenames.h"
#include "globbing.h"
#include "ult_src.h"
#include "manp.h"
#include "security.h"
#include "encodings.h"
#include "convert_name.h"
#include "man.h"

#ifdef SECURE_MAN_UID
extern uid_t ruid;
extern uid_t euid;
#endif /* SECURE_MAN_UID */

/* the magic cookie to request preprocessing */
#define PP_COOKIE "'\\\" "

/* the default preprocessor sequence */
#ifndef DEFAULT_MANROFFSEQ
#  define DEFAULT_MANROFFSEQ ""
#endif

/* placeholder for the manual page name in the less prompt string */
#define MAN_PN "$MAN_PN"

/* Some systems lack these */
#ifndef STDIN_FILENO
#  define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#  define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#  define STDERR_FILENO 2
#endif

const char *lang;

/* external formatter programs, one for use without -t, and one with -t */
#define NFMT_PROG "./mandb_nfmt"
#define TFMT_PROG "./mandb_tfmt"
#undef ALT_EXT_FORMAT	/* allow external formatters located in cat hierarchy */

static int global_manpath = -1;	/* global or user manual page hierarchy? */
static int skip;		/* page exists but has been skipped */

#if defined _AIX || defined __sgi
char **global_argv;
#endif

struct candidate {
	const char *req_name;
	char from_db;
	char cat;
	const char *path;
	struct mandata *source;
	struct candidate *next;
};

#define CANDIDATE_FILESYSTEM 0
#define CANDIDATE_DATABASE   1

static char *lang_dir (const char *filename)
{
	char *ld;	/* the lang dir: point to static data */
	const char *fm;	/* the first "/man/" dir */
	const char *sm;	/* the second "/man?/" dir */

	ld = xstrdup ("");
	if (!filename) 
		return ld;

	/* Check whether filename is in a man page hierarchy. */
	fm = strstr (filename, "/man/");
	if (!fm)
		return ld;
	sm = strstr (fm + 3, "/man");
	if (!sm)
		return ld;
	if (sm[5] != '/')
		return ld;
	if (!strchr ("123456789lno", sm[4]))
		return ld;

	/* If there's no lang dir element, it's an English man page. */
	if (sm == fm + 4)
		return xstrdup ("C");

	/* found a lang dir */
	fm += 5;
	sm = strchr (fm, '/');
	if (!sm)
		return ld;
	free (ld);
	ld = xstrndup (fm, sm - fm);
	debug ("found lang dir element %s\n", ld);
	return ld;
}

static __inline__ void gripe_system (pipeline *p, int status)
{
	error (CHILD_FAIL, 0, _("command exited with status %d: %s"),
	       status, pipeline_tostring (p));
}


static int checked_system (pipeline *p)
{
	int status;

	status = do_system_drop_privs (p);
	if (status != 0)
		gripe_system (p, status);

	return status;
}


static char *manpathlist[MAXDIRS];

/* globals */
int quiet = 1;
char *program_name;
char *database;
MYDBM_FILE dbf; 
extern const char *extension; /* for globbing.c */
extern char *user_config_file;	/* defined in manp.c */
extern int disable_cache;
extern int optind;

/* locals */
static const char *alt_system_name;
static const char **section_list;		
static const char *section;
static char *colon_sep_section_list;
static const char *preprocessors;
static const char *pager;
static const char *locale;
static char *internal_locale;
static char *prompt_string;
static char *less;
static const char *std_sections[] = STD_SECTIONS;
static char *manp;
static const char *external;
static struct hashtable *db_hash = NULL;

static int troff;
static const char *roff_device = NULL;
static int print_where, print_where_cat;
static int catman;
static int local_man_file;
static int findall;
static int update;
static int match_case;

static int ascii;		/* insert tr in the output pipe */
static int save_cat; 		/* security breach? Can we save the cat? */
static int different_encoding;	/* was an explicit encoding specified? */

static int found_a_stray;		/* found a straycat */

#ifdef MAN_CATS
static char *tmp_cat_file;	/* for open_cat_stream(), close_cat_stream() */
static int tmp_cat_fd;
static int created_tmp_cat;			/* dto. */
static int man_modtime;		/* modtime of man page, for commit_tmp_cat() */
#endif

static const struct option long_options[] =
{
    {"local-file", no_argument, 	0, 'l'},
    {"manpath", required_argument, 	0, 'M'},
    {"pager", required_argument, 	0, 'P'},
    {"sections", required_argument, 	0, 'S'},
    {"all", no_argument, 		0, 'a'},
    {"debug", no_argument, 		0, 'd'},
    {"whatis", no_argument, 		0, 'f'},
    {"help", no_argument, 		0, 'h'},
    {"apropos", no_argument, 		0, 'k'},
    {"version", no_argument, 		0, 'V'},
    {"systems", required_argument, 	0, 'm'},
    {"preprocessor", required_argument,	0, 'p'},
    {"location", no_argument, 		0, 'w'},
    {"where", no_argument,		0, 'w'},
    {"location-cat", no_argument,	0, 'W'},
    {"where-cat", no_argument,		0, 'W'},
    {"locale", required_argument,	0, 'L'},
    {"extension", required_argument,	0, 'e'},
    {"update", no_argument, 		0, 'u'},
    {"prompt", required_argument,	0, 'r'},
    {"default", no_argument,		0, 'D'},
    {"ascii", no_argument,		0, '7'},
    {"catman", no_argument, 		0, 'c'},
    {"encoding", required_argument,	0, 'E'},
    {"ignore-case", no_argument,	0, 'i'},
    {"match-case", no_argument,		0, 'I'},
    {"config-file", required_argument,	0, 'C'},

#ifdef HAS_TROFF
    {"troff", no_argument, 		0, 't'},
    {"troff-device", optional_argument,	0, 'T'},
# ifdef TROFF_IS_GROFF
    {"ditroff", no_argument, 		0, 'Z'},
    {"gxditview", optional_argument,	0, 'X'},
    {"html", optional_argument,		0, 'H'},
# endif
    {0, 0, 0, 0}
};

static const char args[] = "7DlM:P:S:adfhH::kVum:p:tT::wWe:L:Zcr:X::E:iIC:";

# ifdef TROFF_IS_GROFF
static int ditroff;
static const char *gxditview;
static int htmlout;
static const char *html_pager;
# endif /* TROFF_IS_GROFF */

#else /* !HAS_TROFF */

    {0, 0, 0, 0}
};

static const char args[] = "7DlM:P:S:adfhkVum:p:wWe:L:cr:E:iIC:";

#endif /* HAS_TROFF */

/* issue a usage message, then exit with the given status */
static void usage (int status)
{
#ifdef HAS_TROFF
#  ifdef TROFF_IS_GROFF
	const char formatter[] = "groff";
#  else
	const char formatter[] = "troff";
#  endif /* TROFF_IS_GROFF */
#endif /* HAS_TROFF */
	
#ifdef HAS_TROFF
	printf (_(
		"usage: %s [-c|-f|-k|-w|-tZT device] [-i|-I] [-adlhu7V] [-Mpath] [-Ppager]\n"
		"           [-Cfile] [-Slist] [-msystem] [-pstring] [-Llocale] [-eextension]\n"
		"           [section] page ...\n"),
		program_name);
#else
	printf (_(
		"usage: %s [-c|-f|-k|-w] [-i|-I] [-adlhu7V] [-Mpath] [-Ppager]\n"
		"           [-Cfile] [-Slist] [-msystem] [-pstring] [-Llocale] [-eextension]\n"
		"           [section] page ...\n"),
		program_name);
#endif

	puts (_(
		"-a, --all                   find all matching manual pages.\n"
		"-d, --debug                 emit debugging messages.\n"
		"-e, --extension             limit search to extension type `extension'.\n"
		"-f, --whatis                equivalent to whatis.\n"
		"-k, --apropos               equivalent to apropos.\n"
		"-w, --where, --location     print physical location of man page(s).\n"
		"-W, --where-cat,\n"
		"    --location-cat          print physical location of cat file(s).\n"
		"-l, --local-file            interpret `page' argument(s) as local filename(s).\n"
		"-u, --update                force a cache consistency check.\n"
		"-i, --ignore-case           look for pages case-insensitively (default).\n"
		"-I, --match-case            look for pages case-sensitively.\n"
		"-r, --prompt string         provide the `less' pager with a prompt\n"
		"-c, --catman                used by catman to reformat out of date cat pages.\n"
		"-7, --ascii                 display ASCII translation of certain latin1 chars.\n"
		"-E, --encoding encoding     use the selected nroff device and display in pager."));
#ifdef HAS_TROFF
	printf (_(
		"-t, --troff                 use %s to format pages.\n"
		"-T, --troff-device device   use %s with selected device.\n"),
		formatter, formatter);
# ifdef TROFF_IS_GROFF
	puts (_("-H, --html                  use lynx or argument to display html output.\n"
		"-Z, --ditroff               use groff and force it to produce ditroff.\n"
		"-X, --gxditview             use groff and display through gxditview (X11):\n"
		"                            -X = -TX75, -X100 = -TX100, -X100-12 = -TX100-12."));
# endif /* TROFF_IS_GROFF */
#endif /* HAS_TROFF */

	puts (_(
		"-D, --default               reset all options to their default values.\n"
		"-C, --config-file file      use this user configuration file.\n"
		"-M, --manpath path          set search path for manual pages to `path'.\n"
		"-P, --pager pager           use program `pager' to display output.\n"
		"-S, --sections list         use colon separated section list.\n"
		"-m, --systems system        search for man pages from other unix system(s).\n"
		"-L, --locale locale         define the locale for this particular man search.\n"
		"-p, --preprocessor string   string indicates which preprocessors to run.\n"
		"                             e - [n]eqn   p - pic    t - tbl\n"
		"                             g - grap     r - refer  v - vgrind\n"
		"-V, --version               show version.\n"
		"-h, --help                  show this usage message."));

	exit (status);
}

/*
 * changed these messages from stdout to stderr,
 * (Fabrizio Polacco) Fri, 14 Feb 1997 01:30:07 +0200
 */
static void gripe_no_name (const char *sect)
{
	if (sect) {
		fprintf (stderr, _("No manual entry for %s\n"), sect);
		fprintf (stderr,
			 _("(Alternatively, what manual page do you want from "
			   "section %s?)\n"),
			 sect);
	} else
		fputs (_("What manual page do you want?\n"), stderr);

	exit (FAIL);
}

static struct termios tms;
static int tms_set = 0;

static void set_term (void)
{
	if (tms_set)
		tcsetattr (fileno (stdin), TCSANOW, &tms);
}

static void get_term (void)
{
	if (isatty (fileno (stdout))) {
		debug ("is a tty\n");
		tcgetattr (fileno (stdin), &tms);
		if (!tms_set++)
			atexit (set_term);
	}
}

static int get_roff_line_length (void)
{
	int line_length = get_line_length ();

	/* groff >= 1.18 defaults to 78. */
	if (!troff && line_length != 80) {
		int length = line_length * 39 / 40;
		if (length > line_length - 2)
			return line_length - 2;
		else
			return length;
	} else
		return 0;
}

static void add_roff_line_length (command *cmd, int *save_cat_p)
{
	int length = get_roff_line_length ();
	if (length) {
		char optionll[32], optionlt[32];
		debug ("Using %d-character lines\n", length);
		*save_cat_p = 0;
		sprintf (optionll, "-rLL=%dn", length);
		sprintf (optionlt, "-rLT=%dn", length);
		command_args (cmd, optionll, optionlt, NULL);
	}
}

static __inline__ void gripe_no_man (const char *name, const char *sec)
{
	/* On AIX and IRIX, fall back to the vendor supplied browser. */
#if defined _AIX || defined __sgi
	if (!troff) {
		putenv ("MANPATH=");  /* reset the MANPATH variable */
		execv ("/usr/bin/man", global_argv);
	}
#endif

	if (sec)
		fprintf (stderr, _("No manual entry for %s in section %s\n"),
			 name, sec);
	else
		fprintf (stderr, _("No manual entry for %s\n"), name);

#ifdef UNDOC_COMMAND
	if (pathsearch_executable (name))
		fprintf (stderr,
			 _("See '%s' for help when manual pages are not "
			   "available.\n"), UNDOC_COMMAND);
#endif
}

/* fire up the appropriate external program */
static void do_extern (int argc, char *argv[])
{
	pipeline *p;
	command *cmd;

	cmd = command_new (external);
	/* Please keep these in the same order as they are in whatis.c. */
	if (debug_level)
		command_arg (cmd, "-d");
	if (alt_system_name)
		command_args (cmd, "-s", alt_system_name, NULL);
	if (manp)
		command_args (cmd, "-M", manp, NULL);
	if (locale)
		command_args (cmd, "-L", locale, NULL);
	if (user_config_file)
		command_args (cmd, "-C", user_config_file, NULL);
	while (optind < argc)
		command_arg (cmd, argv[optind++]);
	p = pipeline_new_commands (cmd, NULL);

	/* privs are already dropped */
	pipeline_start (p);
	exit (pipeline_wait (p));
}

/* lookup $MANOPT and if available, put in *argv[] format for getopt() */
static __inline__ char **manopt_to_env (int *argc)
{
	char *manopt, *opt_start, **argv;

	manopt = getenv ("MANOPT");
	if (manopt == NULL || *manopt == '\0')
		return NULL;

	opt_start = manopt = xstrdup (manopt);

	/* allocate space for the program name */
	*argc = 0;
	argv = (char **) xmalloc ((*argc + 3) * sizeof (char *));
	argv[(*argc)++] = program_name;
	
	/* for each [ \t]+ delimited string, allocate an array space and fill
	   it in. An escaped space is treated specially */	
	while (*manopt) {
		switch (*manopt) {
			case ' ':
			case '\t':
				if (manopt != opt_start) {
					*manopt = '\0';
					argv = (char **) 
					       xrealloc (argv, (*argc + 3) * 
							 sizeof (char *));
					argv[(*argc)++] = opt_start;
				}
				while (CTYPE (isspace, *(manopt + 1)))
					*++manopt = '\0';
				opt_start = manopt + 1;
				break;
			case '\\':
				if (*(manopt + 1) == ' ')
					manopt++;
				break;
			default:
				break;
		}
		manopt++;
	}

	if (*opt_start)
		argv[(*argc)++] = opt_start;
	argv[*argc] = NULL;			

	return argv;
}

/* Return char array with 'less' special chars escaped. Uses static storage. */
static __inline__ const char *escape_less (const char *string)
{
	static char *escaped_string; 
	char *ptr;

	/* 2*strlen will always be long enough to hold the escaped string */
	ptr = escaped_string = (char *) xrealloc (escaped_string, 
						  2 * strlen (string) + 1);
	
	while (*string) {
		if (*string == '?' ||
		    *string == ':' ||
		    *string == '.' ||
		    *string == '%' ||
		    *string == '\\')
			*ptr++ = '\\';

		*ptr++ = *string++;
	}

	*ptr = *string;
	return escaped_string;
}

#if defined(MAN_DB_CREATES) || defined(MAN_DB_UPDATES)
/* Run mandb to ensure databases are up to date. Only used with -u.
 * Returns the exit status of mandb.
 *
 * If filename is non-NULL, uses mandb's -f option to update a single file.
 */
static int run_mandb (int create, const char *manpath, const char *filename)
{
	pipeline *mandb_pl = pipeline_new ();
	command *mandb_cmd = command_new ("mandb");

	if (debug_level)
		command_arg (mandb_cmd, "-d");
	else
		command_arg (mandb_cmd, "-q");

	if (filename)
		command_args (mandb_cmd, "-f", filename, NULL);
	else if (create)
		command_arg (mandb_cmd, "-c");
	else
		command_arg (mandb_cmd, "-p");

	if (manpath)
		command_arg (mandb_cmd, manpath);

	pipeline_command (mandb_pl, mandb_cmd);

	if (debug_level) {
		debug ("running mandb: ");
		pipeline_dump (mandb_pl, stderr);
	}

	pipeline_start (mandb_pl);
	return pipeline_wait (mandb_pl);
}
#endif /* MAN_DB_CREATES || MAN_DB_UPDATES */


/* man issued with `-l' option */
static int local_man_loop (const char *argv)
{
	int exit_status = OK;
	int local_mf = local_man_file;

	drop_effective_privs ();
	local_man_file = 1;
	if (strcmp (argv, "-") == 0)
		display (NULL, "", NULL, "(stdin)", NULL);
	else {
		struct stat st;

		if (cwd[0]) {
			debug ("chdir %s\n", cwd);
			if (chdir (cwd)) {
				error (0, errno, _("can't chdir to %s"), cwd);
				regain_effective_privs ();
				return 0;
			}
		}

		/* Check that the file exists and isn't e.g. a directory */
		if (stat (argv, &st)) {
			error (0, errno, "%s", argv);
			return NOT_FOUND;
		}

		if (S_ISDIR (st.st_mode)) {
			error (0, EISDIR, "%s", argv);
			return NOT_FOUND;
		}

		if (S_ISCHR (st.st_mode) || S_ISBLK (st.st_mode)) {
			/* EINVAL is about the best I can do. */
			error (0, EINVAL, "%s", argv);
			return NOT_FOUND;
		}

		if (exit_status == OK) {
			char *argv_copy = xstrdup (argv);
			lang = lang_dir (argv);
			if (!display (NULL, argv, NULL, basename (argv_copy),
				      NULL)) {
				if (local_mf)
					error (0, errno, "%s", argv);
				exit_status = NOT_FOUND;
			}
			free (argv_copy);
		}
	}
	local_man_file = local_mf;
	regain_effective_privs ();
	return exit_status;
}

static void int_handler (int signo)
{
	debug ("\ninterrupt signal %d handler\n", signo);
	exit (INTERRUPTED);
}

int main (int argc, char *argv[])
{
	int argc_env, exit_status = OK;
	char **argv_env;
	const char *tmp;
	char *multiple_locale = NULL;

	program_name = xstrdup (basename (argv[0]));

	umask (022);
	/* initialise the locale */
	if (!setlocale (LC_ALL, ""))
		/* Obviously can't translate this. */
		error (0, 0, "can't set the locale; make sure $LC_* and $LANG "
			     "are correct");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);

	internal_locale = setlocale (LC_MESSAGES, NULL);
	/* Use LANGUAGE only when LC_MESSAGES locale category is
	 * neither "C" nor "POSIX". */
	if (internal_locale && strcmp (internal_locale, "C") &&
	    strcmp (internal_locale, "POSIX")) {
		multiple_locale = getenv ("LANGUAGE");
		if (multiple_locale && *multiple_locale)
			internal_locale = multiple_locale;
	}
	internal_locale = xstrdup (internal_locale ? internal_locale : "C");

/* export argv, it might be needed when invoking the vendor supplied browser */
#if defined _AIX || defined __sgi
	global_argv = argv;
#endif

	{ /* opens base streams in case someone like "info" closed them */
		struct stat buf;
		if (fileno (stdin) < 0 ||
		    ((fstat (fileno (stdin), &buf) < 0) && (errno == EBADF))) 
			freopen ("/dev/null", "r", stdin);
		if (fileno (stdout) < 0 ||
		    ((fstat (fileno (stdout), &buf) < 0) && (errno == EBADF)))
			freopen ("/dev/null", "w", stdout);
		if (fileno (stderr) < 0 ||
		    ((fstat (fileno (stderr), &buf) < 0) && (errno == EBADF)))
			freopen ("/dev/null", "w", stderr);
	}

	/* This will enable us to do some profiling and know
	where gmon.out will end up. Must chdir(cwd) before we return */
	cwd = getcwd_allocated ();
	if (!cwd) {
		cwd = xmalloc (1);
		cwd[0] = '\0';
	}

	/* First of all, find out if $MANOPT is set. If so, put it in 
	   *argv[] format for getopt to play with. */
	argv_env = manopt_to_env (&argc_env);
	if (argv_env) {
		man_getopt (argc_env, argv_env);
		optind = 0;
	}

	/* give the actual program args to getopt */
	man_getopt (argc, argv);

#ifdef SECURE_MAN_UID
	/* record who we are and drop effective privs for later use */
	init_security ();
#endif /* SECURE_MAN_UID */

	signal (SIGINT, int_handler);

	pipeline_install_sigchld ();

	read_config_file ();

	/* if the user wants whatis or apropos, give it to them... */
	if (external)
		do_extern (argc, argv);

	get_term (); /* stores terminal settings */
#ifdef SECURE_MAN_UID
	debug ("real user = %d; effective user = %d\n", ruid, euid);
#endif /* SECURE_MAN_UID */

#ifdef HAVE_SETLOCALE
	/* close this locale and reinitialise if a new locale was 
	   issued as an argument or in $MANOPT */
	if (locale) {
		free (internal_locale);
		internal_locale = xstrdup (setlocale (LC_ALL, locale));
		if (internal_locale == NULL)
			internal_locale = xstrdup (locale);

		debug ("main(): locale = %s, internal_locale = %s\n",
		       locale, internal_locale);
		if (internal_locale) {
			extern int _nl_msg_cat_cntr;
			setenv ("LANGUAGE", internal_locale, 1);
			++_nl_msg_cat_cntr;
			multiple_locale = NULL;
		}
	}

#endif /* HAVE_SETLOCALE */

#ifdef TROFF_IS_GROFF
	if (htmlout) {
		if (!html_pager) {
			html_pager = getenv ("BROWSER");
			if (!html_pager)
				html_pager = WEB_BROWSER;
		}
		pager = html_pager;
	}
#endif
	if (pager == NULL) {
		pager = getenv ("MANPAGER");
		if (pager == NULL) {
			pager = getenv ("PAGER");
			if (pager == NULL)
				pager = get_def_user ("pager", PAGER);
		}
	}
	if (*pager == '\0')
		pager = get_def_user ("cat", CAT);

	if (prompt_string == NULL)
#ifdef LESS_PROMPT
		prompt_string = LESS_PROMPT;
#else
		prompt_string = _(
				" Manual page " MAN_PN
				" ?ltline %lt?L/%L.:byte %bB?s/%s..?e (END):"
				"?pB %pB\\%..");
#endif

	/* Restore and save $LESS in $MAN_ORIG_LESS so that recursive uses
	 * of man work as expected.
	 */
	less = getenv ("MAN_ORIG_LESS");
	if (less == NULL)
		less = getenv ("LESS");
	setenv ("MAN_ORIG_LESS", less ? less : "", 1);

	debug ("\nusing %s as pager\n", pager);

	if (optind == argc)
		gripe_no_name (NULL);

	/* man issued with `-l' option */
	if (local_man_file) {
		while (optind < argc) {
			exit_status = local_man_loop (argv[optind]);
			optind++;
		}
		free (cwd);
		free (internal_locale);
		free (program_name);
		exit (exit_status);
	}

	if (manp == NULL) {
		char tmp_locale[3];
		int idx;

		manp = add_nls_manpath (get_manpath (alt_system_name),
					internal_locale);
		/* Handle multiple :-separated locales in LANGUAGE */
		idx = multiple_locale ? strlen (multiple_locale) : 0;
		while (idx) {
			while (idx && multiple_locale[idx] != ':')
				idx--;
			if (multiple_locale[idx] == ':')
				idx++;
			tmp_locale[0] = multiple_locale[idx];
			tmp_locale[1] = multiple_locale[idx + 1];
			tmp_locale[2] = 0;
			/* step back over preceding ':' */
			if (idx) idx--;
			if (idx) idx--;
			debug ("checking for locale %s\n", tmp_locale);
			manp = add_nls_manpath (manp, tmp_locale);
		}
	} else
		free (get_manpath (NULL));

	create_pathlist (manp, manpathlist);

	debug ("*manpath search path* = %s\n", manp);

	/* finished manpath processing, regain privs */
	regain_effective_privs ();

	section_list = get_section_list ();

#ifdef MAN_DB_UPDATES
	/* If `-u', do it now. */
	if (update) {
		int status = run_mandb (0, NULL, NULL);
		if (status)
			error (0, 0,
			       _("mandb command failed with exit status %d"),
			       status);
	}
#endif /* MAN_DB_UPDATES */

	while (optind < argc) {
		int status = OK;
		int found = 0;
		static int maybe_section = 0;
		const char *nextarg = argv[optind++];

		/*
     		 * See if this argument is a valid section name.  If not,
      		 * is_section returns NULL.
      		 */
		tmp = is_section (nextarg);
		if (tmp) {
			section = tmp;
			debug ("\nsection: %s\n", section);
			maybe_section = 1;
		}

		if (maybe_section) {
			if (optind < argc)
				/* e.g. 'man 3perl Shell' */
				nextarg = argv[optind++];
			else
				/* e.g. 'man 9wm' */
				section = NULL;
				/* ... but leave maybe_section set so we can
				 * tell later that this happened.
				 */
		}

		/* this is where we actually start looking for the man page */
		skip = 0;
		status = man (nextarg, &found);

		/* clean out the cache of database lookups for each man page */
		hash_free (db_hash);
		db_hash = NULL;

		if (section && maybe_section) {
			if (status != OK && !catman) {
				/* Maybe the section wasn't a section after
				 * all? e.g. 'man 9wm fvwm'.
				 */
				debug ("\nRetrying section %s as name\n",
				       section);
				tmp = section;
				section = NULL;
				status = man (tmp, &found);
				hash_free (db_hash);
				db_hash = NULL;
				/* ... but don't gripe about it if it doesn't
				 * work!
				 */
				if (status == OK) {
					/* It was a name after all, so arrange
					 * to try the next page again with a
					 * null section.
					 */
					nextarg = tmp;
					--optind;
				} else
					/* No go, it really was a section. */
					section = tmp;
			}
		}

		if (status != OK && !catman) {
			if (!skip) {
				exit_status = status;
				if (exit_status == NOT_FOUND) {
					if (!section && maybe_section &&
					    CTYPE (isdigit, nextarg[0]))
						gripe_no_name (nextarg);
					else
						gripe_no_man (nextarg, section);
				}
			}
		} else {
			debug ("\nFound %d man pages\n", found);
			if (catman) {
				printf ("%s", nextarg);
				if (section)
					printf ("(%s)", section);
				if (optind != argc)
					fputs (", ", stdout);
				else
					fputs (".\n", stdout);
			}
		}

		maybe_section = 0;

		chkr_garbage_detector ();
	}
	hash_free (db_hash);
	db_hash = NULL;

	drop_effective_privs ();

	/* For profiling */
	if (cwd[0])
		chdir (cwd);

	free_pathlist (manpathlist);
	free (cwd);
	free (internal_locale);
	free (program_name);
	exit (exit_status);
}

/* parse the arguments contained in *argv[] and set appropriate vars */
static void man_getopt (int argc, char *argv[])
{
	int c, option_index; /* not used, but required by getopt_long() */
	static int apropos, whatis; /* retain values between calls */

	while ((c = getopt_long (argc, argv, args,
				 long_options, &option_index)) != EOF) {

		switch (c) {

			case 'l':
				local_man_file = 1;
				break;
			case 'e':
				extension = optarg;
				break;
			case 'M':
				manp = optarg;
				break;
		    	case 'P':
				pager = optarg;
				break;
		    	case 'S':
				if (*optarg)
					colon_sep_section_list = optarg;
				break;
			case 'V':
				ver();
				break;
		    	case 'a':
				findall = 1; 
				break;
			case 'u':
				update = 1;
				break;
			case 'i':
				match_case = 0;
				break;
			case 'I':
				match_case = 1;
				break;
			case 'c':
				catman = 1;
				break;
		    	case 'd':
				debug_level = 1;
				break;
		    	case 'f':
				external = WHATIS;
				apropos = 1;
				break;
		    	case 'k':
				external = APROPOS;
				whatis = 1;
				break;
		    	case 'm':
				alt_system_name = optarg;
				break;
			case 'L':
				locale = optarg;
				break;
		    	case 'p':
				preprocessors = optarg;
				break;
			case '7':
				ascii = 1;
				break;
			case 'E':
				roff_device = optarg;
				different_encoding = 1;
				break;
#ifdef HAS_TROFF
		    	case 't':
				troff = 1;
				break;

			case 'T':
				/* @@@ traditional nroff knows -T,
				   troff does not (gets ignored).
				   All incarnations of groff know it.
				   Why does -T imply -t? */
				/* as this is an optional argument */
				roff_device = (optarg ? optarg : "ps");
				troff = 1;
				break;
			case 'X':
#ifdef TROFF_IS_GROFF
				troff = 1;
				gxditview = (optarg ? optarg : "75");
#endif /* TROFF_IS_GROFF */
				break;
			case 'H':
#ifdef TROFF_IS_GROFF
				html_pager = (optarg ? optarg : NULL);
				htmlout = 1;
				troff = 1;
				roff_device = "html";
#endif /* TROFF_IS_GROFF */
				break;
			case 'Z':
#ifdef TROFF_IS_GROFF
				ditroff = 1;
				troff = 1;
#endif /* TROFF_IS_GROFF */
				break;
#endif /* HAS_TROFF */
		    	case 'w':
				print_where = 1;
				break;
			case 'W':
				print_where_cat = 1;
				break;
			case 'r':
				prompt_string = optarg;
				break;
			case 'C':
				user_config_file = optarg;
				break;
			case 'D':
		    		/* discard all preset options */
		    		local_man_file = findall = update = catman =
					debug_level = troff =
					print_where = print_where_cat =
					ascii = different_encoding =
					match_case = 0;
#ifdef TROFF_IS_GROFF
				ditroff = 0;
				gxditview = NULL;
				htmlout = 0;
				html_pager = NULL;
#endif
		    		roff_device = extension = pager = locale
		    			     = alt_system_name = external
		    			     = preprocessors = NULL;
				colon_sep_section_list = manp = NULL;
		    		break;
		    	case 'h':
		    		usage(OK);
		    		break;
		    	default:
				usage(FAIL);
				break;
		}
	}

	/* check for incompatible options */
	if (troff + whatis + apropos + catman +
	    (print_where || print_where_cat) > 1) {
		error (0, 0,
		       strappend (NULL,
				  troff ? "-[tTZH] " : "",
				  whatis ? "-f " : "",
				  apropos ? "-k " : "",
				  catman ? "-c " : "",
				  print_where ? "-w " : "",
				  print_where_cat ? "-W " : "",
				  _(": incompatible options"), NULL));
		usage (FAIL);
	}
}

/*
 * Check to see if the argument is a valid section number. 
 * If the name matches one of
 * the sections listed in section_list, we'll assume that it's a section.
 * The list of sections in config.h simply allows us to specify oddly
 * named directories like .../man3f.  Yuk.
 */
static __inline__ const char *is_section (const char *name)
{
	const char **vs;

	for (vs = section_list; *vs; vs++) {
		if (STREQ (*vs, name))
			return name;
		/* allow e.g. 3perl but disallow 8139too and libfoo */
		if (strlen (*vs) == 1 && CTYPE (isdigit, **vs) &&
		    strlen (name) > 1 && !CTYPE (isdigit, name[1]) &&
		    STRNEQ (*vs, name, 1))
			return name;
	}
	return NULL;
}

/* Snarf pre-processors from file, return (static) string or NULL on failure */
static const char *get_preprocessors_from_file (pipeline *decomp)
{
	static char *directive = NULL;

	if (directive) {
		free (directive);
		directive = NULL;
	}

#ifdef PP_COOKIE
	const char *line;

	if (!decomp)
		return NULL;

	line = pipeline_peekline (decomp);
	if (!line)
		return NULL;

	if (!strncmp (line, PP_COOKIE, 4)) {
		const char *newline = strchr (line, '\n');
		if (newline)
			directive = xstrndup (line + 4, newline - (line + 4));
		else
			directive = xstrdup (line + 4);
	}

	/* if we didn't find PP_COOKIE, then directive == NULL */
#endif
	return directive;
}


/* Determine pre-processors, set save_cat and return
   (static) string */
static const char *get_preprocessors (pipeline *decomp, const char *dbfilters)
{
	const char *pp_string;
	const char *pp_source;

	/* try in order: database, command line, file, environment, default */
	/* command line overrides the database, but database empty overrides default */
	if (dbfilters && (dbfilters[0] != '-') && !preprocessors) {
		pp_string = dbfilters;
		pp_source = "database";
		save_cat = 1;
	} else if ((pp_string = preprocessors)) {
		pp_source = "command line";
		save_cat = 0;
	} else if ((pp_string = get_preprocessors_from_file (decomp))) {
		pp_source = "file";
		save_cat = 1;
	} else if ((pp_string = getenv ("MANROFFSEQ"))) {
		pp_source = "environment";
		save_cat = 0;
	} else if (!dbfilters) {
		pp_string = DEFAULT_MANROFFSEQ;
		pp_source = "default";
		save_cat = 1;
	} else {
		pp_string = "";
		pp_source = "no filters";
		save_cat = 1;
	}

	debug ("pre-processors `%s' from %s\n", pp_string, pp_source);
	return pp_string;
}

/* Return pipeline to format file to stdout. */
static pipeline *make_roff_command (const char *dir, const char *file,
				    pipeline *decomp, const char *dbfilters)
{
	const char *pp_string;
	char *fmt_prog;
	pipeline *p = pipeline_new ();
	command *cmd;

#ifndef ALT_EXT_FORMAT
	dir = dir; /* not used unless looking for formatters in catdir */
#endif

	pp_string = get_preprocessors (decomp, dbfilters);

#ifdef ALT_EXT_FORMAT
	/* Check both external formatter locations */
	if (dir) {
		char *catpath = get_catpath
			(dir, global_manpath ? SYSTEM_CAT : USER_CAT);

		/* If we have an alternate catpath */
		if (catpath) {
			fmt_prog = strappend (catpath, "/",
					      troff ? TFMT_PROG : NFMT_PROG, 
					      NULL);
			if (access (fmt_prog, X_OK)) {
				free (fmt_prog);
				fmt_prog = xstrdup (troff ? TFMT_PROG :
							    NFMT_PROG);
				if (access (fmt_prog, X_OK)) {
					free (fmt_prog);
					fmt_prog = NULL;
				}
			}
		/* If we don't */
		} else {
#endif /* ALT_EXT_FORMAT */

			fmt_prog = xstrdup (troff ? TFMT_PROG : NFMT_PROG);
			if (access (fmt_prog, X_OK)) {
				free (fmt_prog);
				fmt_prog = NULL;
			}

#ifdef ALT_EXT_FORMAT
		}
	} else
		fmt_prog = NULL;
#endif /* ALT_EXT_FORMAT */
	
	if (fmt_prog)
		debug ("External formatter %s\n", fmt_prog);
				
	if (!fmt_prog) {
		/* we don't have an external formatter script */
		int using_tbl = 0;
		const char *output_encoding = NULL, *locale_charset = NULL;
		char *pp_encoding = NULL;

		pipeline_command_argstr (p, get_def ("soelim", SOELIM));

		if (strstr (pp_string, "-*-")) {
			const char *pp_search = strstr (pp_string, "-*-") + 3;
			while (*pp_search == ' ')
				++pp_search;
			if (STRNEQ (pp_search, "coding:", 7)) {
				const char *pp_encoding_end;
				pp_search += 7;
				while (*pp_search == ' ')
					++pp_search;
				pp_encoding_end = strchr (pp_search, ' ');
				if (pp_encoding_end)
					pp_encoding = xstrndup
						(pp_search,
						 pp_encoding_end - pp_search);
				else
					pp_encoding = xstrdup (pp_search);
				debug ("preprocessor encoding: %s\n",
				       pp_encoding);
			}
		}

		/* Load the roff_device value dependent on the language dir
		 * in the path.
		 */
		if (!troff) {
			char *page_encoding;
			const char *source_encoding, *roff_encoding;
			char *cat_charset;
			const char *groff_preconv;

#define STRC(s, otherwise) ((s) ? (s) : (otherwise))

			if (pp_encoding) {
				page_encoding = xstrdup (pp_encoding);
			} else
				page_encoding = get_page_encoding (lang);
			source_encoding = get_source_encoding (lang);
			debug ("page_encoding = %s\n", page_encoding);
			debug ("source_encoding = %s\n", source_encoding);

			cat_charset = get_standard_output_encoding (lang);
			locale_charset = get_locale_charset ();
			debug ("cat_charset = %s\n",
			       STRC (cat_charset, "NULL"));
			debug ("locale_charset = %s\n",
			       STRC (locale_charset, "NULL"));

			/* Only save cat pages for this manual hierarchy's
			 * default character set. If we don't know the cat
			 * charset, anything goes.
			 */
			if (cat_charset &&
			    (!locale_charset ||
			     !STREQ (cat_charset, locale_charset)))
				save_cat = 0;

			/* Pick the default device for this locale if there
			 * wasn't one selected explicitly.
			 */
			if (!roff_device) {
				roff_device =
					get_default_device (locale_charset,
							    source_encoding);
				debug ("roff_device (locale) = %s\n",
				       STRC (roff_device, "NULL"));
			}

			roff_encoding = get_roff_encoding (roff_device,
							   source_encoding);
			debug ("roff_encoding = %s\n", roff_encoding);

			/* We may need to recode:
			 *   from page_encoding to roff_encoding on input;
			 *   from output_encoding to locale_charset on output.
			 * If we have preconv, then use it to recode the
			 * input to a safe escaped form.
			 */
			groff_preconv = get_groff_preconv ();
			if (groff_preconv)
				pipeline_command_args
					(p, groff_preconv, "-e", page_encoding,
					 NULL);
			else if (roff_encoding &&
			    !STREQ (page_encoding, roff_encoding))
				pipeline_command_args (p, "iconv", "-c",
						       "-f", page_encoding,
						       "-t", roff_encoding,
						       NULL);

			output_encoding = get_output_encoding (roff_device);
			if (!output_encoding)
				output_encoding = source_encoding;
			debug ("output_encoding = %s\n", output_encoding);

			if (!getenv ("LESSCHARSET")) {
				const char *less_charset =
					get_less_charset (locale_charset);
				debug ("less_charset = %s\n", less_charset);
				putenv (strappend (NULL, "LESSCHARSET=",
						   less_charset, NULL));
			}

			free (page_encoding);
			free (cat_charset);
		}

		do {
			int wants_dev = 0; /* filter wants a dev argument */
			int wants_post = 0; /* postprocessor arguments */

			cmd = NULL;
			/* set cmd according to *pp_string, on
                           errors leave cmd as NULL */
			switch (*pp_string) {
			case 'e':
				if (troff)
					cmd = command_new_argstr
						(get_def ("eqn", EQN));
				else
					cmd = command_new_argstr
						(get_def ("neqn", NEQN));
				wants_dev = 1;
				break;
			case 'g':
				cmd = command_new_argstr
					(get_def ("grap", GRAP));
				break;
			case 'p':
				cmd = command_new_argstr
					(get_def ("pic", PIC));
				break;
			case 't':
				cmd = command_new_argstr
					(get_def ("tbl", TBL));
				using_tbl = 1;
				break;
			case 'v':
				cmd = command_new_argstr
					(get_def ("vgrind", VGRIND));
				break;
			case 'r':
				cmd = command_new_argstr
					(get_def ("refer", REFER));
				break;
			case ' ':
			case '-':
			case 0:
				/* done with preprocessors, now add roff */
                                if (troff)
					cmd = command_new_argstr
						(get_def ("troff", TROFF));
                                else
					cmd = command_new_argstr
						(get_def ("nroff", NROFF));

#ifdef TROFF_IS_GROFF
				if (troff && ditroff)
					command_arg (cmd, "-Z");
				else if (!troff)
					add_roff_line_length (cmd, &save_cat);
#endif

				wants_dev = 1;
				wants_post = 1;
				break;
			}

			if (!cmd) {
				assert (*pp_string); /* didn't fail on roff */
				error (0, 0,
				       _("ignoring unknown preprocessor `%c'"),
				       *pp_string);
				continue;
			}

			if (wants_dev) {
				if (roff_device) {
					char *tmpdev = strappend (NULL, "-T",
								  roff_device,
								  NULL);
					command_arg (cmd, tmpdev);
					free (tmpdev);
				} else if (gxditview) {
					char *tmpdev = strappend (NULL, "-TX",
								  gxditview,
								  NULL);
					command_arg (cmd, tmpdev);
					free (tmpdev);
				}
			}

			if (wants_post) {
				if (gxditview)
					command_arg (cmd, "-X");

				if (roff_device && STREQ (roff_device, "ps"))
					/* Tell grops to guess the page
					 * size.
					 */
					command_arg (cmd, "-P-g");
			}

			pipeline_command (p, cmd);

			if (*pp_string == ' ' || *pp_string == '-')
				break;
		} while (*pp_string++);

		if (!different_encoding && output_encoding && locale_charset &&
		    !STREQ (output_encoding, locale_charset))
			pipeline_command_args (p, "iconv", "-c",
					       "-f", output_encoding,
					       "-t", locale_charset, NULL);

		if (!troff && *COL) {
			/* get rid of special characters if not writing to a
			 * terminal
			 */
			const char *man_keep_formatting =
				getenv ("MAN_KEEP_FORMATTING");
			if ((!man_keep_formatting || !*man_keep_formatting) &&
			    !isatty (fileno (stdout))) {
				save_cat = 0;
				setenv ("GROFF_NO_SGR", "1", 1);
				pipeline_command_args (p, COL,
						       "-b", "-p", "-x", NULL);
			}
#ifndef GNU_NROFF
			/* tbl needs col */
			else if (using_tbl && !troff && *COL)
				pipeline_command_args (p, COL, NULL);
#endif /* GNU_NROFF */
		}

		free (pp_encoding);
	} else {
		/* use external formatter script, it takes arguments
		   input file, preprocessor string, and (optional)
		   output device */
		cmd = command_new_args (fmt_prog, file, pp_string, NULL);
		if (roff_device)
			command_arg (cmd, roff_device);
		pipeline_command (p, cmd);
	}

	return p;
}

/* Return pipeline to run a browser on a given file, observing
 * http://www.tuxedo.org/~esr/BROWSER/.
 *
 * (Actually, I really implement
 * http://www.dwheeler.com/browse/secure_browser.html, but it's
 * backward-compatible.)
 *
 * TODO: Is there any way to use the pipeline library better here?
 */
static pipeline *make_browser (const char *pattern, const char *file)
{
	pipeline *p;
	char *browser = xmalloc (1);
	int found_percent_s = 0;
	char *percent;
	char *esc_file;

	*browser = '\0';

	percent = strchr (pattern, '%');
	while (percent) {
		size_t len = strlen (browser);
		browser = xrealloc (browser, len + 1 + (percent - pattern));
		strncat (browser, pattern, percent - pattern);
		switch (*(percent + 1)) {
			case '\0':
			case '%':
				browser = strappend (browser, "%", NULL);
				break;
			case 'c':
				browser = strappend (browser, ":", NULL);
				break;
			case 's':
				esc_file = escape_shell (file);
				browser = strappend (browser, esc_file, NULL);
				free (esc_file);
				found_percent_s = 1;
				break;
			default:
				len = strlen (browser); /* cannot be NULL */
				browser = xrealloc (browser, len + 3);
				strncat (browser, percent, 2);
				break;
		}
		if (*(percent + 1))
			pattern = percent + 2;
		else
			pattern = percent + 1;
		percent = strchr (pattern, '%');
	}
	browser = strappend (browser, pattern, NULL);
	if (!found_percent_s) {
		esc_file = escape_shell (file);
		browser = strappend (browser, " ", esc_file, NULL);
		free (esc_file);
	}

	p = pipeline_new ();
	pipeline_command_args (p, "/bin/sh", "-c", browser, NULL);
	free (browser);

	return p;
}

static void setenv_less (const char *title)
{
	const char *esc_title;
	char *less_opts, *man_pn;
	const char *force = getenv ("MANLESS");

	if (force) {
		setenv ("LESS", force, 1);
		return;
	}

	esc_title = escape_less (title);
	less_opts = xmalloc (strlen (LESS_OPTS) +
			     strlen (prompt_string) * 2 + 1);
	sprintf (less_opts, LESS_OPTS, prompt_string, prompt_string);
	less_opts = strappend (less_opts, less, NULL);
	man_pn = strstr (less_opts, MAN_PN);
	while (man_pn) {
		char *subst_opts =
			xmalloc (strlen (less_opts) - strlen (MAN_PN) +
				 strlen (esc_title) + 1);
		strncpy (subst_opts, less_opts, man_pn - less_opts);
		subst_opts[man_pn - less_opts] = '\0';
		strcat (subst_opts, esc_title);
		strcat (subst_opts, man_pn + strlen (MAN_PN));
		free (less_opts);
		less_opts = subst_opts;
		man_pn = strstr (less_opts, MAN_PN);
	}

	debug ("Setting LESS to %s\n", less_opts);
	/* If there isn't enough space in the environment, ignore it. */
	setenv ("LESS", less_opts, 1);

	debug ("Setting MAN_PN to %s\n", esc_title);
	setenv ("MAN_PN", esc_title, 1);

	free (less_opts);
}

/* Return pipeline to display file. NULL means stdin.
 *
 * TODO: htmlout case is pretty weird now. I'd like the intelligence to be
 * somewhere other than format_display.
 */
static pipeline *make_display_command (const char *file, const char *title)
{
	pipeline *p;
	command *cmd;

	setenv_less (title);

	if (file) {
		if (ascii) {
			p = pipeline_new ();
			cmd = command_new_argstr (get_def_user ("cat", CAT));
			command_arg (cmd, file);
			pipeline_command (p, cmd);
			pipeline_command_argstr
				(p, get_def_user ("tr", TR TR_SET1 TR_SET2));
			pipeline_command_argstr (p, pager);
		}
#ifdef TROFF_IS_GROFF
		else if (htmlout)
			/* format_display deals with html_pager */
			p = NULL;
#endif
		else {
			p = pipeline_new ();
			cmd = command_new_argstr (pager);
			command_arg (cmd, file);
			pipeline_command (p, cmd);
		}
	} else {
		if (ascii) {
			p = pipeline_new ();
			pipeline_command_argstr
				(p, get_def_user ("tr", TR TR_SET1 TR_SET2));
			pipeline_command_argstr (p, pager);
		}
#ifdef TROFF_IS_GROFF
		else if (htmlout)
			/* format_display deals with html_pager */
			p = NULL;
#endif
		else {
			p = pipeline_new ();
			pipeline_command_argstr (p, pager);
		}
	}

	return p;
}


/* return a (malloced) temporary name in cat_file's directory */
static char *tmp_cat_filename (const char *cat_file)
{
	char *name;

	if (debug_level) {
		name = xstrdup ("/dev/null");
		tmp_cat_fd = open (name, O_WRONLY);
	} else {
		char *slash;
		name = xstrdup (cat_file);
		slash = strrchr (name, '/');
		if (slash)
			*(slash + 1) = '\0';
		else
			*name = '\0';
		name = strappend (name, "catXXXXXX", NULL);
		tmp_cat_fd = mkstemp (name);
	}

	if (tmp_cat_fd == -1) {
		free (name);
		return NULL;
	} else
		return name;
}


/* If delete unlink tmp_cat, else commit tmp_cat to cat_file.
   Return non-zero on error.
 */
static int commit_tmp_cat (const char *cat_file, const char *tmp_cat,
			   int delete)
{
	int status = 0;

#ifdef SECURE_MAN_UID
	if (!delete && global_manpath && euid == 0) {
		if (debug_level) {
			debug ("fixing temporary cat's ownership\n");
			status = 0;
		} else {
			struct passwd *man_owner = get_man_owner ();
			status = chown (tmp_cat, man_owner->pw_uid, -1);
			if (status)
				error (0, errno, _("can't chown %s"), tmp_cat);
		}
	}
#endif /* SECURE_MAN_UID */

	if (!delete && !status) {
		if (debug_level) {
			debug ("fixing temporary cat's mode\n");
			status = 0;
		} else {
			status = chmod (tmp_cat, CATMODE);
			if (status)
				error (0, errno, _("can't chmod %s"), tmp_cat);
		}
	}

	if (!delete && !status) {
		if (debug_level) {
			debug ("renaming temporary cat to %s\n", cat_file);
			status = 0;
		} else {
			status = rename (tmp_cat, cat_file);
			if (status)
				error (0, errno, _("can't rename %s to %s"),
				       tmp_cat, cat_file);
		}
	}

	if (!delete && !status) {
		if (debug_level) {
			debug ("setting modtime on cat file %s\n", cat_file);
			status = 0;
		} else {
			time_t now = time (NULL);
			struct utimbuf utb;
			utb.actime = now;
			if (man_modtime)
				utb.modtime = man_modtime;
			else
				utb.modtime = 0;
			status = utime (cat_file, &utb);
			if (status)
				error (0, errno, _("can't set times on %s"),
				       cat_file);
		}
	}

	if (delete || status) {
		if (debug_level)
			debug ("unlinking temporary cat\n");
		else if (unlink (tmp_cat))
			error (0, errno, _("can't unlink %s"), tmp_cat);
	}

	return status;
}

#ifdef MAN_CATS

/* Return pipeline to write formatted manual page to for saving as cat file. */
static pipeline *open_cat_stream (const char *cat_file)
{
	pipeline *cat_p;
#  ifdef COMP_CAT
	command *comp_cmd;
#  endif

	created_tmp_cat = 0;

	debug ("creating temporary cat for %s\n", cat_file);

	tmp_cat_file = tmp_cat_filename (cat_file);
	if (tmp_cat_file)
		created_tmp_cat = 1;
	else {
		if (!debug_level && (errno == EACCES || errno == EROFS)) {
			/* No permission to write to the cat file. Oh well,
			 * return NULL and let the caller sort it out.
			 */
			debug ("can't write to temporary cat for %s\n",
			       cat_file);
			return NULL;
		} else
			error (FATAL, errno,
			       _("can't create temporary cat for %s"),
			       cat_file);
	}

	if (!debug_level)
		push_cleanup ((cleanup_fun) unlink, tmp_cat_file, 1);

#  ifdef COMP_CAT
	/* write to a pipe that compresses into tmp_cat_file */

	/* fork the compressor */
	cat_p = pipeline_new ();
	comp_cmd = command_new_argstr (get_def ("compressor", COMPRESSOR));
	comp_cmd->nice = 10;
	pipeline_command (cat_p, comp_cmd);
#  else
	/* write directly to tmp_cat_file */

	/* fake up a pipeline structure */
	cat_p = pipeline_new ();
#  endif
	cat_p->want_out = tmp_cat_fd; /* pipeline_start will close it */

	return cat_p;
}

/* Close the cat page stream, return non-zero on error.
   If delete don't update the cat file.
 */
static int close_cat_stream (pipeline *cat_p, const char *cat_file,
			     int delete)
{
	int status;

	status = pipeline_wait (cat_p);
	debug ("cat-saver exited with status %d\n", status);

	pipeline_free (cat_p);

	if (created_tmp_cat) {
		status |= commit_tmp_cat (cat_file, tmp_cat_file,
					  delete || status);
		if (!debug_level)
			pop_cleanup ();
	}
	free (tmp_cat_file);
	return status;
}

/* TODO: This should all be refactored after work on the decompression
 * library is complete.
 */
void discard_stderr (pipeline *p)
{
	int i;

	for (i = 0; i < p->ncommands; ++i)
		p->commands[i]->discard_err = 1;
}

/*
 * format a manual page with format_cmd, display it with disp_cmd, and
 * save it to cat_file
 */
static int format_display_and_save (pipeline *decomp,
				    pipeline *format_cmd,
				    pipeline *disp_cmd,
				    const char *cat_file)
{
	pipeline *sav_p = open_cat_stream (cat_file);
	int instat;
	RETSIGTYPE (*old_handler)(int) = signal (SIGPIPE, SIG_IGN);

	if (global_manpath)
		drop_effective_privs ();

	discard_stderr (format_cmd);

	pipeline_connect (decomp, format_cmd, NULL);
	if (sav_p) {
		pipeline_connect (format_cmd, disp_cmd, sav_p, NULL);
		pipeline_pump (decomp, format_cmd, disp_cmd, sav_p, NULL);
	} else {
		pipeline_connect (format_cmd, disp_cmd, NULL);
		pipeline_pump (decomp, format_cmd, disp_cmd, NULL);
	}

	if (global_manpath)
		regain_effective_privs ();

	pipeline_wait (decomp);
	instat = pipeline_wait (format_cmd);
	if (sav_p)
		close_cat_stream (sav_p, cat_file, instat);
	pipeline_wait (disp_cmd);
	signal (SIGPIPE, old_handler);
	return instat;
}
#endif /* MAN_CATS */

/* Format a manual page with format_cmd and display it with disp_cmd.
 * Handle temporary file creation if necessary.
 * TODO: merge with format_display_and_save
 */
static void format_display (pipeline *decomp,
			    pipeline *format_cmd, pipeline *disp_cmd,
			    const char *man_file)
{
	int status;
	char *old_cwd = NULL;
	char *htmldir = NULL, *htmlfile = NULL;

	if (format_cmd)
		discard_stderr (format_cmd);

	drop_effective_privs ();

#ifdef TROFF_IS_GROFF
	if (format_cmd && htmlout) {
		char *man_file_copy, *man_base, *man_ext;

		old_cwd = getcwd_allocated ();
		if (!old_cwd) {
			old_cwd = xmalloc (1);
			old_cwd[0] = '\0';
		}
		htmldir = create_tempdir ("hman");
		if (chdir (htmldir) == -1)
			error (FATAL, errno, _("can't change to directory %s"),
			       htmldir);
		man_file_copy = xstrdup (man_file);
		man_base = basename (man_file_copy);
		man_ext = strchr (man_base, '.');
		if (man_ext)
			*man_ext = '\0';
		htmlfile = xstrdup (htmldir);
		htmlfile = strappend (htmlfile, "/", man_base, ".html", NULL);
		free (man_file_copy);
		format_cmd->want_out = open (htmlfile,
					     O_CREAT | O_EXCL | O_WRONLY,
					     0644);
		if (format_cmd->want_out == -1)
			error (FATAL, errno, _("can't open temporary file %s"),
			       htmlfile);
		pipeline_connect (decomp, format_cmd, NULL);
		pipeline_pump (decomp, format_cmd, NULL);
		pipeline_wait (decomp);
		status = pipeline_wait (format_cmd);
	} else
#endif /* TROFF_IS_GROFF */
	    if (format_cmd) {
		pipeline_connect (decomp, format_cmd, NULL);
		pipeline_connect (format_cmd, disp_cmd, NULL);
		pipeline_pump (decomp, format_cmd, disp_cmd, NULL);
		pipeline_wait (decomp);
		pipeline_wait (format_cmd);
		status = pipeline_wait (disp_cmd);
	} else {
		pipeline_connect (decomp, disp_cmd, NULL);
		pipeline_pump (decomp, disp_cmd, NULL);
		pipeline_wait (decomp);
		status = pipeline_wait (disp_cmd);
	}

#ifdef TROFF_IS_GROFF
	if (format_cmd && htmlout) {
		char *browser_list, *candidate;

		if (status) {
			if (chdir (old_cwd) == -1) {
				error (0, errno,
				       _("can't change to directory %s"),
				       old_cwd);
				chdir ("/");
			}
			if (remove_directory (htmldir) == -1)
				error (0, errno,
				       _("can't remove directory %s"),
				       htmldir);
			free (htmlfile);
			free (htmldir);
			gripe_system (format_cmd, status);
		}

		browser_list = xstrdup (html_pager);
		for (candidate = strtok (browser_list, ":"); candidate;
		     candidate = strtok (NULL, ":")) {
			pipeline *browser;
			debug ("Trying browser: %s\n", candidate);
			browser = make_browser (candidate, htmlfile);
			status = do_system_drop_privs (browser);
			pipeline_free (browser);
			if (!status)
				break;
		}
		if (!candidate)
			error (CHILD_FAIL, 0,
			       "couldn't execute any browser from %s",
			       html_pager);
		free (browser_list);
		if (chdir (old_cwd) == -1) {
			error (0, errno, _("can't change to directory %s"),
			       old_cwd);
			chdir ("/");
		}
		if (remove_directory (htmldir) == -1)
			error (0, errno, _("can't remove directory %s"),
			       htmldir);
		free (htmlfile);
		free (htmldir);
	} else
#endif /* TROFF_IS_GROFF */
	/* TODO: check format_cmd status too? */
	    if (status && status != (SIGPIPE + 0x80) * 256)
		gripe_system (disp_cmd, status);

	regain_effective_privs ();
}

/* "Display" a page in catman mode, which amounts to saving it. */
/* TODO: merge with format_display_and_save? */
static void display_catman (const char *cat_file, pipeline *decomp,
			    pipeline *format_cmd)
{
	char *tmpcat = tmp_cat_filename (cat_file);
	int status;

#ifdef COMP_CAT
	pipeline_command_argstr (format_cmd,
				 get_def ("compressor", COMPRESSOR));
#endif /* COMP_CAT */

	discard_stderr (format_cmd);
	format_cmd->want_out = tmp_cat_fd;

	push_cleanup ((cleanup_fun) unlink, tmpcat, 1);

	/* save the cat as real user
	 * (1) required for user man hierarchy
	 * (2) else depending on ruid's privs is ok, effectively disables
	 *     catman for non-root.
	 */
	drop_effective_privs ();
	pipeline_connect (decomp, format_cmd, NULL);
	pipeline_pump (decomp, format_cmd, NULL);
	pipeline_wait (decomp);
	status = pipeline_wait (format_cmd);
	regain_effective_privs ();
	if (status)
		gripe_system (format_cmd, status);

	close (tmp_cat_fd);
	commit_tmp_cat (cat_file, tmpcat, status);
	pop_cleanup();
	free (tmpcat);
}

/*
 * optionally chdir to dir, if necessary update cat_file from man_file
 * and display it.  if man_file is NULL cat_file is a stray cat.  If
 * !save_cat or cat_file is NULL we must not save the formatted cat.
 * If man_file is "" this is a special case -- we expect the man page
 * on standard input.
 */
static int display (const char *dir, const char *man_file,
		    const char *cat_file, const char *title,
		    const char *dbfilters)
{
	int found;
	static int prompt;
	pipeline *format_cmd;	/* command to format man_file to stdout */
	int display_to_stdout;
	pipeline *decomp = NULL;

	/* if dir is set chdir to it */
	if (dir) {
		debug ("chdir %s\n", dir);

		if (chdir (dir)) {
			error (0, errno, _("can't chdir to %s"), dir);
			return 0;
		}
	}

	/* define format_cmd */
	if (man_file) {
		if (*man_file)
			decomp = decompress_open (man_file);
		else
			decomp = decompress_fdopen (dup (fileno (stdin)));
	}

	if (decomp) {
		pipeline_start (decomp);
		format_cmd = make_roff_command (dir, man_file, decomp,
						dbfilters);
	} else
		format_cmd = NULL;

	/* Get modification time, for commit_tmp_cat(). */
	if (man_file && *man_file) {
		struct stat stb;
		if (stat (man_file, &stb))
			man_modtime = 0;
		else
			man_modtime = stb.st_mtime;
	}

	display_to_stdout = troff;
#ifdef TROFF_IS_GROFF
	if (htmlout)
		display_to_stdout = 0;
#endif

	if (display_to_stdout) {
		/* If we're reading stdin via '-l -', man_file is "". See
		 * below.
		 */
		if (*man_file == '\0')
			found = 1;
		else
			found = !access (man_file, R_OK);
		if (found) {
			if (prompt && do_prompt (title)) {
				pipeline_free (decomp);
				return 0;
			}
			checked_system (format_cmd);
		}
	} else {
		int format = 1;
		int status;

		/* The caller should already have checked for any
		 * FSSTND-style (same hierarchy) cat page that may be
		 * present, and we don't expect to have to update the cat
		 * page in that case. If by some chance we do have to update
		 * it, then there's no harm trying; open_cat_stream() will
		 * refuse gracefully if the file isn't writeable.
		 */

		if (different_encoding
#ifdef TROFF_IS_GROFF
		    || htmlout
#endif
		    || local_man_file
		    || disable_cache)
			save_cat = 0;

		if (!man_file) {
			/* Stray cat. */
			assert (cat_file);
			format = 0;
		} else if (!cat_file) {
			assert (man_file);
			save_cat = 0;
			format = 1;
		} else if (format && save_cat) {
			char *cat_dir;
			char *tmp;

			status = is_changed (man_file, cat_file);
			format = (status == -2) || ((status & 1) == 1);

			/* don't save if we haven't a cat directory */
			cat_dir = xstrdup (cat_file);
			tmp = strrchr (cat_dir, '/');
			if (tmp)
				*tmp = 0;
			save_cat = is_directory (cat_dir) == 1;
			if (!save_cat)
				debug ("cat dir %s does not exist\n", cat_dir);
			free (cat_dir);
		}

		/* if we're trying to read stdin via '-l -' then man_file
		 * will be "" which access() obviously barfs on, but all is
		 * well because the format_cmd will have been created to
		 * expect input via stdin. So we special-case this to avoid
		 * the bogus access() check.
		*/
		if (format == 1 && *man_file == '\0')
			found = 1;
		else
			found = !access (format ? man_file : cat_file, R_OK);

		debug ("format: %d, save_cat: %d, found: %d\n",
		       format, save_cat, found);

		if (!found) {
			pipeline_free (format_cmd);
			pipeline_free (decomp);
			return found;
		}

		if (print_where || print_where_cat) {
			int printed = 0;
			if (print_where && man_file) {
				printf ("%s", man_file);
				printed = 1;
			}
			if (print_where_cat && cat_file && !format) {
				if (printed)
					putchar (' ');
				printf ("%s", cat_file);
			}
			putchar ('\n');
		} else if (catman) {
			if (format) {
				if (!save_cat)
					error (0, 0,
					       _("\ncannot write to "
						 "%s in catman mode"),
					       cat_file);
				else
					display_catman (cat_file, decomp,
							format_cmd);
			}
		} else if (format) {
			/* no cat or out of date */
			pipeline *disp_cmd;

			if (prompt && do_prompt (title)) {
				pipeline_free (format_cmd);
				pipeline_free (decomp);
				if (local_man_file)
					return 1;
				else
					return 0;
			}

			disp_cmd = make_display_command (NULL, title);

#ifdef MAN_CATS
			if (save_cat) {
				/* save cat */
				assert (disp_cmd); /* not htmlout for now */
				format_display_and_save (decomp,
							 format_cmd,
							 disp_cmd,
							 cat_file);
			} else 
#endif /* MAN_CATS */
				/* don't save cat */
				format_display (decomp, format_cmd, disp_cmd,
						man_file);

			pipeline_free (disp_cmd);

		} else {
			/* display preformatted cat */
			pipeline *disp_cmd;
			pipeline *decomp_cat;

			if (prompt && do_prompt (title)) {
				pipeline_free (decomp);
				return 0;
			}

			decomp_cat = decompress_open (cat_file);
			disp_cmd = make_display_command (NULL, title);
			format_display (decomp_cat, NULL, disp_cmd, man_file);
			pipeline_free (disp_cmd);
			pipeline_free (decomp_cat);
		}
	}

	pipeline_free (format_cmd);
	pipeline_free (decomp);

	if (!prompt)
		prompt = found;

	return found;
}


static char *find_cat_file (const char *path, const char *original,
			    const char *man_file)
{
	size_t path_len = strlen (path);
	char *cat_file, *cat_path;
	int status;

	/* Try the FSSTND way first, namely a cat page in the same hierarchy
	 * as the original path to the man page. We don't create these
	 * unless no alternate cat hierarchy is available, but will use them
	 * if they happen to exist already and have the same timestamp as
	 * the corresponding man page. (In practice I'm betting that this
	 * means we'll hardly ever use them at all except for user
	 * hierarchies; but compatibility, eh?)
	 */
	cat_file = convert_name (original, 1);
	if (cat_file) {
		status = is_changed (original, cat_file);
		if (status != -2 && !(status & 1) == 1) {
			debug ("found valid FSSTND cat file %s\n", cat_file);
			return cat_file;
		}
		free (cat_file);
	}

	/* Otherwise, find the cat page we actually want to use or create,
	 * taking any alternate cat hierarchy into account. If the original
	 * path and man_file differ (i.e. original was a symlink or .so
	 * link), try the link target and then the source.
	 */
	if (!STREQ (man_file, original)) {
		global_manpath = is_global_mandir (man_file);
		cat_path = get_catpath
			(man_file, global_manpath ? SYSTEM_CAT : USER_CAT);

		if (cat_path) {
			cat_file = convert_name (cat_path, 0);
			free (cat_path);
		} else if (STRNEQ (man_file, path, path_len) &&
			   man_file[path_len] == '/')
			cat_file = convert_name (man_file, 1);
		else
			cat_file = NULL;

		if (cat_file) {
			char *cat_dir = xstrdup (cat_file);
			char *tmp = strrchr (cat_dir, '/');
			if (tmp)
				*tmp = 0;
			if (is_directory (cat_dir)) {
				debug ("will try cat file %s\n", cat_file);
				return cat_file;
			} else
				debug ("cat dir %s does not exist\n", cat_dir);
			free (cat_dir);
		} else
			debug ("no cat path for %s\n", man_file);
	}

	global_manpath = is_global_mandir (original);
	cat_path = get_catpath
		(original, global_manpath ? SYSTEM_CAT : USER_CAT);

	if (cat_path) {
		cat_file = convert_name (cat_path, 0);
		free (cat_path);
	} else
		cat_file = convert_name (original, 1);

	if (cat_file)
		debug ("will try cat file %s\n", cat_file);
	else
		debug ("no cat path for %s\n", original);

	return cat_file;
}

/* Is this candidate substantially a duplicate of a previous one?
 * Returns 0 if it is distinct, 1 if source/path is no better than search,
 * and 2 if source/path is better than search.
 */
static int duplicate_candidates (struct mandata *source, const char *path,
				 struct candidate *search)
{
	const char *slash1, *slash2;
	char *locale_copy, *p;
	struct locale_bits bits1, bits2, lbits;
	const char *codeset1, *codeset2;
	int ret;

	if (!STREQ (source->name, search->source->name) ||
	    !STREQ (source->sec, search->source->sec) ||
	    !STREQ (source->ext, search->source->ext))
		return 0; /* different name/section/extension */

	if (STREQ (path, search->path)) {
		debug ("duplicate candidate\n");
		return 1;
	}

	/* Figure out if we've had a sufficiently similar candidate for this
	 * language already.
	 */
#define MAX(a,b) ((a) > (b) ? (a) : (b))
	slash1 = strrchr (path, '/');
	slash2 = strrchr (search->path, '/');
	if (!slash1 || !slash2 ||
	    !STRNEQ (path, search->path,
		     MAX (slash1 - path, slash2 - search->path)))
		return 0; /* different path base */
#undef MAX

	unpack_locale_bits (++slash1, &bits1);
	unpack_locale_bits (++slash2, &bits2);

	if (!STREQ (bits1.language, bits2.language)) {
		ret = 0; /* different language */
		goto out;
	}

	/* From here on in we need the current locale as well. */
	locale_copy = xstrdup (internal_locale);
	p = strchr (locale_copy, ':');
	if (p)
		*p = '\0';
	unpack_locale_bits (locale_copy, &lbits);
	free (locale_copy);

	/* For different territories, prefer one that matches the locale if
	 * possible.
	 */
	if (*lbits.territory) {
		if (STREQ (lbits.territory, bits1.territory)) {
			if (!STREQ (lbits.territory, bits2.territory)) {
				ret = 2;
				goto out_locale;
			}
		} else {
			if (STREQ (lbits.territory, bits2.territory)) {
				ret = 1;
				goto out_locale;
			}
		}
	}
	if (!STREQ (bits1.territory, bits2.territory)) {
		ret = 0; /* different territories, no help from locale */
		goto out_locale;
	}

	/* For different modifiers, prefer one that matches the locale if
	 * possible.
	 */
	if (*lbits.modifier) {
		if (STREQ (lbits.modifier, bits1.modifier)) {
			if (!STREQ (lbits.modifier, bits2.modifier)) {
				ret = 2;
				goto out_locale;
			}
		} else {
			if (STREQ (lbits.modifier, bits2.modifier)) {
				ret = 1;
				goto out_locale;
			}
		}
	}
	if (!STREQ (bits1.modifier, bits2.modifier)) {
		ret = 0; /* different modifiers, no help from locale */
		goto out_locale;
	}

	/* Prefer UTF-8 if available. Otherwise, the last one is probably
	 * just as good as this one.
	 */
	codeset1 = get_canonical_charset_name (bits1.codeset);
	codeset2 = get_canonical_charset_name (bits2.codeset);
	if (STREQ (codeset1, "UTF-8")) {
		if (!STREQ (codeset2, "UTF-8")) {
			ret = 2;
			goto out_locale;
		}
	} else {
		/* Either codeset2 is UTF-8 or it's some other legacy
		 * encoding; either way, we prefer it.
		 */
		ret = 1;
		goto out_locale;
	}

	/* Everything seems to be the same; we can find nothing to choose
	 * between them. Prefer the one that got there first.
	 */
	ret = 1;

out_locale:
	free_locale_bits (&lbits);
out:
	free_locale_bits (&bits1);
	free_locale_bits (&bits2);
	return ret;
}

static int compare_candidates (const struct mandata *left,
			       const struct mandata *right,
			       const char *req_name)
{
	int sec_left = 0, sec_right = 0;
	int cmp;

	/* If one candidate matches the requested name exactly, sort it
	 * first. This makes --ignore-case behave more sensibly.
	 */
	/* name is never NULL here, see add_candidate() */
	if (STREQ (left->name, req_name)) {
		if (!STREQ (right->name, req_name))
			return -1;
	} else {
		if (STREQ (right->name, req_name))
			return 1;
	}

	/* Compare pure sections first, then ids, then extensions.
	 * Rationale: whatis refs get the same section and extension as
	 * their source, but may be supplanted by a real page with a
	 * slightly different extension, possibly in another hierarchy (!);
	 * see Debian bug #204249 for the gory details.
	 *
	 * Any extension spelt out in full in section_list effectively
	 * becomes a pure section; this allows extensions to be selectively
	 * moved out of order with respect to their parent sections.
	 */
	if (strcmp (left->ext, right->ext)) {
		/* Find out whether left->ext is ahead of right->ext in
		 * section_list.
		 */
		const char **sp;
		for (sp = section_list; *sp; ++sp) {
			if (!*(*sp + 1)) {
				/* No extension */
				if (!sec_left  && **sp == *(left->ext))
					sec_left  = sp - section_list + 1;
				if (!sec_right && **sp == *(right->ext))
					sec_right = sp - section_list + 1;
			} else if (STREQ (*sp, left->ext)) {
				sec_left  = sp - section_list + 1;
			} else if (STREQ (*sp, right->ext)) {
				sec_right = sp - section_list + 1;
			}
			/* Keep looking for a more specific match */
		}
		if (sec_left != sec_right)
			return sec_left - sec_right;

		cmp = strcmp (left->sec, right->sec);
		if (cmp)
			return cmp;
	}

	/* ULT_MAN comes first, etc. */
	cmp = compare_ids (left->id, right->id);
	if (cmp)
		return cmp;

	/* The order in section_list has already been compared above. For
	 * everything not mentioned explicitly there, we just compare
	 * lexically.
	 */
	cmp = strcmp (left->ext, right->ext);
	if (cmp)
		return cmp;

	/* add_candidate() will keep equal candidates in order of insertion
	 * so that manpath ordering (e.g. language-specific hierarchies)
	 * works.
	 */
	return 0;
}

/* Add an entry to the list of candidates. */
static int add_candidate (struct candidate **head, char from_db, char cat,
			  const char *req_name, const char *path,
			  struct mandata *source)
{
	struct candidate *search, *prev, *insert, *candp;
	int insert_found = 0;

	debug ("candidate: %d %d %s %s %c %s %s %s\n",
	       from_db, cat, req_name, path,
	       source->id, source->name ? source->name : "-",
	       source->sec, source->ext);

	if (!source->name)
		source->name = xstrdup (req_name);

	/* insert will be NULL (insert at start) or a pointer to the element
	 * after which this element should be inserted.
	 */
	insert = NULL;
	search = *head;
	prev = NULL;
	while (search) {
		/* Check for duplicates. */
		int dupcand = duplicate_candidates (source, path, search);
		if (dupcand == 1) {
			debug ("duplicate candidate\n");
			return 0;
		} else if (dupcand == 2) {
			debug ("superior duplicate candidate\n");
			if (prev) {
				prev->next = search->next;
				free (search);
				search = prev->next;
			} else {
				*head = search->next;
				free (search);
				search = *head;
			}
			continue;
		}
		if (!insert_found &&
		    compare_candidates (source, search->source,
					req_name) < 0) {
			insert = prev;
			insert_found = 1;
		}

		prev = search;
		if (search->next)
			search = search->next;
		else
			break;
	}
	if (!insert_found)
		insert = prev;

	candp = (struct candidate *) malloc (sizeof (struct candidate));
	candp->req_name = req_name;
	candp->from_db = from_db;
	candp->cat = cat;
	candp->path = path;
	candp->source = source;
	candp->next = insert ? insert->next : *head;
	if (insert)
		insert->next = candp;
	else
		*head = candp;

	return 1;
}

/*
 * See if the preformatted man page or the source exists in the given
 * section.
 */
static int try_section (const char *path, const char *sec, const char *name,
			struct candidate **cand_head)
{
	int found = 0;
	char **names = NULL, **np;
	char cat = 0;

	debug ("trying section %s with globbing\n", sec);

#ifndef NROFF_MISSING /* #ifdef NROFF */
	/*
  	 * Look for man page source files.
  	 */

	names = look_for_file (path, sec, name, 0, match_case);
	if (!names)
		/*
    		 * No files match.  
    		 * See if there's a preformatted page around that
    		 * we can display.
    		 */
#endif /* NROFF_MISSING */
	{
		if (catman)
			return 1;

		if (!troff && !different_encoding) {
			names = look_for_file (path, sec, name, 1, match_case);
			cat = 1;
		}
	}

	for (np = names; np && *np; np++) {
		struct mandata *info = infoalloc ();
		char *info_buffer = filename_info (*np, info, name);
		const char *ult;
		if (!info_buffer)
			continue;
		info->addr = info_buffer;

		/* What kind of page is this? Since it's a real file, it
		 * must be either ULT_MAN or SO_MAN. ult_src() can tell us
		 * which.
		 */
		ult = ult_src (*np, path, NULL,
			       SO_LINK | SOFT_LINK | HARD_LINK);
		if (!ult) {
			/* already warned */
			debug ("try_section(): bad link %s\n", *np);
			continue;
		}
		if (STREQ (ult, *np))
			info->id = ULT_MAN;
		else
			info->id = SO_MAN;

		found += add_candidate (cand_head, CANDIDATE_FILESYSTEM,
					cat, name, path, info);
		/* Don't free info and info_buffer here. */
	}

	return found;
}

static int display_filesystem (struct candidate *candp)
{
	char *filename = make_filename (candp->path, NULL, candp->source,
					candp->cat ? "cat" : "man");
	/* source->name is never NULL thanks to add_candidate() */
	char *title = strappend (NULL, candp->source->name,
				 "(", candp->source->ext, ")", NULL);
	if (candp->cat) {
		if (troff || different_encoding)
			return 0;
		return display (candp->path, NULL, filename, title, NULL);
	} else {
		const char *man_file;
		char *cat_file;
		int found;

		man_file = ult_src (filename, candp->path, NULL,
				    SO_LINK | SOFT_LINK | HARD_LINK);
		if (man_file == NULL) {
			free (title);
			return 0;
		}

		debug ("found ultimate source file %s\n", man_file);
		lang = lang_dir (man_file);

		cat_file = find_cat_file (candp->path, filename, man_file);
		found = display (candp->path, man_file, cat_file, title, NULL);
		if (cat_file)
			free (cat_file);
		free (title);

		return found;
	}
}

#ifdef MAN_DB_UPDATES
/* wrapper to dbdelete which deals with opening/closing the db */
static void dbdelete_wrapper (const char *page, struct mandata *info)
{
	if (!catman) {
		dbf = MYDBM_RWOPEN (database);
		if (dbf) {
			if (dbdelete (page, info) == 1)
				debug ("%s(%s) not in db!\n", page, info->ext);
			MYDBM_CLOSE(dbf);
		}
	}
}
#endif /* MAN_DB_UPDATES */

/* This started out life as try_section, but a lot of that routine is 
   redundant wrt the db cache. */
static int display_database (struct candidate *candp)
{
	int found = 0;
	char *file;
	const char *name;
	char *title;
	struct mandata *in = candp->source;

	debug ("trying a db located file.\n");
	dbprintf (in);

	/* if the pointer holds some data, this is a reference to the 
	   real page, use that instead. */
	if (*in->pointer != '-')
		name = in->pointer;
	else if (in->name)
		name = in->name;
	else
		name = candp->req_name;

	if (in->id == WHATIS_MAN || in->id == WHATIS_CAT)
		debug (_("%s: relying on whatis refs is deprecated\n"), name);

	title = strappend (NULL, name, "(", in->ext, ")", NULL);

#ifndef NROFF_MISSING /* #ifdef NROFF */
	/*
  	 * Look for man page source files.
  	 */

	if (in->id < STRAY_CAT) {	/* There should be a src page */
		file = make_filename (candp->path, name, in, "man");
		debug ("Checking physical location: %s\n", file);

		if (access (file, R_OK) == 0) {
			const char *man_file;
			char *cat_file;

			man_file = ult_src (file, candp->path, NULL,
					    SO_LINK | SOFT_LINK | HARD_LINK);
			if (man_file == NULL) {
				free (title);
				return found; /* zero */
			}

			debug ("found ultimate source file %s\n", man_file);
			lang = lang_dir (man_file);

			cat_file = find_cat_file (candp->path, file, man_file);
			found += display (candp->path, man_file, cat_file,
					  title, in->filter);
			if (cat_file)
				free (cat_file);
		} /* else {drop through to the bottom and return 0 anyway} */
	} else 

#endif /* NROFF_MISSING */

	if (in->id <= WHATIS_CAT) {
		/* The db says we have a stray cat or whatis ref */

		if (catman) {
			free (title);
			return ++found;
		}

		/* show this page but force an update later to make sure
		   we haven't just added the new page */
		found_a_stray = 1;

		/* If explicitly asked for troff or a different encoding,
		 * don't show a stray cat.
		 */
		if (troff || different_encoding) {
			free (title);
			return found;
		}

		file = make_filename (candp->path, name, in, "cat");
		debug ("Checking physical location: %s\n", file);

		if (access (file, R_OK) != 0) {
			char *catpath;
			catpath = get_catpath (candp->path,
					       global_manpath ? SYSTEM_CAT
							      : USER_CAT);

			if (catpath && strcmp (catpath, candp->path) != 0) {
				file = make_filename (catpath, name,
						      in, "cat");
				free (catpath);
				debug ("Checking physical location: %s\n",
				       file);

				if (access (file, R_OK) != 0) {
					/* don't delete here, 
					   return==0 will do that */
					free (title);
					return found; /* zero */
				}
			} else {
				if (catpath)
					free (catpath);
				free (title);
				return found; /* zero */
			}
		}

		found += display (candp->path, NULL, file, title, in->filter);
	}
	free (title);
	return found;
}

/* test for existence, if fail: call dbdelete_wrapper, else return amount */
static int display_database_check (struct candidate *candp)
{
	int exists = display_database (candp);

#ifdef MAN_DB_UPDATES
	if (!exists && !skip) {
		debug ("dbdelete_wrapper (%s, %p)\n",
		       candp->req_name, candp->source);
		dbdelete_wrapper (candp->req_name, candp->source);
	}
#endif /* MAN_DB_UPDATES */

	return exists;
}

static void db_hash_free (void *defn)
{
	free_mandata_struct (defn);
}

#ifdef MAN_DB_UPDATES
static int maybe_update_file (const char *manpath, const char *name,
			      struct mandata *info)
{
	const char *real_name;
	char *file;
	struct stat buf;
	int status;

	if (!update)
		return 0;

	/* If the pointer holds some data, then we need to look at that
	 * name in the filesystem instead.
	 */
	if (!STRNEQ (info->pointer, "-", 1))
		real_name = info->pointer;
	else if (info->name)
		real_name = info->name;
	else
		real_name = name;

	file = make_filename (manpath, real_name, info, "man");
	if (lstat (file, &buf) != 0)
		return 0;
	if (buf.st_mtime == info->_st_mtime)
		return 0;

	debug ("%s needs to be recached: %ld %ld\n",
	       file, (long) info->_st_mtime, (long) buf.st_mtime);
	status = run_mandb (0, manpath, file);
	if (status)
		error (0, 0, _("mandb command failed with exit status %d"),
		       status);

	return 1;
}
#endif /* MAN_DB_UPDATES */

/* Special return values from try_db(). */

#define TRY_DATABASE_OPEN_FAILED  -1

#ifdef MAN_DB_CREATES
#define TRY_DATABASE_CREATED      -2
#endif /* MAN_DB_CREATES */

#ifdef MAN_DB_UPDATES
#define TRY_DATABASE_UPDATED      -3
#endif /* MAN_DB_UPDATES */

/* Look for a page in the database. If db not accessible, return -1,
   otherwise return number of pages found. */
static int try_db (const char *manpath, const char *sec, const char *name,
		   struct candidate **cand_head)
{
	struct mandata *loc, *data;
	char *catpath;
	int found = 0;
#ifdef MAN_DB_UPDATES
	int found_stale = 0;
#endif /* MAN_DB_UPDATES */

	/* find out where our db for this manpath should be */

	catpath = get_catpath (manpath, global_manpath ? SYSTEM_CAT : USER_CAT);
	if (catpath) {
		database = mkdbname (catpath);
		free (catpath);
	} else
		database = mkdbname (manpath);

	if (!db_hash)
		db_hash = hash_create (&db_hash_free);

	/* Have we looked here already? */
	data = hash_lookup (db_hash, manpath, strlen (manpath));

	if (!data) {
		dbf = MYDBM_RDOPEN (database);
		if (dbf && dbver_rd (dbf)) {
			MYDBM_CLOSE (dbf);
			dbf = NULL;
		}
		if (dbf) {
			debug ("Succeeded in opening %s O_RDONLY\n", database);

			/* if section is set, only return those that match,
			   otherwise NULL retrieves all available */
			data = dblookup_all (name, section, match_case);
			hash_install (db_hash, manpath, strlen (manpath),
				      data);
			MYDBM_CLOSE (dbf);
#ifdef MAN_DB_CREATES
		} else if (!global_manpath) {
			/* create one */
			debug ("Failed to open %s O_RDONLY\n", database);
			if (run_mandb (1, manpath, NULL)) {
				data = infoalloc ();
				data->next = NULL;
				data->addr = NULL;
				hash_install (db_hash,
					      manpath, strlen (manpath),
					      data);
				return TRY_DATABASE_OPEN_FAILED;
			}
			return TRY_DATABASE_CREATED;
#endif /* MAN_DB_CREATES */
		} else {
			debug ("Failed to open %s O_RDONLY\n", database);
			data = infoalloc ();
			data->next = (struct mandata *) NULL;
			data->addr = NULL;
			hash_install (db_hash, manpath, strlen (manpath),
				      data);
			return TRY_DATABASE_OPEN_FAILED;
		}
	}

	/* if we already know that there is nothing here, get on with it */
	if (!data)
		return 0;

	/* We already tried (and failed) to open this db before */
	if (!data->addr)
		return TRY_DATABASE_OPEN_FAILED;

#ifdef MAN_DB_UPDATES
	/* Check that all the entries found are up to date. If not, the
	 * caller should try again.
	 */
	for (loc = data; loc; loc = loc->next)
		if (STREQ (sec, loc->sec) &&
		    (!extension || STREQ (extension, loc->ext)
				|| STREQ (extension, loc->ext + strlen (sec))))
			if (maybe_update_file (manpath, name, loc))
				found_stale = 1;

	if (found_stale) {
		hash_remove (db_hash, manpath, strlen (manpath));
		return TRY_DATABASE_UPDATED;
	}
#endif /* MAN_DB_UPDATES */

	/* cycle through the mandata structures (there's usually only 
	   1 or 2) and see what we have w.r.t. the current section */
	for (loc = data; loc; loc = loc->next)
		if (STREQ (sec, loc->sec) &&
		    (!extension || STREQ (extension, loc->ext)
				|| STREQ (extension, loc->ext + strlen (sec))))
			found += add_candidate (cand_head, CANDIDATE_DATABASE,
						0, name, manpath, loc);

	return found;
}

/* Try to locate the page under the specified manpath, in the desired section,
 * with the supplied name. Glob if necessary. Initially search the filesystem;
 * if that fails, try finding it via a db cache access. */
static int locate_page (const char *manpath, const char *sec, const char *name,
			struct candidate **candidates)
{
	int found, db_ok;

	/* sort out whether we want to treat this hierarchy as 
	   global or user. Differences:

	   global: if setuid, use privs; don't create db.
	   user  : if setuid, drop privs; allow db creation. */

	global_manpath = is_global_mandir (manpath);
	if (!global_manpath)
		drop_effective_privs ();

	debug ("searching in %s, section %s\n", manpath, sec);

	found = try_section (manpath, sec, name, candidates);

	if (!found || findall) {
		db_ok = try_db (manpath, sec, name, candidates);

#ifdef MAN_DB_CREATES
		if (db_ok == TRY_DATABASE_CREATED)
			/* we created a db in the last call */
			db_ok = try_db (manpath, sec, name, candidates);
#endif /* MAN_DB_CREATES */

#ifdef MAN_DB_UPDATES
		if (db_ok == TRY_DATABASE_UPDATED)
			/* We found some outdated entries and rebuilt the
			 * database in the last call. If this keeps
			 * happening, though, give up and punt to the
			 * filesystem.
			 */
			db_ok = try_db (manpath, sec, name, candidates);
#endif /* MAN_DB_UPDATES */

		if (db_ok > 0)  /* we found/opened a db and found something */
			found += db_ok;
	}

	if (!global_manpath)
		regain_effective_privs ();

	return found;
}

static int display_pages (struct candidate *candidates)
{
	struct candidate *candp;
	int found = 0;

	for (candp = candidates; candp; candp = candp->next) {
		global_manpath = is_global_mandir (candp->path);
		if (!global_manpath)
			drop_effective_privs ();

		switch (candp->from_db) {
			case CANDIDATE_FILESYSTEM:
				found += display_filesystem (candp);
				break;
			case CANDIDATE_DATABASE:
				found += display_database_check (candp);
				break;
			default:
				error (0, 0,
				       _("internal error: candidate type %d "
					 "out of range"), candp->from_db);
		}

		if (!global_manpath)
			regain_effective_privs ();

		if (found && !findall)
			return found;
	}

	return found;
}

/*
 * Search for manual pages.
 *
 * If preformatted manual pages are supported, look for the formatted
 * file first, then the man page source file.  If they both exist and
 * the man page source file is newer, or only the source file exists,
 * try to reformat it and write the results in the cat directory.  If
 * it is not possible to write the cat file, simply format and display
 * the man file.
 *
 * If preformatted pages are not supported, or the troff option is
 * being used, only look for the man page source file.
 *
 */
static int man (const char *name, int *found)
{
	struct candidate *candidates = NULL, *cand, *candnext;

	*found = 0;
	fflush (stdout);

	if (strchr (name, '/')) {
		int status = local_man_loop (name);
		if (status == OK)
			*found = 1;
		return status;
	}

	if (section) {
		char **mp;

		for (mp = manpathlist; *mp; mp++)
			*found += locate_page (*mp, section, name, &candidates);
	} else {
		const char **sp;

		for (sp = section_list; *sp; sp++) {
			char **mp;

			for (mp = manpathlist; *mp; mp++)
				*found += locate_page (*mp, *sp, name,
						       &candidates);
		}
	}

	if (*found)
		*found = display_pages (candidates);

	for (cand = candidates; cand; cand = candnext) {
		candnext = cand->next;
		free (cand);
	}

	return *found ? OK : NOT_FOUND;
}


static const char **get_section_list (void)
{
	int i = 0;
	const char **config_sections;
	const char **sections = NULL;
	const char *sec;

	/* Section list from configuration file, or STD_SECTIONS if it's
	 * empty.
	 */
	config_sections = get_sections ();
	if (!*config_sections)
		config_sections = std_sections;

	if (colon_sep_section_list == NULL)
		colon_sep_section_list = getenv ("MANSECT");
	if (colon_sep_section_list == NULL || *colon_sep_section_list == '\0')
		return config_sections;

	for (sec = strtok (colon_sep_section_list, ":"); sec; 
	     sec = strtok (NULL, ":")) {
		sections = xrealloc (sections, (i + 2) * sizeof *sections);
 		sections[i++] = sec;
 	}

	if (i > 0) {
		sections[i] = NULL;
		return sections;
	} else {
		if (sections)
			free (sections);
		return config_sections;
	}
}

/* allow user to skip a page or quit after viewing desired page 
   return 1 to skip
   return 0 to view
 */
static __inline__ int do_prompt (const char *name)
{
	int ch;

	skip = 0;
	if (!isatty (STDOUT_FILENO) || !isatty (STDIN_FILENO))
		return 0;

	fprintf (stderr, _( 
		 "--Man-- next: %s "
		 "[ view (return) | skip (Ctrl-D) | quit (Ctrl-C) ]\n"), 
		 name);
	fflush (stderr);

	do {
		ch = getchar ();
		switch (ch) {
			case '\n':
				return 0;
			case EOF:
				skip = 1;
				return 1;
			default:
				break;
		}
	} while (1);

	return 0;
}
