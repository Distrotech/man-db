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
 *
 * Mon Mar 13 20:27:36 GMT 1995  Wilf. (G.Wilford@ee.surrey.ac.uk) 
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#if defined(STDC_HEADERS)
#  include <string.h>
#  include <stdlib.h>
#elif defined(HAVE_STRING_H)
#  include <string.h>
#elif defined(HAVE_STRINGS_H)
#  include <strings.h>
#else /* no string(s) header */
extern char *strrchr();
#endif /* STDC_HEADERS */

#include <ctype.h>

#include "glob.h"
#include "fnmatch.h"

#include <sys/types.h>
#include <dirent.h>

#include "manconfig.h"

#include "error.h"
#include "hashtable.h"
#include "cleanup.h"

#include "globbing.h"

const char *extension;
static const char *mandir_layout = MANDIR_LAYOUT;

static __inline__ char *end_pattern (char *pattern, const char *sec)
{
	if (extension)
		pattern = appendstr (pattern, ".*", extension, "*", NULL);
	else
		pattern = appendstr (pattern, ".", sec, "*", NULL);

	return pattern;
}

#define LAYOUT_GNU	1
#define LAYOUT_HPUX	2
#define LAYOUT_IRIX	4
#define LAYOUT_SOLARIS	8
#define LAYOUT_BSD	16

static int parse_layout (const char *layout)
{
	if (!*layout)
		return LAYOUT_GNU | LAYOUT_HPUX | LAYOUT_IRIX |
		       LAYOUT_SOLARIS | LAYOUT_BSD;
	else {
		int flags = 0;

		char *upper_layout = xstrdup (layout);
		char *layoutp;
		for (layoutp = upper_layout; *layoutp; layoutp++)
			*layoutp = CTYPE (toupper, *layoutp);

		if (strstr (upper_layout, "GNU"))
			flags |= LAYOUT_GNU;
		if (strstr (upper_layout, "HPUX"))
			flags |= LAYOUT_HPUX;
		if (strstr (upper_layout, "IRIX"))
			flags |= LAYOUT_IRIX;
		if (strstr (upper_layout, "SOLARIS"))
			flags |= LAYOUT_SOLARIS;
		if (strstr (upper_layout, "BSD"))
			flags |= LAYOUT_BSD;

		free (upper_layout);
		return flags;
	}
}

struct dirent_hashent {
	char **names;
	size_t names_len, names_max;
};

static void dirent_hash_free (void *defn)
{
	struct dirent_hashent *hashent = defn;
	size_t i;

	for (i = 0; i < hashent->names_len; ++i)
		free (hashent->names[i]);
	free (hashent->names);
	free (hashent);
}

static struct hashtable *dirent_hash = NULL;

static int cache_compare (const void *a, const void *b)
{
	const char *left = *(const char **) a;
	const char *right = *(const char **) b;
	return strcasecmp (left, right);
}

static struct dirent_hashent *update_directory_cache (const char *path)
{
	struct dirent_hashent *cache;
	DIR *dir;
	struct dirent *entry;

	if (!dirent_hash) {
		dirent_hash = hash_create (&dirent_hash_free);
		push_cleanup ((cleanup_fun) hash_free, dirent_hash, 0);
	}
	cache = hash_lookup (dirent_hash, path, strlen (path));

	/* Check whether we've got this one already. */
	if (cache) {
		debug ("update_directory_cache %s: hit\n", path);
		return cache;
	}

	debug ("update_directory_cache %s: miss\n", path);

	dir = opendir (path);
	if (!dir) {
		debug_error ("can't open directory %s", path);
		return NULL;
	}

	cache = xmalloc (sizeof (struct dirent_hashent));
	cache->names_len = 0;
	cache->names_max = 1024;
	cache->names = xmalloc (sizeof (char *) * cache->names_max);

	/* Dump all the entries into cache->names, resizing if necessary. */
	for (entry = readdir (dir); entry; entry = readdir (dir)) {
		if (cache->names_len >= cache->names_max) {
			cache->names_max *= 2;
			cache->names =
				xrealloc (cache->names,
					  sizeof (char *) * cache->names_max);
		}
		cache->names[cache->names_len++] = xstrdup (entry->d_name);
	}

	qsort (cache->names, cache->names_len, sizeof *cache->names,
	       &cache_compare);

	hash_install (dirent_hash, path, strlen (path), cache);
	closedir (dir);

	return cache;
}

struct pattern_bsearch {
	char *pattern;
	size_t len;
};

static int pattern_compare (const void *a, const void *b)
{
	const struct pattern_bsearch *key = a;
	const char *memb = *(const char **) b;
	return strncasecmp (key->pattern, memb, key->len);
}

static int match_in_directory (const char *path, const char *pattern,
			       int ignore_case, glob_t *pglob)
{
	struct dirent_hashent *cache;
	size_t allocated = 4;
	int flags;
	struct pattern_bsearch pattern_start;
	char **bsearched;
	size_t i;

	/* look_for_file declares this static, so it's zero-initialised.
	 * globfree() can deal with checking it before freeing.
	 */
	globfree (pglob);

	pglob->gl_pathc = 0;
	pglob->gl_pathv = NULL;
	pglob->gl_offs = 0;

	cache = update_directory_cache (path);
	if (!cache) {
		debug ("directory cache update failed\n");
		return -1;
	}

	debug ("globbing pattern in %s: %s\n", path, pattern);

	pglob->gl_pathv = xmalloc (allocated * sizeof (char *));
	flags = ignore_case ? FNM_CASEFOLD : 0;

	pattern_start.pattern = xstrndup (pattern,
					  strcspn (pattern, "?*{}\\"));
	pattern_start.len = strlen (pattern_start.pattern);
	bsearched = bsearch (&pattern_start, cache->names, cache->names_len,
			     sizeof *cache->names, &pattern_compare);
	if (!bsearched) {
		free (pattern_start.pattern);
		pglob->gl_pathv[0] = NULL;
		return 0;
	}
	while (bsearched > cache->names &&
	       !strncasecmp (pattern_start.pattern, *(bsearched - 1),
			     pattern_start.len))
		--bsearched;

	for (i = bsearched - cache->names; i < cache->names_len; ++i) {
		int fnm;

		if (strncasecmp (pattern_start.pattern, cache->names[i],
				 pattern_start.len))
			break;

		fnm = fnmatch (pattern, cache->names[i], flags);
		if (fnm)
			continue;

		debug ("matched: %s/%s\n", path, cache->names[i]);

		if (pglob->gl_pathc >= allocated) {
			allocated *= 2;
			pglob->gl_pathv = xrealloc (
				pglob->gl_pathv, allocated * sizeof (char *));
		}
		pglob->gl_pathv[pglob->gl_pathc++] =
			appendstr (NULL, path, "/", cache->names[i], NULL);
	}

	free (pattern_start.pattern);

	if (pglob->gl_pathc >= allocated) {
		allocated *= 2;
		pglob->gl_pathv = xrealloc (pglob->gl_pathv,
					    allocated * sizeof (char *));
	}
	pglob->gl_pathv[pglob->gl_pathc] = NULL;

	return 0;
}

char **look_for_file (const char *hier, const char *sec,
		      const char *unesc_name, int cat, int match_case)
{
	char *pattern = NULL, *path = NULL;
	static glob_t gbuf;
	static int cleanup_installed = 0;
	int status = 1;
	static int layout = -1;
	char *name;

	if (!cleanup_installed) {
		/* appease valgrind */
		push_cleanup ((cleanup_fun) globfree, &gbuf, 0);
		cleanup_installed = 1;
	}

	/* This routine only does a minimum amount of matching. It does not
	   find cat files in the alternate cat directory. */

	if (layout == -1) {
		layout = parse_layout (mandir_layout);
		debug ("Layout is %s (%d)\n", mandir_layout, layout);
	}

	name = escape_shell (unesc_name);

	/* allow lookups like "3x foo" to match "../man3/foo.3x" */

	if ((layout & LAYOUT_GNU) && CTYPE (isdigit, *sec) && sec[1] != '\0') {
		path = appendstr (path, hier, cat ? "/cat" : "/man", "\t",
				  NULL);
		*strrchr (path, '\t') = *sec;
		pattern = end_pattern (appendstr (pattern, name, NULL), sec);

		status = match_in_directory (path, pattern, !match_case,
					     &gbuf);
	}

	/* AIX glob.h doesn't define GLOB_NOMATCH and the manpage is vague
	   regarding return status if there are no matches so check the
	   path count member also */
	   
	if ((layout & LAYOUT_GNU) && (status != 0 || gbuf.gl_pathc == 0)) {
		if (path)
			*path = '\0';
		if (pattern)
			*pattern = '\0';
		path = appendstr (path, hier, cat ? "/cat" : "/man", sec,
				  NULL);
		pattern = end_pattern (appendstr (pattern, name, NULL), sec);

		status = match_in_directory (path, pattern, !match_case,
					     &gbuf);
	}

	/* Try HPUX style compressed man pages */
	if ((layout & LAYOUT_HPUX) && (status != 0 || gbuf.gl_pathc == 0)) {
		if (path)
			*path = '\0';
		if (pattern)
			*pattern = '\0';
		path = appendstr (path, hier, cat ? "/cat" : "/man",
				  sec, ".Z", NULL);
		pattern = end_pattern (appendstr (pattern, name, NULL), sec);

		status = match_in_directory (path, pattern, !match_case,
					     &gbuf);
	}

	/* Try man pages without the section extension --- IRIX man pages */
	if ((layout & LAYOUT_IRIX) && (status != 0 || gbuf.gl_pathc == 0)) {
		if (path)
			*path = '\0';
		if (pattern)
			*pattern = '\0';
		path = appendstr (path, hier, cat ? "/cat" : "/man", sec,
				  NULL);
		pattern = appendstr (pattern, name, ".*", NULL);

		status = match_in_directory (path, pattern, !match_case,
					     &gbuf);
	}

	/* Try Solaris style man page directories */
	if ((layout & LAYOUT_SOLARIS) && (status != 0 || gbuf.gl_pathc == 0)) {
		if (path)
			*path = '\0';
		if (pattern)
			*pattern = '\0';
		/* TODO: This needs to be man/sec*, not just man/sec. */
		path = appendstr (path, hier, cat ? "/cat" : "/man", sec,
				  NULL);
		pattern = end_pattern (appendstr (pattern, name, NULL), sec);

		status = match_in_directory (path, pattern, !match_case,
					     &gbuf);
	}

	/* BSD cat pages take the extension .0 */
	if ((layout & LAYOUT_BSD) && (status != 0 || gbuf.gl_pathc == 0)) {
		if (path)
			*path = '\0';
		if (pattern)
			*pattern = '\0';
		if (cat) {
			path = appendstr (path, hier, "/cat", sec, NULL);
			pattern = appendstr (pattern, name, ".0*", NULL);
		} else {
			path = appendstr (path, hier, "/man", sec, NULL);
			pattern = end_pattern (appendstr (pattern, name, NULL),
					       sec);
		}
		status = match_in_directory (path, pattern, !match_case,
					     &gbuf);
	}

	free (name);
	free (path);
	free (pattern);

	if (status != 0 || gbuf.gl_pathc == 0)
		return NULL;
	else
		return gbuf.gl_pathv;
}
