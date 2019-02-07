/*

module: errlib.c

purpose: library of error functions

reference: Stevens, Unix network programming (3ed)

*/

#include <stdio.h>
#include <stdlib.h> /* exit() */
#include <stdarg.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>

#define MAXLINE 4095

int daemon_proc = 0; /* set to 0 if stdout/stderr available, else set to 1 */


/* Print message and return to caller
 * Caller specifies "errnoflag" and "level" */

static void err_doit (int errnoflag, int level, const char *fmt, va_list ap) {
	int errno_save = errno;
	size_t n;
	char buf[MAXLINE+1];

	vsnprintf (buf, MAXLINE, fmt, ap);
	n = strlen(buf);
	if (errnoflag)
		snprintf (buf+n, MAXLINE-n, ": %s", strerror(errno_save));
	strcat (buf, "\n");

	if (daemon_proc) {
		syslog (level, "%s", buf);
	} else {
		fflush (stdout);
		fputs (buf, stderr);
		fflush (stderr);
	}
}


/* Nonfatal error related to system call
 * Print message and return */

void err_ret (const char *fmt, ...) {
	va_list ap;

	va_start (ap, fmt);
	err_doit (1, LOG_INFO, fmt, ap);
	va_end (ap);
	return;
}


/* Fatal error related to system call
 * Print message and terminate */


void err_sys (const char *fmt, ...) {
	va_list ap;

	va_start (ap, fmt);
	err_doit (1, LOG_ERR, fmt, ap);
	va_end (ap);
	exit (1);
}


/* Nonfatal error unrelated to system call
 * Print message and return */

void err_msg (const char *fmt, ...) {
	va_list ap;

	va_start (ap, fmt);
	err_doit (0, LOG_INFO, fmt, ap);
	va_end (ap);
	return;
}

/* Fatal error unrelated to system call
 * Print message and terminate */

void err_quit (const char *fmt, ...) {
	va_list ap;

	va_start (ap, fmt);
	err_doit (0, LOG_ERR, fmt, ap);
	va_end (ap);
	exit (1);
}
