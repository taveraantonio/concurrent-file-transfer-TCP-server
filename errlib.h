/*

module: errlib.h

purpose: definitions of functions in errlib.c

reference: Stevens, Unix network programming (3ed)

*/

#ifndef _ERRLIB_H

#define _ERRLIB_H

#include <stdarg.h>

extern int daemon_proc;

void err_msg (const char *fmt, ...);

void err_quit (const char *fmt, ...);

void err_ret (const char *fmt, ...);

void err_sys (const char *fmt, ...);

void err_dump (const char *fmt, ...);

#endif
