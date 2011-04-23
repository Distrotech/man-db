# DO NOT EDIT! GENERATED AUTOMATICALLY!
# Copyright (C) 2002-2011 Free Software Foundation, Inc.
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
# In projects that use version control, this file can be treated like
# other built files.


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
  AC_REQUIRE([AM_PROG_CC_C_O])
  # Code from module alloca:
  # Code from module alloca-opt:
  # Code from module areadlink-with-size:
  # Code from module arg-nonnull:
  # Code from module argp:
  # Code from module bitrotate:
  # Code from module btowc:
  # Code from module c++defs:
  # Code from module canonicalize:
  # Code from module canonicalize-lgpl:
  # Code from module chdir-long:
  # Code from module chown:
  # Code from module cloexec:
  # Code from module close:
  # Code from module close-hook:
  # Code from module configmake:
  # Code from module d-ino:
  # Code from module d-type:
  # Code from module dirent:
  # Code from module dirfd:
  # Code from module dirname:
  # Code from module dirname-lgpl:
  # Code from module dosname:
  # Code from module double-slash-root:
  # Code from module dup2:
  # Code from module environ:
  # Code from module errno:
  # Code from module error:
  # Code from module exitfail:
  # Code from module extensions:
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  # Code from module fchdir:
  # Code from module fclose:
  # Code from module fcntl:
  # Code from module fcntl-h:
  # Code from module fdopendir:
  # Code from module file-set:
  # Code from module float:
  # Code from module flock:
  # Code from module fnmatch:
  # Code from module fnmatch-gnu:
  # Code from module getcwd:
  # Code from module getdtablesize:
  # Code from module getlogin_r:
  # Code from module getopt-gnu:
  # Code from module getopt-posix:
  # Code from module gettext:
  # Code from module gettext-h:
  # Code from module gettimeofday:
  # Code from module glob:
  # Code from module hash:
  # Code from module hash-pjw:
  # Code from module hash-triple:
  # Code from module havelib:
  # Code from module idpriv-drop:
  # Code from module idpriv-droptemp:
  # Code from module include_next:
  # Code from module inline:
  # Code from module intprops:
  # Code from module langinfo:
  # Code from module lchown:
  # Code from module lib-ignore:
  # Code from module localcharset:
  # Code from module lock:
  # Code from module lstat:
  # Code from module malloc-gnu:
  # Code from module malloc-posix:
  # Code from module malloca:
  # Code from module mbrtowc:
  # Code from module mbsinit:
  # Code from module mbsrtowcs:
  # Code from module mbtowc:
  # Code from module memchr:
  # Code from module mempcpy:
  # Code from module memrchr:
  # Code from module minmax:
  # Code from module mkdir:
  # Code from module mkdtemp:
  # Code from module mkstemp:
  # Code from module multiarch:
  # Code from module nl_langinfo:
  # Code from module open:
  # Code from module openat:
  # Code from module openat-die:
  # Code from module pathmax:
  # Code from module rawmemchr:
  # Code from module readlink:
  # Code from module realloc-posix:
  # Code from module regex:
  # Code from module rename:
  # Code from module rmdir:
  # Code from module same:
  # Code from module same-inode:
  # Code from module save-cwd:
  # Code from module setenv:
  # Code from module sigaction:
  # Code from module signal:
  # Code from module sigprocmask:
  # Code from module size_max:
  # Code from module sleep:
  # Code from module ssize_t:
  # Code from module stat:
  # Code from module stdarg:
  dnl Some compilers (e.g., AIX 5.3 cc) need to be in c99 mode
  dnl for the builtin va_copy to work.  With Autoconf 2.60 or later,
  dnl AC_PROG_CC_STDC arranges for this.  With older Autoconf AC_PROG_CC_STDC
  dnl shouldn't hurt, though installers are on their own to set c99 mode.
  AC_REQUIRE([AC_PROG_CC_STDC])
  # Code from module stdbool:
  # Code from module stddef:
  # Code from module stdint:
  # Code from module stdio:
  # Code from module stdlib:
  # Code from module strcase:
  # Code from module strchrnul:
  # Code from module strdup-posix:
  # Code from module streq:
  # Code from module strerror:
  # Code from module string:
  # Code from module strings:
  # Code from module strndup:
  # Code from module strnlen:
  # Code from module strnlen1:
  # Code from module strsep:
  # Code from module sys_file:
  # Code from module sys_stat:
  # Code from module sys_time:
  # Code from module sysexits:
  # Code from module tempname:
  # Code from module threadlib:
  gl_THREADLIB_EARLY
  # Code from module time:
  # Code from module unistd:
  # Code from module unistd-safer:
  # Code from module unlink:
  # Code from module unsetenv:
  # Code from module vasnprintf:
  # Code from module vasprintf:
  # Code from module verify:
  # Code from module vsnprintf:
  # Code from module warn-on-use:
  # Code from module warnings:
  # Code from module wchar:
  # Code from module wcrtomb:
  # Code from module wctype-h:
  # Code from module xalloc:
  # Code from module xalloc-die:
  # Code from module xgetcwd:
  # Code from module xsize:
  # Code from module xstrndup:
  # Code from module xvasprintf:
])

# This macro should be invoked from ./configure.ac, in the section
# "Check for header files, types and library functions".
AC_DEFUN([gl_INIT],
[
  AM_CONDITIONAL([GL_COND_LIBTOOL], [true])
  gl_cond_libtool=true
  gl_m4_base='gnulib/m4'
  m4_pushdef([AC_LIBOBJ], m4_defn([gl_LIBOBJ]))
  m4_pushdef([AC_REPLACE_FUNCS], m4_defn([gl_REPLACE_FUNCS]))
  m4_pushdef([AC_LIBSOURCES], m4_defn([gl_LIBSOURCES]))
  m4_pushdef([gl_LIBSOURCES_LIST], [])
  m4_pushdef([gl_LIBSOURCES_DIR], [])
  gl_COMMON
  gl_source_base='gnulib/lib'
  # Code from module alloca:
changequote(,)dnl
LTALLOCA=`echo "$ALLOCA" | sed -e 's/\.[^.]* /.lo /g;s/\.[^.]*$/.lo/'`
changequote([, ])dnl
AC_SUBST([LTALLOCA])
  # Code from module alloca-opt:
  gl_FUNC_ALLOCA
  # Code from module areadlink-with-size:
  # Code from module arg-nonnull:
  # Code from module argp:
  gl_ARGP
  m4_ifdef([AM_XGETTEXT_OPTION],
    [AM_][XGETTEXT_OPTION([--flag=argp_error:2:c-format])
     AM_][XGETTEXT_OPTION([--flag=argp_failure:4:c-format])])
  # Code from module bitrotate:
  AC_REQUIRE([AC_C_INLINE])
  # Code from module btowc:
  gl_FUNC_BTOWC
  gl_WCHAR_MODULE_INDICATOR([btowc])
  # Code from module c++defs:
  # Code from module canonicalize:
  gl_FUNC_CANONICALIZE_FILENAME_MODE
  gl_MODULE_INDICATOR([canonicalize])
  gl_MODULE_INDICATOR_FOR_TESTS([canonicalize])
  gl_STDLIB_MODULE_INDICATOR([canonicalize_file_name])
  # Code from module canonicalize-lgpl:
  gl_CANONICALIZE_LGPL
  gl_MODULE_INDICATOR([canonicalize-lgpl])
  gl_STDLIB_MODULE_INDICATOR([canonicalize_file_name])
  gl_STDLIB_MODULE_INDICATOR([realpath])
  # Code from module chdir-long:
  gl_FUNC_CHDIR_LONG
  # Code from module chown:
  gl_FUNC_CHOWN
  gl_UNISTD_MODULE_INDICATOR([chown])
  # Code from module cloexec:
  gl_CLOEXEC
  gl_MODULE_INDICATOR_FOR_TESTS([cloexec])
  # Code from module close:
  gl_FUNC_CLOSE
  gl_UNISTD_MODULE_INDICATOR([close])
  # Code from module close-hook:
  # Code from module configmake:
  gl_CONFIGMAKE_PREP
  # Code from module d-ino:
  gl_CHECK_TYPE_STRUCT_DIRENT_D_INO
  # Code from module d-type:
  gl_CHECK_TYPE_STRUCT_DIRENT_D_TYPE
  # Code from module dirent:
  gl_DIRENT_H
  # Code from module dirfd:
  gl_FUNC_DIRFD
  gl_DIRENT_MODULE_INDICATOR([dirfd])
  # Code from module dirname:
  gl_DIRNAME
  gl_MODULE_INDICATOR([dirname])
  # Code from module dirname-lgpl:
  gl_DIRNAME_LGPL
  # Code from module dosname:
  # Code from module double-slash-root:
  gl_DOUBLE_SLASH_ROOT
  # Code from module dup2:
  gl_FUNC_DUP2
  gl_UNISTD_MODULE_INDICATOR([dup2])
  # Code from module environ:
  gl_ENVIRON
  gl_UNISTD_MODULE_INDICATOR([environ])
  # Code from module errno:
  gl_HEADER_ERRNO_H
  # Code from module error:
  gl_ERROR
  m4_ifdef([AM_XGETTEXT_OPTION],
    [AM_][XGETTEXT_OPTION([--flag=error:3:c-format])
     AM_][XGETTEXT_OPTION([--flag=error_at_line:5:c-format])])
  # Code from module exitfail:
  # Code from module extensions:
  # Code from module fchdir:
  gl_FUNC_FCHDIR
  gl_UNISTD_MODULE_INDICATOR([fchdir])
  # Code from module fclose:
  gl_FUNC_FCLOSE
  gl_STDIO_MODULE_INDICATOR([fclose])
  # Code from module fcntl:
  gl_FUNC_FCNTL
  gl_FCNTL_MODULE_INDICATOR([fcntl])
  # Code from module fcntl-h:
  gl_FCNTL_H
  # Code from module fdopendir:
  gl_FUNC_FDOPENDIR
  gl_DIRENT_MODULE_INDICATOR([fdopendir])
  gl_MODULE_INDICATOR([fdopendir])
  # Code from module file-set:
  # Code from module float:
  gl_FLOAT_H
  # Code from module flock:
  gl_FUNC_FLOCK
  gl_HEADER_SYS_FILE_MODULE_INDICATOR([flock])
  # Code from module fnmatch:
  gl_FUNC_FNMATCH_POSIX
  # Code from module fnmatch-gnu:
  gl_FUNC_FNMATCH_GNU
  # Code from module getcwd:
  gl_FUNC_GETCWD
  gl_UNISTD_MODULE_INDICATOR([getcwd])
  # Code from module getdtablesize:
  gl_FUNC_GETDTABLESIZE
  gl_UNISTD_MODULE_INDICATOR([getdtablesize])
  # Code from module getlogin_r:
  gl_FUNC_GETLOGIN_R
  gl_UNISTD_MODULE_INDICATOR([getlogin_r])
  # Code from module getopt-gnu:
  gl_FUNC_GETOPT_GNU
  gl_MODULE_INDICATOR_FOR_TESTS([getopt-gnu])
  # Code from module getopt-posix:
  gl_FUNC_GETOPT_POSIX
  # Code from module gettext:
  dnl you must add AM_GNU_GETTEXT([external]) or similar to configure.ac.
  AM_GNU_GETTEXT_VERSION([0.18.1])
  # Code from module gettext-h:
  AC_SUBST([LIBINTL])
  AC_SUBST([LTLIBINTL])
  # Code from module gettimeofday:
  gl_FUNC_GETTIMEOFDAY
  gl_SYS_TIME_MODULE_INDICATOR([gettimeofday])
  # Code from module glob:
  gl_GLOB
  # Code from module hash:
  gl_HASH
  # Code from module hash-pjw:
  # Code from module hash-triple:
  # Code from module havelib:
  # Code from module idpriv-drop:
  gl_IDPRIV
  # Code from module idpriv-droptemp:
  gl_IDPRIV
  # Code from module include_next:
  # Code from module inline:
  gl_INLINE
  # Code from module intprops:
  # Code from module langinfo:
  gl_LANGINFO_H
  # Code from module lchown:
  gl_FUNC_LCHOWN
  gl_UNISTD_MODULE_INDICATOR([lchown])
  # Code from module lib-ignore:
  gl_IGNORE_UNUSED_LIBRARIES
  # Code from module localcharset:
  gl_LOCALCHARSET
  LOCALCHARSET_TESTS_ENVIRONMENT="CHARSETALIASDIR=\"\$(top_builddir)/$gl_source_base\""
  AC_SUBST([LOCALCHARSET_TESTS_ENVIRONMENT])
  # Code from module lock:
  gl_LOCK
  # Code from module lstat:
  gl_FUNC_LSTAT
  gl_SYS_STAT_MODULE_INDICATOR([lstat])
  # Code from module malloc-gnu:
  gl_FUNC_MALLOC_GNU
  gl_MODULE_INDICATOR([malloc-gnu])
  # Code from module malloc-posix:
  gl_FUNC_MALLOC_POSIX
  gl_STDLIB_MODULE_INDICATOR([malloc-posix])
  # Code from module malloca:
  gl_MALLOCA
  # Code from module mbrtowc:
  gl_FUNC_MBRTOWC
  gl_WCHAR_MODULE_INDICATOR([mbrtowc])
  # Code from module mbsinit:
  gl_FUNC_MBSINIT
  gl_WCHAR_MODULE_INDICATOR([mbsinit])
  # Code from module mbsrtowcs:
  gl_FUNC_MBSRTOWCS
  gl_WCHAR_MODULE_INDICATOR([mbsrtowcs])
  # Code from module mbtowc:
  gl_FUNC_MBTOWC
  gl_STDLIB_MODULE_INDICATOR([mbtowc])
  # Code from module memchr:
  gl_FUNC_MEMCHR
  gl_STRING_MODULE_INDICATOR([memchr])
  # Code from module mempcpy:
  gl_FUNC_MEMPCPY
  gl_STRING_MODULE_INDICATOR([mempcpy])
  # Code from module memrchr:
  gl_FUNC_MEMRCHR
  gl_STRING_MODULE_INDICATOR([memrchr])
  # Code from module minmax:
  gl_MINMAX
  # Code from module mkdir:
  gl_FUNC_MKDIR
  # Code from module mkdtemp:
  gt_FUNC_MKDTEMP
  gl_STDLIB_MODULE_INDICATOR([mkdtemp])
  # Code from module mkstemp:
  gl_FUNC_MKSTEMP
  gl_STDLIB_MODULE_INDICATOR([mkstemp])
  # Code from module multiarch:
  gl_MULTIARCH
  # Code from module nl_langinfo:
  gl_FUNC_NL_LANGINFO
  gl_LANGINFO_MODULE_INDICATOR([nl_langinfo])
  # Code from module open:
  gl_FUNC_OPEN
  gl_FCNTL_MODULE_INDICATOR([open])
  # Code from module openat:
  gl_FUNC_OPENAT
  # Code from module openat-die:
  # Code from module pathmax:
  gl_PATHMAX
  # Code from module rawmemchr:
  gl_FUNC_RAWMEMCHR
  gl_STRING_MODULE_INDICATOR([rawmemchr])
  # Code from module readlink:
  gl_FUNC_READLINK
  gl_UNISTD_MODULE_INDICATOR([readlink])
  # Code from module realloc-posix:
  gl_FUNC_REALLOC_POSIX
  gl_STDLIB_MODULE_INDICATOR([realloc-posix])
  # Code from module regex:
  gl_REGEX
  # Code from module rename:
  gl_FUNC_RENAME
  gl_STDIO_MODULE_INDICATOR([rename])
  # Code from module rmdir:
  gl_FUNC_RMDIR
  gl_UNISTD_MODULE_INDICATOR([rmdir])
  # Code from module same:
  gl_SAME
  # Code from module same-inode:
  # Code from module save-cwd:
  gl_SAVE_CWD
  # Code from module setenv:
  gl_FUNC_SETENV
  gl_STDLIB_MODULE_INDICATOR([setenv])
  # Code from module sigaction:
  gl_SIGACTION
  gl_SIGNAL_MODULE_INDICATOR([sigaction])
  # Code from module signal:
  gl_SIGNAL_H
  # Code from module sigprocmask:
  gl_SIGNALBLOCKING
  gl_SIGNAL_MODULE_INDICATOR([sigprocmask])
  # Code from module size_max:
  gl_SIZE_MAX
  # Code from module sleep:
  gl_FUNC_SLEEP
  gl_UNISTD_MODULE_INDICATOR([sleep])
  # Code from module ssize_t:
  gt_TYPE_SSIZE_T
  # Code from module stat:
  gl_FUNC_STAT
  gl_SYS_STAT_MODULE_INDICATOR([stat])
  # Code from module stdarg:
  gl_STDARG_H
  # Code from module stdbool:
  AM_STDBOOL_H
  # Code from module stddef:
  gl_STDDEF_H
  # Code from module stdint:
  gl_STDINT_H
  # Code from module stdio:
  gl_STDIO_H
  # Code from module stdlib:
  gl_STDLIB_H
  # Code from module strcase:
  gl_STRCASE
  # Code from module strchrnul:
  gl_FUNC_STRCHRNUL
  gl_STRING_MODULE_INDICATOR([strchrnul])
  # Code from module strdup-posix:
  gl_FUNC_STRDUP_POSIX
  gl_STRING_MODULE_INDICATOR([strdup])
  # Code from module streq:
  # Code from module strerror:
  gl_FUNC_STRERROR
  gl_STRING_MODULE_INDICATOR([strerror])
  # Code from module string:
  gl_HEADER_STRING_H
  # Code from module strings:
  gl_HEADER_STRINGS_H
  # Code from module strndup:
  gl_FUNC_STRNDUP
  gl_STRING_MODULE_INDICATOR([strndup])
  # Code from module strnlen:
  gl_FUNC_STRNLEN
  gl_STRING_MODULE_INDICATOR([strnlen])
  # Code from module strnlen1:
  # Code from module strsep:
  gl_FUNC_STRSEP
  gl_STRING_MODULE_INDICATOR([strsep])
  # Code from module sys_file:
  gl_HEADER_SYS_FILE_H
  AC_PROG_MKDIR_P
  # Code from module sys_stat:
  gl_HEADER_SYS_STAT_H
  AC_PROG_MKDIR_P
  # Code from module sys_time:
  gl_HEADER_SYS_TIME_H
  AC_PROG_MKDIR_P
  # Code from module sysexits:
  gl_SYSEXITS
  # Code from module tempname:
  gl_FUNC_GEN_TEMPNAME
  # Code from module threadlib:
  gl_THREADLIB
  # Code from module time:
  gl_HEADER_TIME_H
  # Code from module unistd:
  gl_UNISTD_H
  # Code from module unistd-safer:
  gl_UNISTD_SAFER
  # Code from module unlink:
  gl_FUNC_UNLINK
  gl_UNISTD_MODULE_INDICATOR([unlink])
  # Code from module unsetenv:
  gl_FUNC_UNSETENV
  gl_STDLIB_MODULE_INDICATOR([unsetenv])
  # Code from module vasnprintf:
  gl_FUNC_VASNPRINTF
  # Code from module vasprintf:
  gl_FUNC_VASPRINTF
  gl_STDIO_MODULE_INDICATOR([vasprintf])
  m4_ifdef([AM_XGETTEXT_OPTION],
    [AM_][XGETTEXT_OPTION([--flag=asprintf:2:c-format])
     AM_][XGETTEXT_OPTION([--flag=vasprintf:2:c-format])])
  # Code from module verify:
  # Code from module vsnprintf:
  gl_FUNC_VSNPRINTF
  gl_STDIO_MODULE_INDICATOR([vsnprintf])
  # Code from module warn-on-use:
  # Code from module warnings:
  AC_SUBST([WARN_CFLAGS])
  # Code from module wchar:
  gl_WCHAR_H
  # Code from module wcrtomb:
  gl_FUNC_WCRTOMB
  gl_WCHAR_MODULE_INDICATOR([wcrtomb])
  # Code from module wctype-h:
  gl_WCTYPE_H
  # Code from module xalloc:
  gl_XALLOC
  # Code from module xalloc-die:
  # Code from module xgetcwd:
  gl_XGETCWD
  # Code from module xsize:
  gl_XSIZE
  # Code from module xstrndup:
  gl_XSTRNDUP
  # Code from module xvasprintf:
  gl_XVASPRINTF
  m4_ifdef([AM_XGETTEXT_OPTION],
    [AM_][XGETTEXT_OPTION([--flag=xasprintf:1:c-format])])
  # End of code from modules
  m4_ifval(gl_LIBSOURCES_LIST, [
    m4_syscmd([test ! -d ]m4_defn([gl_LIBSOURCES_DIR])[ ||
      for gl_file in ]gl_LIBSOURCES_LIST[ ; do
        if test ! -r ]m4_defn([gl_LIBSOURCES_DIR])[/$gl_file ; then
          echo "missing file ]m4_defn([gl_LIBSOURCES_DIR])[/$gl_file" >&2
          exit 1
        fi
      done])dnl
      m4_if(m4_sysval, [0], [],
        [AC_FATAL([expected source file, required through AC_LIBSOURCES, not found])])
  ])
  m4_popdef([gl_LIBSOURCES_DIR])
  m4_popdef([gl_LIBSOURCES_LIST])
  m4_popdef([AC_LIBSOURCES])
  m4_popdef([AC_REPLACE_FUNCS])
  m4_popdef([AC_LIBOBJ])
  AC_CONFIG_COMMANDS_PRE([
    gl_libobjs=
    gl_ltlibobjs=
    if test -n "$gl_LIBOBJS"; then
      # Remove the extension.
      sed_drop_objext='s/\.o$//;s/\.obj$//'
      for i in `for i in $gl_LIBOBJS; do echo "$i"; done | sed -e "$sed_drop_objext" | sort | uniq`; do
        gl_libobjs="$gl_libobjs $i.$ac_objext"
        gl_ltlibobjs="$gl_ltlibobjs $i.lo"
      done
    fi
    AC_SUBST([gl_LIBOBJS], [$gl_libobjs])
    AC_SUBST([gl_LTLIBOBJS], [$gl_ltlibobjs])
  ])
  gltests_libdeps=
  gltests_ltlibdeps=
  m4_pushdef([AC_LIBOBJ], m4_defn([gltests_LIBOBJ]))
  m4_pushdef([AC_REPLACE_FUNCS], m4_defn([gltests_REPLACE_FUNCS]))
  m4_pushdef([AC_LIBSOURCES], m4_defn([gltests_LIBSOURCES]))
  m4_pushdef([gltests_LIBSOURCES_LIST], [])
  m4_pushdef([gltests_LIBSOURCES_DIR], [])
  gl_COMMON
  gl_source_base='tests'
changequote(,)dnl
  gltests_WITNESS=IN_`echo "${PACKAGE-$PACKAGE_TARNAME}" | LC_ALL=C tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ | LC_ALL=C sed -e 's/[^A-Z0-9_]/_/g'`_GNULIB_TESTS
changequote([, ])dnl
  AC_SUBST([gltests_WITNESS])
  gl_module_indicator_condition=$gltests_WITNESS
  m4_pushdef([gl_MODULE_INDICATOR_CONDITION], [$gl_module_indicator_condition])
  m4_popdef([gl_MODULE_INDICATOR_CONDITION])
  m4_ifval(gltests_LIBSOURCES_LIST, [
    m4_syscmd([test ! -d ]m4_defn([gltests_LIBSOURCES_DIR])[ ||
      for gl_file in ]gltests_LIBSOURCES_LIST[ ; do
        if test ! -r ]m4_defn([gltests_LIBSOURCES_DIR])[/$gl_file ; then
          echo "missing file ]m4_defn([gltests_LIBSOURCES_DIR])[/$gl_file" >&2
          exit 1
        fi
      done])dnl
      m4_if(m4_sysval, [0], [],
        [AC_FATAL([expected source file, required through AC_LIBSOURCES, not found])])
  ])
  m4_popdef([gltests_LIBSOURCES_DIR])
  m4_popdef([gltests_LIBSOURCES_LIST])
  m4_popdef([AC_LIBSOURCES])
  m4_popdef([AC_REPLACE_FUNCS])
  m4_popdef([AC_LIBOBJ])
  AC_CONFIG_COMMANDS_PRE([
    gltests_libobjs=
    gltests_ltlibobjs=
    if test -n "$gltests_LIBOBJS"; then
      # Remove the extension.
      sed_drop_objext='s/\.o$//;s/\.obj$//'
      for i in `for i in $gltests_LIBOBJS; do echo "$i"; done | sed -e "$sed_drop_objext" | sort | uniq`; do
        gltests_libobjs="$gltests_libobjs $i.$ac_objext"
        gltests_ltlibobjs="$gltests_ltlibobjs $i.lo"
      done
    fi
    AC_SUBST([gltests_LIBOBJS], [$gltests_libobjs])
    AC_SUBST([gltests_LTLIBOBJS], [$gltests_ltlibobjs])
  ])
])

# Like AC_LIBOBJ, except that the module name goes
# into gl_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gl_LIBOBJ], [
  AS_LITERAL_IF([$1], [gl_LIBSOURCES([$1.c])])dnl
  gl_LIBOBJS="$gl_LIBOBJS $1.$ac_objext"
])

# Like AC_REPLACE_FUNCS, except that the module name goes
# into gl_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gl_REPLACE_FUNCS], [
  m4_foreach_w([gl_NAME], [$1], [AC_LIBSOURCES(gl_NAME[.c])])dnl
  AC_CHECK_FUNCS([$1], , [gl_LIBOBJ($ac_func)])
])

# Like AC_LIBSOURCES, except the directory where the source file is
# expected is derived from the gnulib-tool parameterization,
# and alloca is special cased (for the alloca-opt module).
# We could also entirely rely on EXTRA_lib..._SOURCES.
AC_DEFUN([gl_LIBSOURCES], [
  m4_foreach([_gl_NAME], [$1], [
    m4_if(_gl_NAME, [alloca.c], [], [
      m4_define([gl_LIBSOURCES_DIR], [gnulib/lib])
      m4_append([gl_LIBSOURCES_LIST], _gl_NAME, [ ])
    ])
  ])
])

# Like AC_LIBOBJ, except that the module name goes
# into gltests_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gltests_LIBOBJ], [
  AS_LITERAL_IF([$1], [gltests_LIBSOURCES([$1.c])])dnl
  gltests_LIBOBJS="$gltests_LIBOBJS $1.$ac_objext"
])

# Like AC_REPLACE_FUNCS, except that the module name goes
# into gltests_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gltests_REPLACE_FUNCS], [
  m4_foreach_w([gl_NAME], [$1], [AC_LIBSOURCES(gl_NAME[.c])])dnl
  AC_CHECK_FUNCS([$1], , [gltests_LIBOBJ($ac_func)])
])

# Like AC_LIBSOURCES, except the directory where the source file is
# expected is derived from the gnulib-tool parameterization,
# and alloca is special cased (for the alloca-opt module).
# We could also entirely rely on EXTRA_lib..._SOURCES.
AC_DEFUN([gltests_LIBSOURCES], [
  m4_foreach([_gl_NAME], [$1], [
    m4_if(_gl_NAME, [alloca.c], [], [
      m4_define([gltests_LIBSOURCES_DIR], [tests])
      m4_append([gltests_LIBSOURCES_LIST], _gl_NAME, [ ])
    ])
  ])
])

# This macro records the list of files which have been installed by
# gnulib-tool and may be removed by future gnulib-tool invocations.
AC_DEFUN([gl_FILE_LIST], [
  build-aux/arg-nonnull.h
  build-aux/c++defs.h
  build-aux/config.rpath
  build-aux/warn-on-use.h
  lib/alloca.c
  lib/alloca.in.h
  lib/areadlink-with-size.c
  lib/areadlink.h
  lib/argp-ba.c
  lib/argp-eexst.c
  lib/argp-fmtstream.c
  lib/argp-fmtstream.h
  lib/argp-fs-xinl.c
  lib/argp-help.c
  lib/argp-namefrob.h
  lib/argp-parse.c
  lib/argp-pin.c
  lib/argp-pv.c
  lib/argp-pvh.c
  lib/argp-xinl.c
  lib/argp.h
  lib/asnprintf.c
  lib/asprintf.c
  lib/at-func.c
  lib/basename-lgpl.c
  lib/basename.c
  lib/bitrotate.h
  lib/btowc.c
  lib/canonicalize-lgpl.c
  lib/canonicalize.c
  lib/canonicalize.h
  lib/chdir-long.c
  lib/chdir-long.h
  lib/chown.c
  lib/cloexec.c
  lib/cloexec.h
  lib/close-hook.c
  lib/close-hook.h
  lib/close.c
  lib/config.charset
  lib/dirent.in.h
  lib/dirfd.c
  lib/dirname-lgpl.c
  lib/dirname.c
  lib/dirname.h
  lib/dosname.h
  lib/dup-safer.c
  lib/dup2.c
  lib/errno.in.h
  lib/error.c
  lib/error.h
  lib/exitfail.c
  lib/exitfail.h
  lib/fchdir.c
  lib/fchmodat.c
  lib/fchown-stub.c
  lib/fchownat.c
  lib/fclose.c
  lib/fcntl.c
  lib/fcntl.in.h
  lib/fd-safer.c
  lib/fdopendir.c
  lib/file-set.c
  lib/file-set.h
  lib/float+.h
  lib/float.in.h
  lib/flock.c
  lib/fnmatch.c
  lib/fnmatch.in.h
  lib/fnmatch_loop.c
  lib/fstatat.c
  lib/getcwd.c
  lib/getdtablesize.c
  lib/getlogin_r.c
  lib/getopt.c
  lib/getopt.in.h
  lib/getopt1.c
  lib/getopt_int.h
  lib/gettext.h
  lib/gettimeofday.c
  lib/glob-libc.h
  lib/glob.c
  lib/glob.in.h
  lib/glthread/lock.c
  lib/glthread/lock.h
  lib/glthread/threadlib.c
  lib/hash-pjw.c
  lib/hash-pjw.h
  lib/hash-triple.c
  lib/hash-triple.h
  lib/hash.c
  lib/hash.h
  lib/idpriv-drop.c
  lib/idpriv-droptemp.c
  lib/idpriv.h
  lib/intprops.h
  lib/langinfo.in.h
  lib/lchown.c
  lib/localcharset.c
  lib/localcharset.h
  lib/lstat.c
  lib/malloc.c
  lib/malloca.c
  lib/malloca.h
  lib/malloca.valgrind
  lib/mbrtowc.c
  lib/mbsinit.c
  lib/mbsrtowcs-impl.h
  lib/mbsrtowcs-state.c
  lib/mbsrtowcs.c
  lib/mbtowc-impl.h
  lib/mbtowc.c
  lib/memchr.c
  lib/memchr.valgrind
  lib/mempcpy.c
  lib/memrchr.c
  lib/minmax.h
  lib/mkdir.c
  lib/mkdirat.c
  lib/mkdtemp.c
  lib/mkstemp.c
  lib/nl_langinfo.c
  lib/open.c
  lib/openat-die.c
  lib/openat-priv.h
  lib/openat-proc.c
  lib/openat.c
  lib/openat.h
  lib/pathmax.h
  lib/pipe-safer.c
  lib/printf-args.c
  lib/printf-args.h
  lib/printf-parse.c
  lib/printf-parse.h
  lib/rawmemchr.c
  lib/rawmemchr.valgrind
  lib/readlink.c
  lib/realloc.c
  lib/ref-add.sin
  lib/ref-del.sin
  lib/regcomp.c
  lib/regex.c
  lib/regex.h
  lib/regex_internal.c
  lib/regex_internal.h
  lib/regexec.c
  lib/rename.c
  lib/rmdir.c
  lib/same-inode.h
  lib/same.c
  lib/same.h
  lib/save-cwd.c
  lib/save-cwd.h
  lib/setenv.c
  lib/sig-handler.h
  lib/sigaction.c
  lib/signal.in.h
  lib/sigprocmask.c
  lib/size_max.h
  lib/sleep.c
  lib/stat.c
  lib/stdarg.in.h
  lib/stdbool.in.h
  lib/stddef.in.h
  lib/stdint.in.h
  lib/stdio.in.h
  lib/stdlib.in.h
  lib/strcasecmp.c
  lib/strchrnul.c
  lib/strchrnul.valgrind
  lib/strdup.c
  lib/streq.h
  lib/strerror.c
  lib/string.in.h
  lib/strings.in.h
  lib/stripslash.c
  lib/strncasecmp.c
  lib/strndup.c
  lib/strnlen.c
  lib/strnlen1.c
  lib/strnlen1.h
  lib/strsep.c
  lib/sys_file.in.h
  lib/sys_stat.in.h
  lib/sys_time.in.h
  lib/sysexits.in.h
  lib/tempname.c
  lib/tempname.h
  lib/time.in.h
  lib/unistd--.h
  lib/unistd-safer.h
  lib/unistd.in.h
  lib/unlink.c
  lib/unlinkat.c
  lib/unsetenv.c
  lib/vasnprintf.c
  lib/vasnprintf.h
  lib/vasprintf.c
  lib/verify.h
  lib/vsnprintf.c
  lib/wchar.in.h
  lib/wcrtomb.c
  lib/wctype.in.h
  lib/xalloc-die.c
  lib/xalloc.h
  lib/xasprintf.c
  lib/xgetcwd.c
  lib/xgetcwd.h
  lib/xmalloc.c
  lib/xsize.h
  lib/xstrndup.c
  lib/xstrndup.h
  lib/xvasprintf.c
  lib/xvasprintf.h
  m4/00gnulib.m4
  m4/alloca.m4
  m4/argp.m4
  m4/btowc.m4
  m4/canonicalize.m4
  m4/chdir-long.m4
  m4/chown.m4
  m4/cloexec.m4
  m4/close.m4
  m4/codeset.m4
  m4/configmake.m4
  m4/d-ino.m4
  m4/d-type.m4
  m4/dirent_h.m4
  m4/dirfd.m4
  m4/dirname.m4
  m4/double-slash-root.m4
  m4/dup2.m4
  m4/eealloc.m4
  m4/environ.m4
  m4/errno_h.m4
  m4/error.m4
  m4/extensions.m4
  m4/fchdir.m4
  m4/fclose.m4
  m4/fcntl-o.m4
  m4/fcntl.m4
  m4/fcntl_h.m4
  m4/fdopendir.m4
  m4/float_h.m4
  m4/flock.m4
  m4/fnmatch.m4
  m4/getcwd-abort-bug.m4
  m4/getcwd-path-max.m4
  m4/getcwd.m4
  m4/getdtablesize.m4
  m4/getlogin_r.m4
  m4/getopt.m4
  m4/gettext.m4
  m4/gettimeofday.m4
  m4/glibc2.m4
  m4/glibc21.m4
  m4/glob.m4
  m4/gnulib-common.m4
  m4/hash.m4
  m4/iconv.m4
  m4/idpriv.m4
  m4/include_next.m4
  m4/inline.m4
  m4/intdiv0.m4
  m4/intl.m4
  m4/intldir.m4
  m4/intlmacosx.m4
  m4/intmax.m4
  m4/intmax_t.m4
  m4/inttypes-pri.m4
  m4/inttypes_h.m4
  m4/langinfo_h.m4
  m4/lchown.m4
  m4/lcmessage.m4
  m4/lib-ignore.m4
  m4/lib-ld.m4
  m4/lib-link.m4
  m4/lib-prefix.m4
  m4/localcharset.m4
  m4/locale-fr.m4
  m4/locale-ja.m4
  m4/locale-zh.m4
  m4/lock.m4
  m4/longlong.m4
  m4/lstat.m4
  m4/malloc.m4
  m4/malloca.m4
  m4/mbrtowc.m4
  m4/mbsinit.m4
  m4/mbsrtowcs.m4
  m4/mbstate_t.m4
  m4/mbtowc.m4
  m4/memchr.m4
  m4/mempcpy.m4
  m4/memrchr.m4
  m4/minmax.m4
  m4/mkdir.m4
  m4/mkdtemp.m4
  m4/mkstemp.m4
  m4/mmap-anon.m4
  m4/mode_t.m4
  m4/multiarch.m4
  m4/nl_langinfo.m4
  m4/nls.m4
  m4/onceonly.m4
  m4/open.m4
  m4/openat.m4
  m4/pathmax.m4
  m4/po.m4
  m4/printf-posix.m4
  m4/printf.m4
  m4/progtest.m4
  m4/rawmemchr.m4
  m4/readlink.m4
  m4/realloc.m4
  m4/regex.m4
  m4/rename.m4
  m4/rmdir.m4
  m4/same.m4
  m4/save-cwd.m4
  m4/setenv.m4
  m4/sigaction.m4
  m4/signal_h.m4
  m4/signalblocking.m4
  m4/size_max.m4
  m4/sleep.m4
  m4/ssize_t.m4
  m4/stat.m4
  m4/stdarg.m4
  m4/stdbool.m4
  m4/stddef_h.m4
  m4/stdint.m4
  m4/stdint_h.m4
  m4/stdio_h.m4
  m4/stdlib_h.m4
  m4/strcase.m4
  m4/strchrnul.m4
  m4/strdup.m4
  m4/strerror.m4
  m4/string_h.m4
  m4/strings_h.m4
  m4/strndup.m4
  m4/strnlen.m4
  m4/strsep.m4
  m4/sys_file_h.m4
  m4/sys_stat_h.m4
  m4/sys_time_h.m4
  m4/sysexits.m4
  m4/tempname.m4
  m4/threadlib.m4
  m4/time_h.m4
  m4/uintmax_t.m4
  m4/unistd-safer.m4
  m4/unistd_h.m4
  m4/unlink.m4
  m4/vasnprintf.m4
  m4/vasprintf.m4
  m4/visibility.m4
  m4/vsnprintf.m4
  m4/warn-on-use.m4
  m4/warnings.m4
  m4/wchar_h.m4
  m4/wchar_t.m4
  m4/wcrtomb.m4
  m4/wctype_h.m4
  m4/wint_t.m4
  m4/xalloc.m4
  m4/xgetcwd.m4
  m4/xsize.m4
  m4/xstrndup.m4
  m4/xvasprintf.m4
])
