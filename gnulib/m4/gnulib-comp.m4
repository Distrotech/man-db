# DO NOT EDIT! GENERATED AUTOMATICALLY!
# Copyright (C) 2004-2007 Free Software Foundation, Inc.
#
# This file is free software, distributed under the terms of the GNU
# General Public License.  As a special exception to the GNU General
# Public License, this file may be distributed as part of a program
# that contains a configuration script generated by Autoconf, under
# the same distribution terms as the rest of that program.
#
# Generated by gnulib-tool.
#
# This file represents the compiled summary of the specification in
# gnulib-cache.m4. It lists the computed macro invocations that need
# to be invoked from configure.ac.
# In projects using CVS, this file can be treated like other built files.


# This macro should be invoked from ./configure.ac, in the section
# "Checks for programs", right after AC_PROG_CC, and certainly before
# any checks for libraries, header files, types and library functions.
AC_DEFUN([gl_EARLY],
[
  m4_pattern_forbid([^gl_[A-Z]])dnl the gnulib macro namespace
  m4_pattern_allow([^gl_ES$])dnl a valid locale name
  m4_pattern_allow([^gl_LIBOBJS$])dnl a variable
  m4_pattern_allow([^gl_LTLIBOBJS$])dnl a variable
  AC_REQUIRE([AC_PROG_RANLIB])
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
])

# This macro should be invoked from ./configure.ac, in the section
# "Check for header files, types and library functions".
AC_DEFUN([gl_INIT],
[
  m4_pushdef([AC_LIBOBJ], m4_defn([gl_LIBOBJ]))
  m4_pushdef([AC_REPLACE_FUNCS], m4_defn([gl_REPLACE_FUNCS]))
  m4_pushdef([AC_LIBSOURCES], m4_defn([gl_LIBSOURCES]))
  AM_CONDITIONAL([GL_COND_LIBTOOL], [false])
  gl_cond_libtool=false
  gl_libdeps=
  gl_ltlibdeps=
  gl_source_base='gnulib/lib'
  gl_FUNC_ALLOCA
  gl_CHECK_TYPE_STRUCT_DIRENT_D_TYPE
  gl_ERROR
  gl_EXITFAIL
  # No macro. You should also use one of fnmatch-posix or fnmatch-gnu.
  gl_FUNC_FNMATCH_GNU
  gl_GETLOGIN_R
  gl_UNISTD_MODULE_INDICATOR([getlogin_r])
  gl_GETOPT
  AC_SUBST([LIBINTL])
  AC_SUBST([LTLIBINTL])
  gl_FUNC_GETTIMEOFDAY
  gl_GLOB
  gl_INLINE
  gl_FUNC_MALLOC_POSIX
  gl_STDLIB_MODULE_INDICATOR([malloc-posix])
  gl_MALLOCA
  gl_FUNC_MEMPCPY
  gl_STRING_MODULE_INDICATOR([mempcpy])
  gt_FUNC_MKDTEMP
  gl_STDLIB_MODULE_INDICATOR([mkdtemp])
  gl_FUNC_MKSTEMP
  gl_STDLIB_MODULE_INDICATOR([mkstemp])
  gl_FUNC_SETENV
  gl_FUNC_UNSETENV
  AM_STDBOOL_H
  gl_STDINT_H
  gl_STDLIB_H
  gl_FUNC_STRDUP
  gl_STRING_MODULE_INDICATOR([strdup])
  gl_HEADER_STRING_H
  gl_FUNC_STRNDUP
  gl_STRING_MODULE_INDICATOR([strndup])
  gl_FUNC_STRNLEN
  gl_STRING_MODULE_INDICATOR([strnlen])
  gl_HEADER_SYS_STAT_H
  AC_PROG_MKDIR_P
  gl_HEADER_SYS_TIME_H
  AC_PROG_MKDIR_P
  gl_FUNC_GEN_TEMPNAME
  gl_UNISTD_H
  gl_WCHAR_H
  gl_WCTYPE_H
  gl_XALLOC
  gl_XSTRNDUP
  LIBGNU_LIBDEPS="$gl_libdeps"
  AC_SUBST([LIBGNU_LIBDEPS])
  LIBGNU_LTLIBDEPS="$gl_ltlibdeps"
  AC_SUBST([LIBGNU_LTLIBDEPS])
  m4_popdef([AC_LIBSOURCES])
  m4_popdef([AC_REPLACE_FUNCS])
  m4_popdef([AC_LIBOBJ])
  AC_CONFIG_COMMANDS_PRE([
    gl_libobjs=
    gl_ltlibobjs=
    if test -n "$gl_LIBOBJS"; then
      # Remove the extension.
      sed_drop_objext='s/\.o$//;s/\.obj$//'
      for i in `for i in $gl_LIBOBJS; do echo "$i"; done | sed "$sed_drop_objext" | sort | uniq`; do
        gl_libobjs="$gl_libobjs $i.$ac_objext"
        gl_ltlibobjs="$gl_ltlibobjs $i.lo"
      done
    fi
    AC_SUBST([gl_LIBOBJS], [$gl_libobjs])
    AC_SUBST([gl_LTLIBOBJS], [$gl_ltlibobjs])
  ])
])

# Like AC_LIBOBJ, except that the module name goes
# into gl_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gl_LIBOBJ],
  [gl_LIBOBJS="$gl_LIBOBJS $1.$ac_objext"])

# Like AC_REPLACE_FUNCS, except that the module name goes
# into gl_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gl_REPLACE_FUNCS],
  [AC_CHECK_FUNCS([$1], , [gl_LIBOBJ($ac_func)])])

# Like AC_LIBSOURCES, except that it does nothing.
# We rely on EXTRA_lib..._SOURCES instead.
AC_DEFUN([gl_LIBSOURCES],
  [])

# This macro records the list of files which have been installed by
# gnulib-tool and may be removed by future gnulib-tool invocations.
AC_DEFUN([gl_FILE_LIST], [
  build-aux/link-warning.h
  lib/alloca.c
  lib/alloca_.h
  lib/error.c
  lib/error.h
  lib/exitfail.c
  lib/exitfail.h
  lib/fnmatch.c
  lib/fnmatch_.h
  lib/fnmatch_loop.c
  lib/getlogin_r.c
  lib/getopt.c
  lib/getopt1.c
  lib/getopt_.h
  lib/getopt_int.h
  lib/gettext.h
  lib/gettimeofday.c
  lib/glob-libc.h
  lib/glob.c
  lib/glob_.h
  lib/malloc.c
  lib/malloca.c
  lib/malloca.h
  lib/malloca.valgrind
  lib/mempcpy.c
  lib/mkdtemp.c
  lib/mkstemp.c
  lib/setenv.c
  lib/setenv.h
  lib/stdbool_.h
  lib/stdint_.h
  lib/stdlib_.h
  lib/strdup.c
  lib/string_.h
  lib/strndup.c
  lib/strnlen.c
  lib/sys_stat_.h
  lib/sys_time_.h
  lib/tempname.c
  lib/tempname.h
  lib/unistd_.h
  lib/unsetenv.c
  lib/wchar_.h
  lib/wctype_.h
  lib/xalloc-die.c
  lib/xalloc.h
  lib/xmalloc.c
  lib/xstrndup.c
  lib/xstrndup.h
  m4/absolute-header.m4
  m4/alloca.m4
  m4/d-type.m4
  m4/eealloc.m4
  m4/error.m4
  m4/exitfail.m4
  m4/extensions.m4
  m4/fnmatch.m4
  m4/getlogin_r.m4
  m4/getopt.m4
  m4/gettimeofday.m4
  m4/glob.m4
  m4/gnulib-common.m4
  m4/include_next.m4
  m4/inline.m4
  m4/longlong.m4
  m4/malloc.m4
  m4/malloca.m4
  m4/mbstate_t.m4
  m4/mempcpy.m4
  m4/mkdtemp.m4
  m4/mkstemp.m4
  m4/onceonly_2_57.m4
  m4/setenv.m4
  m4/stdbool.m4
  m4/stdint.m4
  m4/stdlib_h.m4
  m4/strdup.m4
  m4/string_h.m4
  m4/strndup.m4
  m4/strnlen.m4
  m4/sys_stat_h.m4
  m4/sys_time_h.m4
  m4/tempname.m4
  m4/ulonglong.m4
  m4/unistd_h.m4
  m4/wchar.m4
  m4/wctype.m4
  m4/wint_t.m4
  m4/xalloc.m4
  m4/xstrndup.m4
])
