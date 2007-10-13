/*
 * whatis.c: search the index or whatis database(s) for words.
 *  
 * Copyright (C) 1994, 1995 Graeme W. Wilford. (Wilf.)
 * Copyright (C) 2001, 2002 Colin Watson.
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
 * routines for whatis and apropos programs. Whatis looks up the 
 * keyword for the description, apropos searches the entire database 
 * for word matches.
 *
 * Mon Aug  8 20:35:30 BST 1994  Wilf. (G.Wilford@ee.surrey.ac.uk) 
 *
 * CJW: Add more safety in the face of corrupted databases.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#ifndef STDC_HEADERS
extern int errno;
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if defined(STDC_HEADERS)
#  include <stdlib.h>
#  include <string.h>
#elif defined(HAVE_STRING_H)
#  include <string.h>
#elif defined(HAVE_STRINGS_H)
#  include <strings.h>
#else
extern char *strrchr();
#endif /* no string(s) header */

#include "gettext.h"
#include <locale.h>
#define _(String) gettext (String)

#ifdef HAVE_ICONV
#  include <iconv.h>
#endif /* HAVE_ICONV */

#ifdef HAVE_REGEX_H
#  include <sys/types.h>
#  include <regex.h>
#endif /* HAVE_REGEX_H */

#ifdef HAVE_LIBGEN_H
#  include <libgen.h>
#endif /* HAVE_LIBGEN_H */

#include "fnmatch.h"

#include "getopt.h"

#include "manconfig.h"

#include "error.h"
#include "setenv.h"
#include "pipeline.h"
#include "linelength.h"
#include "hashtable.h"

#include "mydbm.h"
#include "db_storage.h"

#include "encodings.h"
#include "manp.h"

static char *manpathlist[MAXDIRS];

extern char *user_config_file;
extern char *optarg;
extern int optind, opterr, optopt;
static int num_keywords;

char *program_name;
int am_apropos;
char *database;
MYDBM_FILE dbf;
int quiet = 1;

#ifdef HAVE_ICONV
iconv_t conv_to_locale;
#endif /* HAVE_ICONV */

#if defined(POSIX_REGEX) || defined(BSD_REGEX)
#  define REGEX
#endif

#ifdef REGEX
#  if defined(POSIX_REGEX)
static regex_t preg;  
#  endif
static int regex;
static int exact;
#  ifndef HAVE_REGEX_H
extern char *re_comp();
extern void regfree();
#  endif
#endif

static int wildcard;

static int require_all;

static int long_output;

static const char *section;

static struct hashtable *apropos_seen = NULL;

static const char args[] = "dvrewas:lhVm:M:fkL:C:";

static const struct option long_options[] =
{
	{"debug",	no_argument,		0, 'd'},
	{"verbose",	no_argument,		0, 'v'},
	{"regex",	no_argument,		0, 'r'},
	{"exact",	no_argument,		0, 'e'},
	{"wildcard",	no_argument,		0, 'w'},
	{"and",		no_argument,		0, 'a'}, /* apropos only */
	{"long",	no_argument,		0, 'l'},
	{"section",	required_argument,	0, 's'},
	{"help",	no_argument,		0, 'h'},
	{"version",	no_argument,		0, 'V'},
	{"systems",	required_argument,	0, 'm'},
	{"manpath",	required_argument,	0, 'M'},
	{"whatis",	no_argument,		0, 'f'},
	{"apropos",	no_argument,		0, 'k'},
	{"locale",	required_argument,	0, 'L'},
	{"config-file",	required_argument,	0, 'C'},
	{0, 0, 0, 0}
};

static void apropos_usage (int status)
{
	printf (_("usage: %s [-dalhV] [-r|-w|-e] [-s section] [-m systems] [-M manpath]\n"
		  "               [-L locale] [-C file] keyword ...\n"), program_name);
	printf (_(
		"-d, --debug                produce debugging info.\n"
		"-v, --verbose              print verbose warning messages.\n"
		"-r, --regex                interpret each keyword as a regex (default).\n"
		"-e, --exact                search each keyword for exact match.\n"
		"-w, --wildcard             the keyword(s) contain wildcards.\n"
		"-a, --and                  require all keywords to match.\n"
		"-l, --long                 do not trim output to terminal width.\n"
		"-s, --section section      search only this section.\n"
		"-m, --systems system       include alternate systems' man pages.\n"
		"-M, --manpath path         set search path for manual pages to `path'.\n"
		"-L, --locale locale        define the locale for this search.\n"
		"-C, --config-file file     use this user configuration file.\n"
		"-V, --version              show version.\n"
		"-h, --help                 show this usage message.\n"));

	exit (status);
}

static void whatis_usage (int status)
{
	printf (_("usage: %s [-dlhV] [-r|-w] [-s section] [-m systems] [-M manpath]\n"
		  "              [-L locale] [-C file] keyword ...\n"), program_name);
	printf (_(
		"-d, --debug                produce debugging info.\n"
		"-v, --verbose              print verbose warning messages.\n"
		"-r, --regex                interpret each keyword as a regex.\n"
		"-w, --wildcard             the keyword(s) contain wildcards.\n"
		"-l, --long                 do not trim output to terminal width.\n"
		"-s, --section section      search only this section.\n"
		"-m, --systems system       include alternate systems' man pages.\n"
		"-M, --manpath path         set search path for manual pages to `path'.\n"
		"-L, --locale locale        define the locale for this search.\n"
		"-C, --config-file file     use this user configuration file.\n"
		"-V, --version              show version.\n"
		"-h, --help                 show this usage message.\n"));

	exit (status);
}

static void usage (int status)
{
	if (am_apropos)
		apropos_usage (status);
	else
		whatis_usage (status);
}

static char *simple_convert (iconv_t conv, char *string)
{
#ifdef HAVE_ICONV
	if (conv != (iconv_t) -1) {
		size_t string_conv_alloc = strlen (string) + 1;
		char *string_conv = xmalloc (string_conv_alloc);
		for (;;) {
			char *inptr = string, *outptr = string_conv;
			size_t inleft = strlen (string);
			size_t outleft = string_conv_alloc - 1;
			if (iconv (conv, (ICONV_CONST char **) &inptr, &inleft,
				   &outptr, &outleft) == (size_t) -1 &&
			    errno == E2BIG) {
				string_conv_alloc <<= 1;
				string_conv = xrealloc (string_conv,
							string_conv_alloc);
			} else {
				/* Either we succeeded, or we've done our
				 * best; go ahead and print what we've got.
				 */
				string_conv[string_conv_alloc - 1 - outleft] =
					'\0';
				break;
			}
		}
		return string_conv;
	} else
#endif /* HAVE_ICONV */
		return xstrdup (string);
}

/* do the old thing, if we cannot find the relevant database */
static __inline__ int use_grep (char *page, char *manpath)
{
	char *whatis_file = appendstr (NULL, manpath, "/whatis", NULL);
	int status;

	if (access (whatis_file, R_OK) == 0) {
		pipeline *grep_pl = pipeline_new ();
		command *grep_cmd;
		const char *flags;
		char *anchored_page = NULL;

		if (am_apropos) {
#ifdef REGEX
			if (regex)
				flags = get_def_user (
					"apropos_regex_grep_flags",
					APROPOS_REGEX_GREP_FLAGS);
			else
#endif
				flags = get_def_user ("apropos_grep_flags",
						      APROPOS_GREP_FLAGS);
			anchored_page = xstrdup (page);
		} else {
			flags = get_def_user ("whatis_grep_flags",
					      WHATIS_GREP_FLAGS);
			anchored_page = appendstr (NULL, "^", page, NULL);
		}

		grep_cmd = command_new_argstr (get_def_user ("grep", GREP));
		command_argstr (grep_cmd, flags);
		command_args (grep_cmd, anchored_page, whatis_file, NULL);
		pipeline_command (grep_pl, grep_cmd);

		status = (do_system (grep_pl) == 0);

		free (anchored_page);
		pipeline_free (grep_pl);
	} else {
		debug ("warning: can't read the fallback whatis text database "
		       "%s/whatis\n", manpath);
		status = 0;
	}

	free (whatis_file);
	return status;
}

/* fill_in_whatis() is really a ../libdb/db_lookup.c routine but whatis.c
   is the only file that actually requires access to the whatis text... */
   
/* Take mandata struct (earlier returned from a dblookup()) and return 
   the relative whatis */
static char *get_whatis (struct mandata *info, const char *page)
{
	int rounds;
	const char *newpage;

	/* See if we need to fill in the whatis here. */
	if (*(info->pointer) == '-' || STREQ (info->pointer, page)) {
		if (info->whatis != NULL && *(info->whatis))
			return xstrdup (info->whatis);
		if (!quiet && *(info->pointer) != '-')
			error (0, 0, _("warning: %s contains a pointer loop"),
			       page);
		return xstrdup (_("(unknown subject)"));
	}

	/* Now we have to work through pointers. The limit of 10 is fairly
	 * arbitrary: it's just there to avoid an infinite loop.
	 */
	newpage = info->pointer;
	info = dblookup_exact (newpage, info->ext, 1);
	for (rounds = 0; rounds < 10; rounds++) {
		struct mandata *newinfo;

		/* If the pointer lookup fails, do nothing. */
		if (!info)
			return xstrdup (_("(unknown subject)"));

		/* See if we need to fill in the whatis here. */
		if (*(info->pointer) == '-' ||
		    STREQ (info->pointer, newpage)) {
			if (info->whatis != NULL && *(info->whatis)) {
				char *return_whatis = xstrdup (info->whatis);
				free_mandata_struct (info);
				return return_whatis;
			}
			if (!quiet && *(info->pointer) != '-')
				error (0, 0,
				       _("warning: %s contains a pointer loop"),
				       page);
			free_mandata_struct (info);
			return xstrdup (_("(unknown subject)"));
		}

		newinfo = dblookup_exact (info->pointer, info->ext, 1);
		free_mandata_struct (info);
		info = newinfo;
	}

	if (!quiet)
		error (0, 0, _("warning: %s contains a pointer loop"), page);
	return xstrdup (_("(unknown subject)"));
}

/* print out any matches found */
static void display (struct mandata *info, char *page)
{
	char *string, *whatis, *string_conv;
	const char *page_name;
	int line_len, rest;

	whatis = get_whatis (info, page);
	
	dbprintf (info);

	if (info->name)
		page_name = info->name;
	else
		page_name = page;

	line_len = get_line_length ();

	if (strlen (page_name) > (size_t) (line_len / 2)) {
		string = xstrndup (page_name, line_len / 2 - 3);
		string = appendstr (string, "...", NULL);
	} else
		string = xstrdup (page_name);
	string = appendstr (string, " (", info->ext, ")", NULL);
	if (!STREQ (info->pointer, "-") && !STREQ (info->pointer, page))
		string = appendstr (string, " [", info->pointer, "]", NULL);

	if (strlen (string) < (size_t) 20) {
		int i;
		string = xrealloc (string, 21);
		for (i = strlen (string); i < 20; ++i)
			string[i] = ' ';
		string[i] = '\0';
	}
	string = appendstr (string, " - ", NULL);

	rest = line_len - strlen (string);
	if (!long_output && strlen (whatis) > (size_t) rest) {
		whatis[rest - 3] = '\0';
		string = appendstr (string, whatis, "...\n", NULL);
	} else
		string = appendstr (string, whatis, "\n", NULL);

	string_conv = simple_convert (conv_to_locale, string);
	fputs (string_conv, stdout);

	free (whatis);
	free (string_conv);
	free (string);
}

/* return lowered version of s */
static char *lower (char *s)
{
	char *low, *p;

	p = low = (char *) xmalloc (strlen (s) +1);

	while (*s) {
		if (CTYPE (isupper, *s))
			*p++ = CTYPE (tolower, *s++);
		else
			*p++ = *s++;
	}

	*p = *s;
	return low;
}

/* lookup the page and display the results */
static __inline__ int do_whatis (char *page)
{
	struct mandata *info;
	int count = 0;

	info = dblookup_all (page, section, 0);
	while (info) {
		struct mandata *pinfo;
			
		display (info, page);
		count++;
		pinfo = info->next;	/* go on to next structure */
		free_mandata_elements (info);
	 	free (info);
		info = pinfo;
	}
	return count;
}

/* return 1 if page matches name, else 0 */
static int parse_name (char *page, char *dbname)
{ 
#ifdef REGEX
	if (regex)
#  if defined(POSIX_REGEX)
		return (regexec (&preg, dbname, 0, (regmatch_t *) 0, 0) == 0);
#  elif defined(BSD_REGEX)
		return re_exec (dbname);
#  endif
#endif /* REGEX */

	if (am_apropos && !wildcard) {
		char *lowdbname = lower (dbname);
		int ret = STREQ (lowdbname, page);
		free (lowdbname);
		return ret;
	}

	return (fnmatch (page, dbname, 0) == 0);
}

/* return 1 on word match */
static int match (char *lowpage, char *whatis)
{
	char *lowwhatis = lower (whatis);
	size_t len = strlen (lowpage);
	char *p, *begin;

	begin = lowwhatis;
	
	/* check for string match, then see if it is a _word_ */
	while (lowwhatis && (p = strstr (lowwhatis, lowpage))) {
		char *left = p - 1; 
		char *right = p + len;

		if ((p == begin || (!CTYPE (islower, *left) && *left != '_')) &&
		    (!*right || (!CTYPE (islower, *right) && *right != '_'))) {
		    	free (begin);
		    	return 1;
		}
		lowwhatis = p + 1;
	}

	free (begin);
	return 0;
}

/* TODO: How on earth do we allow multiple-word matches without
 * reimplementing fnmatch()?
 */
static int word_fnmatch (char *lowpage, char *whatis)
{
	char *lowwhatis = lower (whatis);
	char *begin = lowwhatis, *p;

	for (p = lowwhatis; *p; p++) {
		if (CTYPE (islower, *p) || *p == '_')
			continue;

		/* Check for multiple non-word characters in a row. */
		if (p <= begin + 1)
			begin++;
		else {
			*p = '\0';
			if (fnmatch (lowpage, begin, 0) == 0)
				return 1;
			begin = p + 1;
		}
	}

	return 0;
}

/* return 1 if page matches whatis, else 0 */
static int parse_whatis (char *page, char *lowpage, char *whatis)
{ 
#ifdef REGEX
	if (regex) 
#  if defined(POSIX_REGEX)
		return (regexec (&preg, whatis, 0, (regmatch_t *) 0, 0) == 0);
#  elif defined(BSD_REGEX)
		return re_exec (whatis);
#  endif
#endif /* REGEX */

	if (wildcard) {
		if (exact)
			return (fnmatch (page, whatis, 0) == 0);
		else
			return word_fnmatch (page, whatis);
	}

	return match (lowpage, whatis);
}

/* cjwatson: Optimized functions don't seem to be correct in some
 * circumstances; disabled for now.
 */
#undef BTREE

/* scan for the page, print any matches */
static int do_apropos (char *page, char *lowpage)
{
	datum key, cont;
	int found = 0;

#ifndef BTREE
	datum nextkey;

	key = MYDBM_FIRSTKEY (dbf);
	while (MYDBM_DPTR (key)) {
		cont = MYDBM_FETCH (dbf, key);
#else /* BTREE */
	int end;

	end = btree_nextkeydata (dbf, &key, &cont);
	while (!end) {
#endif /* !BTREE */
		char *tab;
		int got_match;
		struct mandata info;

		memset (&info, 0, sizeof (info));

		/* bug#4372, NULL pointer dereference in MYDBM_DPTR (cont),
		 * fix by dassen@wi.leidenuniv.nl (J.H.M.Dassen), thanx Ray.
		 * cjwatson: In that case, complain and exit, otherwise we
		 * might loop (bug #95052).
		 */
		if (!MYDBM_DPTR (cont))
		{
			debug ("key was %s\n", MYDBM_DPTR (key));
			error (FATAL, 0,
			       _("Database %s corrupted; rebuild with "
				 "mandb --create"),
			       database);
		}

		if (*MYDBM_DPTR (key) == '$')
			goto nextpage;

		if (*MYDBM_DPTR (cont) == '\t')
			goto nextpage;

		/* a real page */

		split_content (MYDBM_DPTR (cont), &info);

		/* If there's a section given, does it match either the
		 * section or extension of this page?
		 */
		if (section &&
		    (!STREQ (section, info.sec) && !STREQ (section, info.ext)))
			goto nextpage;

		tab = strrchr (MYDBM_DPTR (key), '\t');
		if (tab) 
			 *tab = '\0';

		if (am_apropos) {
			char *whatis;
			char *seen_key;
			int *seen_count;

			if (info.name)
				seen_key = xstrdup (info.name);
			else
				seen_key = xstrdup (MYDBM_DPTR (key));
			seen_key = appendstr (seen_key, " (", info.ext, ")",
					      NULL);
			seen_count = hash_lookup (apropos_seen, seen_key,
						  strlen (seen_key));
			if (seen_count && !require_all)
				goto nextpage_tab;
			got_match = parse_name (lowpage, MYDBM_DPTR (key));
			whatis = xstrdup (info.whatis);
			if (!got_match && whatis)
				got_match = parse_whatis (page, lowpage,
							  whatis);
			free (whatis);
			if (got_match) {
				if (!seen_count) {
					seen_count = xmalloc
						(sizeof *seen_count);
					*seen_count = 0;
					hash_install (apropos_seen, seen_key,
						      strlen (seen_key),
						      seen_count);
				}
				++(*seen_count);
				if (!require_all ||
				    *seen_count == num_keywords)
					display (&info, MYDBM_DPTR (key));
			}
			free (seen_key);
			found++;
		} else {
			got_match = parse_name (page, MYDBM_DPTR (key));
			if (got_match)
				display (&info, MYDBM_DPTR (key));
		}

		found += got_match;

nextpage_tab:
		if (tab)
			*tab = '\t';
nextpage:
#ifndef BTREE
		nextkey = MYDBM_NEXTKEY (dbf, key);
		MYDBM_FREE (MYDBM_DPTR (cont));
		MYDBM_FREE (MYDBM_DPTR (key));
		key = nextkey; 
#else /* BTREE */
		MYDBM_FREE (MYDBM_DPTR (cont));
		MYDBM_FREE (MYDBM_DPTR (key));
		end = btree_nextkeydata (dbf, &key, &cont);
#endif /* !BTREE */
		info.addr = NULL; /* == MYDBM_DPTR (cont), freed above */
		free_mandata_elements (&info);
	}

	return found;
}

/* loop through the man paths, searching for a match */
static int search (char *page)
{
	int found = 0;
	char *lowpage = lower (page);
	char *catpath, **mp;

	debug ("lower(%s) = \"%s\"\n", page, lowpage);

	for (mp = manpathlist; *mp; mp++) {
		catpath = get_catpath (*mp, SYSTEM_CAT | USER_CAT);
		
		if (catpath) {
			database = mkdbname (catpath);
			free (catpath);
		} else
			database = mkdbname (*mp);

		debug ("path=%s\n", *mp);

		dbf = MYDBM_RDOPEN (database);
		if (dbf && dbver_rd (dbf)) {
			MYDBM_CLOSE (dbf);
			dbf = NULL;
		}
		if (!dbf) {
			found += use_grep (page, *mp);			
			continue;
		}

		if (am_apropos)
			found += do_apropos (page, lowpage);
		else {
# ifdef REGEX
			if (regex || wildcard) {
# else /* !REGEX */
			if (wildcard) {
# endif /* REGEX */
				found += do_apropos (page, lowpage);
			} else
				found += do_whatis (page);
		}
		free (database);
		database = NULL;
		MYDBM_CLOSE (dbf);
	}

	chkr_garbage_detector ();

	if (!found)
		printf (_("%s: nothing appropriate.\n"), page);

	free (lowpage);

	return found;
}

int main (int argc, char *argv[])
{
	int c;
	char *manp = NULL;
	const char *alt_systems = "";
	char *multiple_locale = NULL, *internal_locale;
	const char *locale = NULL;
#ifdef HAVE_ICONV
	char *locale_charset;
#endif
	int status = OK;

	program_name = xstrdup (basename (argv[0]));
	if (STREQ (program_name, APROPOS_NAME))
		am_apropos = 1;
	else
		am_apropos = 0;

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

	while ((c = getopt_long (argc, argv, args,
				 long_options, NULL)) != EOF) {
		switch (c) {

			case 'd':
				debug_level = 1;
				break;
			case 'v':
				quiet = 0;
				break;
			case 'L':
				locale = optarg;
				break;
			case 'm':
				alt_systems = optarg;
				break;
			case 'M':
				manp = xstrdup (optarg);
				break;
			case 'e':
#ifdef REGEX
				regex = 0;
				exact = 1;
#endif
				break;
			case 'r':
#ifdef REGEX
				regex = 1;
#endif
				break;
			case 'w':
#ifdef REGEX
				regex = 0;
#endif
				wildcard = 1;
				break;
			case 'a':
				if (am_apropos)
					require_all = 1;
				else
					usage (FAIL);
				break;
			case 'l':
				long_output = 1;
				break;
			case 's':
				section = optarg;
				break;
			case 'f': /* fall through */
			case 'k': /* ignore, may be passed by man */
				break;
			case 'C':
				user_config_file = optarg;
				break;
			case 'V':
				ver ();
				break;
			case 'h':
				usage (OK);
				break;
			default:
				usage (FAIL);
				break;
		}
	}

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

	pipeline_install_sigchld ();

#if defined(REGEX)
	/* Become it even if it's null - GNU standards */
	/* if (getenv ("POSIXLY_CORRECT")) */
	if (am_apropos && !exact && !wildcard)
		regex = 1;
#endif

	/* Make sure that we have a keyword! */
	num_keywords = argc - optind;
	if (!num_keywords) {
		printf (_("%s what?\n"), program_name);
		free (internal_locale);
		free (program_name);
		return 0;
	}

	/* sort out the internal manpath */
	if (manp == NULL) {
		char tmp_locale[3];
		int idx;

		manp = add_nls_manpath (get_manpath (alt_systems),
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

	apropos_seen = hash_create (&plain_hash_free);

#ifdef HAVE_ICONV
	locale_charset = appendstr (NULL, get_locale_charset (), "//IGNORE",
				    NULL);
	conv_to_locale = iconv_open (locale_charset, "UTF-8");
	free (locale_charset);
#endif /* HAVE_ICONV */

	while (optind < argc) {
#if defined(POSIX_REGEX)		
		if (regex) {
			int errcode = regcomp (&preg, argv[optind], 
					       REG_EXTENDED |
					       REG_NOSUB |
					       REG_ICASE);
						   
			if (errcode) {
				char error_string[64];
				regerror (errcode, &preg, error_string, 64);
				error (FAIL, 0, _("fatal: regex `%s': %s"),
				       argv[optind], error_string);
			}
		}
#elif defined(BSD_REGEX)
		if (regex) {
			/* How to set type of regex ...? */
			char *error_string = re_comp (argv[optind]);
			if (error_string)
				error (FAIL, 0, _("fatal: regex `%s': %s"),
				       argv[optind], error_string);
		}
#endif /* REGEX */
		if (!search (argv[optind++]))
			status = NOT_FOUND;
#ifdef POSIX_REGEX
		regfree (&preg);
#endif /* POSIX_REGEX */
	}

#ifdef HAVE_ICONV
	if (conv_to_locale != (iconv_t) -1)
		iconv_close (conv_to_locale);
#endif /* HAVE_ICONV */
	hash_free (apropos_seen);
	free_pathlist (manpathlist);
	free (manp);
	free (internal_locale);
	free (program_name);
	exit (status);
}
