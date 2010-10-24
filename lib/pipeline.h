/* Copyright (C) 1989, 1990, 1991, 1992, 2000, 2002
 * Free Software Foundation, Inc.
 * Copyright (C) 2003, 2004, 2005, 2007, 2008 Colin Watson.
 *   Written for groff by James Clark (jjc@jclark.com)
 *   Adapted for man-db by Colin Watson.
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
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 */

#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>

typedef void command_function_type (void *);
typedef void command_function_free_type (void *);

struct command;
typedef struct command command;

struct pipeline;
typedef struct pipeline pipeline;

/* ---------------------------------------------------------------------- */

/* Functions to build individual commands. */

/* Construct a new command. */
command *command_new (const char *name);

/* Convenience constructors wrapping command_new() and command_arg().
 * Terminate arguments with NULL.
 */
command *command_new_argv (const char *name, va_list argv);
command *command_new_args (const char *name, ...) ATTRIBUTE_SENTINEL;

/* Split argstr on whitespace to construct a command and arguments,
 * honouring shell-style single-quoting, double-quoting, and backslashes,
 * but not other shell evil like wildcards, semicolons, or backquotes. This
 * is a backward-compatibility hack to support old configuration file
 * directives; please try to avoid using it in new code.
 */
command *command_new_argstr (const char *argstr);

/* Construct a new command that calls a given function rather than executing
 * a process. The data argument is passed as the function's only argument,
 * and will be freed before returning using free_func (if non-NULL).
 *
 * command_* functions that deal with arguments cannot be used with the
 * command returned by this function.
 */
command *command_new_function (const char *name,
			       command_function_type *func,
			       command_function_free_type *free_func,
			       void *data);

/* Construct a new command that runs a sequence of commands. The commands
 * will be executed in forked children; if any exits non-zero then it will
 * terminate the sequence, as with "&&" in shell.
 *
 * command_* functions that deal with arguments cannot be used with the
 * command returned by this function.
 */
command *command_new_sequence (const char *name, ...) ATTRIBUTE_SENTINEL;

/* Return a new command that just passes data from its input to its output. */
command *command_new_passthrough (void);

/* Return a duplicate of a command. */
command *command_dup (command *cmd);

/* Add an argument to a command. */
void command_arg (command *cmd, const char *arg);

/* Convenience function to add an argument with printf substitutions. */
void command_argf (command *cmd, const char *format, ...)
	ATTRIBUTE_FORMAT_PRINTF(2, 3);

/* Convenience functions wrapping command_arg().
 * Terminate arguments with NULL.
 */
void command_argv (command *cmd, va_list argv);
void command_args (command *cmd, ...) ATTRIBUTE_SENTINEL;

/* Split argstr on whitespace to add a list of arguments, honouring
 * shell-style single-quoting, double-quoting, and backslashes, but not
 * other shell evil like wildcards, semicolons, or backquotes. This is a
 * backward-compatibility hack to support old configuration file directives;
 * please try to avoid using it in new code.
 */
void command_argstr (command *cmd, const char *argstr);

/* Set the nice(3) value for this command.  Defaults to 0.  Errors while
 * attempting to set the nice value are ignored, aside from emitting a debug
 * message.
 */
void command_nice (command *cmd, int nice);

/* If discard_err is non-zero, redirect this command's standard error to
 * /dev/null.  Otherwise, and by default, pass it through.
 */
void command_discard_err (command *cmd, int discard_err);

/* Set an environment variable while running this command. */
void command_setenv (command *cmd, const char *name, const char *value);

/* Add a command to a sequence. */
void command_sequence_command (command *cmd, command *child);

/* Dump a string representation of a command to stream. */
void command_dump (command *cmd, FILE *stream);

/* Return a string representation of a command. The caller should free the
 * result.
 */
char *command_tostring (command *cmd);

/* Destroy a command. Safely does nothing on NULL. */
void command_free (command *cmd);

/* ---------------------------------------------------------------------- */

/* Functions to build pipelines. */

/* Construct a new pipeline. */
pipeline *pipeline_new (void);

/* Convenience constructors wrapping pipeline_new() and pipeline_command().
 * Terminate commands with NULL.
 */
pipeline *pipeline_new_commandv (command *cmd1, va_list cmdv);
pipeline *pipeline_new_commands (command *cmd1, ...) ATTRIBUTE_SENTINEL;

/* Construct a new pipeline and add a single command to it. */
pipeline *pipeline_new_command_args (const char *name, ...) ATTRIBUTE_SENTINEL;

/* Joins two pipelines, neither of which are allowed to be started. Discards
 * want_out, want_outfile, and outfd from p1, and want_in, want_infile, and
 * infd from p2.
 */
pipeline *pipeline_join (pipeline *p1, pipeline *p2);

/* Connect the input of one or more sink pipelines to the output of a source
 * pipeline. The source pipeline may be started, but in that case want_out
 * must be negative; otherwise, discards want_out from source. In any event,
 * discards want_in from all sinks, none of which are allowed to be started.
 * Terminate arguments with NULL.
 *
 * This is an application-level connection; data may be intercepted between
 * the pipelines by the program before calling pipeline_pump(), which sets
 * data flowing from the source to the sinks. It is primarily useful when
 * more than one sink pipeline is involved, in which case the pipelines
 * cannot simply be concatenated into one.
 */
void pipeline_connect (pipeline *source, pipeline *sink, ...)
	ATTRIBUTE_SENTINEL;

/* Add a command to a pipeline. */
void pipeline_command (pipeline *p, command *cmd);

/* Construct a new command and add it to a pipeline in one go. */
void pipeline_command_args (pipeline *p, const char *name, ...)
	ATTRIBUTE_SENTINEL;

/* Construct a new command from a shell-quoted string and add it to a
 * pipeline in one go. See the comment against command_new_argstr() above if
 * you're tempted to use this function.
 */
void pipeline_command_argstr (pipeline *p, const char *argstr);

/* Convenience functions wrapping pipeline_command().
 * Terminate commands with NULL.
 */
void pipeline_commandv (pipeline *p, va_list cmdv);
void pipeline_commands (pipeline *p, ...) ATTRIBUTE_SENTINEL;

/* Return the number of commands in this pipeline. */
int pipeline_get_ncommands (pipeline *p);

/* Return command number n from this pipeline, counting from zero, or NULL
 * if n is out of range.
 */
command *pipeline_get_command (pipeline *p, int n);

/* Set command number n in this pipeline, counting from zero, to cmd, and
 * return the previous command in that position.  Do nothing and return NULL
 * if n is out of range.
 */
command *pipeline_set_command (pipeline *p, int n, command *cmd);

/* Set file descriptors to use as the input and output of the whole
 * pipeline.  If non-negative, fd is used directly as a file descriptor.  If
 * negative, pipeline_start will create pipes and store the input writing
 * half and the output reading half in the pipeline's infd or outfd field as
 * appropriate.  The default is to leave input and output as stdin and
 * stdout unless pipeline_want_infile or pipeline_want_outfile respectively
 * has been called.
 *
 * Calling these functions supersedes any previous call to
 * pipeline_want_infile or pipeline_want_outfile respectively.
 */
void pipeline_want_in (pipeline *p, int fd);
void pipeline_want_out (pipeline *p, int fd);

/* Set file names to open and use as the input and output of the whole
 * pipeline.  This may be more convenient than supplying file descriptors,
 * and guarantees that the files are opened with the same privileges under
 * which the pipeline is run.
 *
 * Calling these functions (even with NULL, which returns to the default of
 * leaving input and output as stdin and stdout) supersedes any previous
 * call to pipeline_want_in or pipeline_want_outfile respectively.
 */
void pipeline_want_infile (pipeline *p, const char *file);
void pipeline_want_outfile (pipeline *p, const char *file);

/* If ignore_signals is non-zero (which is the default), ignore SIGINT and
 * SIGQUIT while the pipeline is running, like system().  Otherwise, leave
 * their dispositions unchanged.
 */
void pipeline_ignore_signals (pipeline *p, int ignore_signals);

/* Get streams corresponding to infd and outfd respectively. The pipeline
 * must be started.
 */
FILE *pipeline_get_infile (pipeline *p);
FILE *pipeline_get_outfile (pipeline *p);

/* Dump a string representation of p to stream. */
void pipeline_dump (pipeline *p, FILE *stream);

/* Return a string representation of p. The caller should free the result. */
char *pipeline_tostring (pipeline *p);

/* Destroy a pipeline and all its commands. Safely does nothing on NULL.
 * May wait for the pipeline to complete if it has not already done so.
 */
void pipeline_free (pipeline *p);

/* ---------------------------------------------------------------------- */

/* Functions to run pipelines and handle signals. */

/* Start the processes in a pipeline. Installs this library's SIGCHLD
 * handler if not already installed. Calls error(FATAL) on error. */
void pipeline_start (pipeline *p);

/* Wait for a pipeline to complete and return the exit status. */
int pipeline_wait (pipeline *p);

/* Start a pipeline, wait for it to complete, and free it, all in one go. */
int pipeline_run (pipeline *p);

/* Pump data among one or more pipelines connected using pipeline_connect()
 * until all source pipelines have reached end-of-file and all data has been
 * written to all sinks (or failed). All relevant pipelines must be
 * supplied: that is, no pipeline that has been connected to a source
 * pipeline may be supplied unless that source pipeline is also supplied.
 * Automatically starts all pipelines if they are not already started, but
 * does not wait for them. Terminate arguments with NULL.
 */
void pipeline_pump (pipeline *p, ...) ATTRIBUTE_SENTINEL;

/* ---------------------------------------------------------------------- */

/* Functions to read output from pipelines. */

/* Read len bytes of data from the pipeline, returning the data block. len
 * is updated with the number of bytes read.
 */
const char *pipeline_read (pipeline *p, size_t *len);

/* Look ahead in the pipeline's output for len bytes of data, returning the
 * data block. len is updated with the number of bytes read. The starting
 * position of the next read or peek is not affected by this call.
 */
const char *pipeline_peek (pipeline *p, size_t *len);

/* Return the number of bytes of data that can be read using pipeline_read
 * or pipeline_peek solely from the peek cache, without having to read from
 * the pipeline itself (and thus potentially block).
 */
size_t pipeline_peek_size (pipeline *p);

/* Skip over and discard len bytes of data from the peek cache. Asserts that
 * enough data is available to skip, so you may want to check using
 * pipeline_peek_size first.
 */
void pipeline_peek_skip (pipeline *p, size_t len);

/* Read a line of data from the pipeline, returning it. */
const char *pipeline_readline (pipeline *p);

/* Look ahead in the pipeline's output for a line of data, returning it. The
 * starting position of the next read or peek is not affected by this call.
 */
const char *pipeline_peekline (pipeline *p);

#endif /* PIPELINE_H */
