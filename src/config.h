/* src/sysconf.h.  Generated from config.hin by configure.  */
/* -*- C -*- */
/* Copyright (c) 2007-2010 John E. Davis
 * This file is part of JED editor library source.
 *
 * You may distribute this file under the terms the GNU General Public
 * License.  See the file COPYING for more information.
 */

/* Note: this is for unix only. */

#ifndef JED_CONFIG_H
#define JED_CONFIG_H

/* define if you have stdlib.h */
#define HAVE_STDLIB_H 1

/* define if you have unistd.h */
#define HAVE_UNISTD_H 1

/* define if you have termios.h */
#define HAVE_TERMIOS_H 1

/* define if you have memory.h */
#define HAVE_MEMORY_H 1

/* define if you have malloc.h */
#define HAVE_MALLOC_H 1

/* define if you have limits.h */
/* #undef HAVE_LIMITS_H */

/* define if you have fcntl.h */
#define HAVE_FCNTL_H 1

/* define if you have sys/fcntl.h */
#define HAVE_SYS_FCNTL_H 1

#define HAVE_PTY_H 1
/* #undef HAVE_SYS_PTY_H */

/* define if you have memset */
#define HAVE_MEMSET 1

/* define if you have memcpy */
#define HAVE_MEMCPY 1

/* define if you have these. */
#define HAVE_SETENV 1
#define HAVE_UNSETENV 1
#define HAVE_SETSID 1
#define HAVE_PUTENV 1
#define HAVE_GETCWD 1
#define HAVE_TCGETATTR 1
#define HAVE_TCSETATTR 1
#define HAVE_CFGETOSPEED 1

#define HAVE_GETPGID 1
#define HAVE_SETPGID 1
#define HAVE_TCSETPGRP 1
#define HAVE_TCGETPGRP 1

#define HAVE_OPENPTY 1

#define HAVE_GLOB_H 1

#define HAVE_SYS_WAIT_H 1
#define HAVE_DIRENT_H 1
/* #undef HAVE_SYS_NDIR_H */
/* #undef HAVE_SYS_DIR_H */
/* #undef HAVE_NDIR_H */

/* #undef HAVE_PCRE_H */

#define HAVE_DLOPEN 1

#define HAVE_GRANTPT 1

#define HAVE_UTIME 1

#define HAVE_SETLOCALE 1

#define HAVE_SYMLINK 1
#define HAVE_GETHOSTNAME 1
#define HAVE_FSYNC 1

/* Define if you have the vsnprintf, snprintf functions and they return
 * EOF upon failure.
 */
#define HAVE_VSNPRINTF 1
#define HAVE_SNPRINTF 1

/* #undef mode_t */
/* #undef pid_t */
/* #undef uid_t */
/* #undef gid_t */

#define HAVE_DEV_T 1
#define HAVE_INO_T 1

#define SIZEOF_SHORT 2
#define SIZEOF_INT 4
#define SIZEOF_LONG 8
#define SIZEOF_FLOAT 4
#define SIZEOF_DOUBLE 8
#define SIZEOF_LONG_LONG 8

#define HAVE_LONG_LONG 1

/* The following set defines may be necessary to activate long file support */
/* #undef _FILE_OFFSET_BITS */
/* #undef _LARGE_FILES */
/* #undef _LARGEFILE_SOURCE */

/* For antialiased fonts, set this to 1 */
#define XJED_HAS_XRENDERFONT 1

/* Undefine this to disable floating point support. */
#define FLOAT_TYPE

#ifndef HAVE_DEV_T
typedef unsigned int dev_t;
#endif
#ifndef HAVE_INO_T
typedef unsigned long ino_t;
#endif

#if defined(ultrix) && !defined(__GNUC__)
# ifndef NO_PROTOTYPES
#  define NO_PROTOTYPES
# endif
#endif

#ifndef REAL_UNIX_SYSTEM
# define REAL_UNIX_SYSTEM
#endif

#ifndef __unix__
# define __unix__ 1
#endif

#ifdef _AIX
# ifndef _POSIX_SOURCE
#  define _POSIX_SOURCE
# endif
# ifndef _ALL_SOURCE
#  define _ALL_SOURCE 1
# endif
/* This may generate warnings but the fact is that without it, xlc will
 * INCORRECTLY inline many str* functions. */
/* # undef __STR__ */
#endif

/* #ifdef NeXT */
/* # ifndef _POSIX_SOURCE */
/* #  define _POSIX_SOURCE */
/* # endif */
/* #endif */

#ifdef NeXT
# define HAVE_UTIME 1
#endif

#ifdef HAVE_TERMIOS_H
# if defined(HAVE_TCGETATTR) && defined(HAVE_TCSETATTR)
#  define REALLY_HAVE_TERMIOS_H
# endif
#endif

#define HAS_MOUSE 1

/* Set this to 1 if the filesystem is case-sensitive */
#define JED_FILE_PRESERVE_CASE 1

#ifndef JED
# define JED
#endif

#endif /* JED_CONFIG_H */
