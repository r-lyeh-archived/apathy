/*
 * Apathy is a lightweight stream/file/mmap/path/virtual-filesystem IO C++11 library.
 * Copyright (c) 2011,2012,2013,2014 Mario 'rlyeh' Rodriguez
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See license copy at http://www.boost.org/LICENSE_1_0.txt)

 * Copyright (c) 2013 Dan Lecocq
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

 * todo:
 *
 * absolute
 * cd
 *
 * path, stem
 * file
 * ext
 * args
 * fileext
 * pathfile
 * pathfileextargs
 *
 * pop,push,at,size()
 * split,join,range
 *
 * sane
 * os
 * directory
 * is_dir, is_file
 * has_trail
 * parent
 * root
 *
 * piped-factories: { "http://", "file://", "zip://file://" }

 * @todo
 * interleaving
 * utf8 path/file cases
 * embed zip tocs

 * - rlyeh ~~ listening to Alice in chains / Nutshell
 */

#include "apathy.hpp"

/* C includes */
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

// C++ includes
#include <algorithm>
#include <chrono>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

/* OS includes */
#if defined(_WIN32)
#   include <winsock2.h>
#   include <windows.h>
#   include <ctime>
#   include <direct.h>
#   include <io.h>
#   include <sys/utime.h>
#   include <shlwapi.h> // PathIsDirectory()
#   pragma comment(lib, "Shlwapi.lib")
#   define $win32(...) __VA_ARGS__
#   define $welse(...)
#   ifdef _MSC_VER
#       define open _open
#       define close _close
#       define mkdir(path_,mode) _mkdir( path(path_).os().c_str() )
		typedef int mode_t;
#   endif
#else
#   include <sys/time.h>
#   include <unistd.h>
#   include <utime.h>
#   define $win32(...)
#   define $welse(...) __VA_ARGS__
#endif

// embed dependencies

#ifdef _MSC_VER

//#line 1 "dirent.hpp"
#ifndef DIRENT_H
#define DIRENT_H

/*
 * Define architecture flags so we don't need to include windows.h.
 * Avoiding windows.h makes it simpler to use windows sockets in conjunction
 * with dirent.h.
 */
#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_IX86)
#   define _X86_
#endif
#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_AMD64)
#define _AMD64_
#endif

#include <stdio.h>
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

/* Indicates that d_type field is available in dirent structure */
#define _DIRENT_HAVE_D_TYPE

/* Indicates that d_namlen field is available in dirent structure */
#define _DIRENT_HAVE_D_NAMLEN

/* Entries missing from MSVC 6.0 */
#if !defined(FILE_ATTRIBUTE_DEVICE)
#   define FILE_ATTRIBUTE_DEVICE 0x40
#endif

/* File type and permission flags for stat() */
#if !defined(S_IFMT)
#   define S_IFMT   _S_IFMT                     /* File type mask */
#endif
#if !defined(S_IFDIR)
#   define S_IFDIR  _S_IFDIR                    /* Directory */
#endif
#if !defined(S_IFCHR)
#   define S_IFCHR  _S_IFCHR                    /* Character device */
#endif
#if !defined(S_IFFIFO)
#   define S_IFFIFO _S_IFFIFO                   /* Pipe */
#endif
#if !defined(S_IFREG)
#   define S_IFREG  _S_IFREG                    /* Regular file */
#endif
#if !defined(S_IREAD)
#   define S_IREAD  _S_IREAD                    /* Read permission */
#endif
#if !defined(S_IWRITE)
#   define S_IWRITE _S_IWRITE                   /* Write permission */
#endif
#if !defined(S_IEXEC)
#   define S_IEXEC  _S_IEXEC                    /* Execute permission */
#endif
#if !defined(S_IFIFO)
#   define S_IFIFO _S_IFIFO                     /* Pipe */
#endif
#if !defined(S_IFBLK)
#   define S_IFBLK   0                          /* Block device */
#endif
#if !defined(S_IFLNK)
#   define S_IFLNK   0                          /* Link */
#endif
#if !defined(S_IFSOCK)
#   define S_IFSOCK  0                          /* Socket */
#endif

#if defined(_MSC_VER)
#   define S_IRUSR  S_IREAD                     /* Read user */
#   define S_IWUSR  S_IWRITE                    /* Write user */
#   define S_IXUSR  0                           /* Execute user */
#   define S_IRGRP  0                           /* Read group */
#   define S_IWGRP  0                           /* Write group */
#   define S_IXGRP  0                           /* Execute group */
#   define S_IROTH  0                           /* Read others */
#   define S_IWOTH  0                           /* Write others */
#   define S_IXOTH  0                           /* Execute others */
#endif

/* Maximum length of file name */
#if !defined(PATH_MAX)
#   define PATH_MAX MAX_PATH
#endif
#if !defined(FILENAME_MAX)
#   define FILENAME_MAX MAX_PATH
#endif
#if !defined(NAME_MAX)
#   define NAME_MAX FILENAME_MAX
#endif

/* File type flags for d_type */
#define DT_UNKNOWN  0
#define DT_REG      S_IFREG
#define DT_DIR      S_IFDIR
#define DT_FIFO     S_IFIFO
#define DT_SOCK     S_IFSOCK
#define DT_CHR      S_IFCHR
#define DT_BLK      S_IFBLK
#define DT_LNK      S_IFLNK

/* Macros for converting between st_mode and d_type */
#define IFTODT(mode) ((mode) & S_IFMT)
#define DTTOIF(type) (type)

/*
 * File type macros.  Note that block devices, sockets and links cannot be
 * distinguished on Windows and the macros S_ISBLK, S_ISSOCK and S_ISLNK are
 * only defined for compatibility.  These macros should always return false
 * on Windows.
 */
#define	S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#define	S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#define	S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#define	S_ISLNK(mode)  (((mode) & S_IFMT) == S_IFLNK)
#define	S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
#define	S_ISCHR(mode)  (((mode) & S_IFMT) == S_IFCHR)
#define	S_ISBLK(mode)  (((mode) & S_IFMT) == S_IFBLK)

/* Return the exact length of d_namlen without zero terminator */
#define _D_EXACT_NAMLEN(p) ((p)->d_namlen)

/* Return number of bytes needed to store d_namlen */
#define _D_ALLOC_NAMLEN(p) (PATH_MAX)

#ifdef __cplusplus
extern "C" {
#endif

/* Wide-character version */
struct _wdirent {
	long d_ino;                                 /* Always zero */
	unsigned short d_reclen;                    /* Structure size */
	size_t d_namlen;                            /* Length of name without \0 */
	int d_type;                                 /* File type */
	wchar_t d_name[PATH_MAX];                   /* File name */
};
typedef struct _wdirent _wdirent;

struct _WDIR {
	struct _wdirent ent;                        /* Current directory entry */
	WIN32_FIND_DATAW data;                      /* Private file data */
	int cached;                                 /* True if data is valid */
	HANDLE handle;                              /* Win32 search handle */
	wchar_t *patt;                              /* Initial directory name */
};
typedef struct _WDIR _WDIR;

static _WDIR *_wopendir (const wchar_t *dirname);
static struct _wdirent *_wreaddir (_WDIR *dirp);
static int _wclosedir (_WDIR *dirp);
static void _wrewinddir (_WDIR* dirp);

/* For compatibility with Symbian */
#define wdirent _wdirent
#define WDIR _WDIR
#define wopendir _wopendir
#define wreaddir _wreaddir
#define wclosedir _wclosedir
#define wrewinddir _wrewinddir

/* Multi-byte character versions */
struct dirent {
	long d_ino;                                 /* Always zero */
	unsigned short d_reclen;                    /* Structure size */
	size_t d_namlen;                            /* Length of name without \0 */
	int d_type;                                 /* File type */
	char d_name[PATH_MAX];                      /* File name */
};
typedef struct dirent dirent;

struct DIR {
	struct dirent ent;
	struct _WDIR *wdirp;
};
typedef struct DIR DIR;

static DIR *opendir (const char *dirname);
static struct dirent *readdir (DIR *dirp);
static int closedir (DIR *dirp);
static void rewinddir (DIR* dirp);

/* Internal utility functions */
static WIN32_FIND_DATAW *dirent_first (_WDIR *dirp);
static WIN32_FIND_DATAW *dirent_next (_WDIR *dirp);

static int dirent_mbstowcs_s(
	size_t *pReturnValue,
	wchar_t *wcstr,
	size_t sizeInWords,
	const char *mbstr,
	size_t count);

static int dirent_wcstombs_s(
	size_t *pReturnValue,
	char *mbstr,
	size_t sizeInBytes,
	const wchar_t *wcstr,
	size_t count);

static void dirent_set_errno (int error);

/*
 * Open directory stream DIRNAME for read and return a pointer to the
 * internal working area that is used to retrieve individual directory
 * entries.
 */
static _WDIR*
_wopendir(
	const wchar_t *dirname)
{
	_WDIR *dirp = NULL;
	int error;

	/* Must have directory name */
	if (dirname == NULL  ||  dirname[0] == '\0') {
		dirent_set_errno (ENOENT);
		return NULL;
	}

	/* Allocate new _WDIR structure */
	dirp = (_WDIR*) malloc (sizeof (struct _WDIR));
	if (dirp != NULL) {
		DWORD n;

		/* Reset _WDIR structure */
		dirp->handle = INVALID_HANDLE_VALUE;
		dirp->patt = NULL;
		dirp->cached = 0;

		/* Compute the length of full path plus zero terminator */
		n = GetFullPathNameW (dirname, 0, NULL, NULL);

		/* Allocate room for absolute directory name and search pattern */
		dirp->patt = (wchar_t*) malloc (sizeof (wchar_t) * n + 16);
		if (dirp->patt) {

			/*
			 * Convert relative directory name to an absolute one.  This
			 * allows rewinddir() to function correctly even when current
			 * working directory is changed between opendir() and rewinddir().
			 */
			n = GetFullPathNameW (dirname, n, dirp->patt, NULL);
			if (n > 0) {
				wchar_t *p;

				/* Append search pattern \* to the directory name */
				p = dirp->patt + n;
				if (dirp->patt < p) {
					switch (p[-1]) {
					case '\\':
					case '/':
					case ':':
						/* Directory ends in path separator, e.g. c:\temp\ */
						/*NOP*/;
						break;

					default:
						/* Directory name doesn't end in path separator */
						*p++ = '\\';
					}
				}
				*p++ = '*';
				*p = '\0';

				/* Open directory stream and retrieve the first entry */
				if (dirent_first (dirp)) {
					/* Directory stream opened successfully */
					error = 0;
				} else {
					/* Cannot retrieve first entry */
					error = 1;
					dirent_set_errno (ENOENT);
				}

			} else {
				/* Cannot retrieve full path name */
				dirent_set_errno (ENOENT);
				error = 1;
			}

		} else {
			/* Cannot allocate memory for search pattern */
			error = 1;
		}

	} else {
		/* Cannot allocate _WDIR structure */
		error = 1;
	}

	/* Clean up in case of error */
	if (error  &&  dirp) {
		_wclosedir (dirp);
		dirp = NULL;
	}

	return dirp;
}

/*
 * Read next directory entry.  The directory entry is returned in dirent
 * structure in the d_name field.  Individual directory entries returned by
 * this function include regular files, sub-directories, pseudo-directories
 * "." and ".." as well as volume labels, hidden files and system files.
 */
static struct _wdirent*
_wreaddir(
	_WDIR *dirp)
{
	WIN32_FIND_DATAW *datap;
	struct _wdirent *entp;

	/* Read next directory entry */
	datap = dirent_next (dirp);
	if (datap) {
		size_t n;
		DWORD attr;

		/* Pointer to directory entry to return */
		entp = &dirp->ent;

		/*
		 * Copy file name as wide-character string.  If the file name is too
		 * long to fit in to the destination buffer, then truncate file name
		 * to PATH_MAX characters and zero-terminate the buffer.
		 */
		n = 0;
		while (n + 1 < PATH_MAX  &&  datap->cFileName[n] != 0) {
			entp->d_name[n] = datap->cFileName[n];
			n++;
		}
		dirp->ent.d_name[n] = 0;

		/* Length of file name excluding zero terminator */
		entp->d_namlen = n;

		/* File type */
		attr = datap->dwFileAttributes;
		if ((attr & FILE_ATTRIBUTE_DEVICE) != 0) {
			entp->d_type = DT_CHR;
		} else if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			entp->d_type = DT_DIR;
		} else {
			entp->d_type = DT_REG;
		}

		/* Reset dummy fields */
		entp->d_ino = 0;
		entp->d_reclen = sizeof (struct _wdirent);

	} else {

		/* Last directory entry read */
		entp = NULL;

	}

	return entp;
}

/*
 * Close directory stream opened by opendir() function.  This invalidates the
 * DIR structure as well as any directory entry read previously by
 * _wreaddir().
 */
static int
_wclosedir(
	_WDIR *dirp)
{
	int ok;
	if (dirp) {

		/* Release search handle */
		if (dirp->handle != INVALID_HANDLE_VALUE) {
			FindClose (dirp->handle);
			dirp->handle = INVALID_HANDLE_VALUE;
		}

		/* Release search pattern */
		if (dirp->patt) {
			free (dirp->patt);
			dirp->patt = NULL;
		}

		/* Release directory structure */
		free (dirp);
		ok = /*success*/0;

	} else {
		/* Invalid directory stream */
		dirent_set_errno (EBADF);
		ok = /*failure*/-1;
	}
	return ok;
}

/*
 * Rewind directory stream such that _wreaddir() returns the very first
 * file name again.
 */
static void
_wrewinddir(
	_WDIR* dirp)
{
	if (dirp) {
		/* Release existing search handle */
		if (dirp->handle != INVALID_HANDLE_VALUE) {
			FindClose (dirp->handle);
		}

		/* Open new search handle */
		dirent_first (dirp);
	}
}

/* Get first directory entry (internal) */
static WIN32_FIND_DATAW*
dirent_first(
	_WDIR *dirp)
{
	WIN32_FIND_DATAW *datap;

	/* Open directory and retrieve the first entry */
	dirp->handle = FindFirstFileW (dirp->patt, &dirp->data);
	if (dirp->handle != INVALID_HANDLE_VALUE) {

		/* a directory entry is now waiting in memory */
		datap = &dirp->data;
		dirp->cached = 1;

	} else {

		/* Failed to re-open directory: no directory entry in memory */
		dirp->cached = 0;
		datap = NULL;

	}
	return datap;
}

/* Get next directory entry (internal) */
static WIN32_FIND_DATAW*
dirent_next(
	_WDIR *dirp)
{
	WIN32_FIND_DATAW *p;

	/* Get next directory entry */
	if (dirp->cached != 0) {

		/* A valid directory entry already in memory */
		p = &dirp->data;
		dirp->cached = 0;

	} else if (dirp->handle != INVALID_HANDLE_VALUE) {

		/* Get the next directory entry from stream */
		if (FindNextFileW (dirp->handle, &dirp->data) != FALSE) {
			/* Got a file */
			p = &dirp->data;
		} else {
			/* The very last entry has been processed or an error occured */
			FindClose (dirp->handle);
			dirp->handle = INVALID_HANDLE_VALUE;
			p = NULL;
		}

	} else {

		/* End of directory stream reached */
		p = NULL;

	}

	return p;
}

/*
 * Open directory stream using plain old C-string.
 */
static DIR*
opendir(
	const char *dirname)
{
	struct DIR *dirp;
	int error;

	/* Must have directory name */
	if (dirname == NULL  ||  dirname[0] == '\0') {
		dirent_set_errno (ENOENT);
		return NULL;
	}

	/* Allocate memory for DIR structure */
	dirp = (DIR*) malloc (sizeof (struct DIR));
	if (dirp) {
		wchar_t wname[PATH_MAX];
		size_t n;

		/* Convert directory name to wide-character string */
		error = dirent_mbstowcs_s (&n, wname, PATH_MAX, dirname, PATH_MAX);
		if (!error) {

			/* Open directory stream using wide-character name */
			dirp->wdirp = _wopendir (wname);
			if (dirp->wdirp) {
				/* Directory stream opened */
				error = 0;
			} else {
				/* Failed to open directory stream */
				error = 1;
			}

		} else {
			/*
			 * Cannot convert file name to wide-character string.  This
			 * occurs if the string contains invalid multi-byte sequences or
			 * the output buffer is too small to contain the resulting
			 * string.
			 */
			error = 1;
		}

	} else {
		/* Cannot allocate DIR structure */
		error = 1;
	}

	/* Clean up in case of error */
	if (error  &&  dirp) {
		free (dirp);
		dirp = NULL;
	}

	return dirp;
}

/*
 * Read next directory entry.
 *
 * When working with text consoles, please note that file names returned by
 * readdir() are represented in the default ANSI code page while any output to
 * console is typically formatted on another code page.  Thus, non-ASCII
 * characters in file names will not usually display correctly on console.  The
 * problem can be fixed in two ways: (1) change the character set of console
 * to 1252 using chcp utility and use Lucida Console font, or (2) use
 * _cprintf function when writing to console.  The _cprinf() will re-encode
 * ANSI strings to the console code page so many non-ASCII characters will
 * display correcly.
 */
static struct dirent*
readdir(
	DIR *dirp)
{
	WIN32_FIND_DATAW *datap;
	struct dirent *entp;

	/* Read next directory entry */
	datap = dirent_next (dirp->wdirp);
	if (datap) {
		size_t n;
		int error;

		/* Attempt to convert file name to multi-byte string */
		error = dirent_wcstombs_s(
			&n, dirp->ent.d_name, PATH_MAX, datap->cFileName, PATH_MAX);

		/*
		 * If the file name cannot be represented by a multi-byte string,
		 * then attempt to use old 8+3 file name.  This allows traditional
		 * Unix-code to access some file names despite of unicode
		 * characters, although file names may seem unfamiliar to the user.
		 *
		 * Be ware that the code below cannot come up with a short file
		 * name unless the file system provides one.  At least
		 * VirtualBox shared folders fail to do this.
		 */
		if (error  &&  datap->cAlternateFileName[0] != '\0') {
			error = dirent_wcstombs_s(
				&n, dirp->ent.d_name, PATH_MAX,
				datap->cAlternateFileName, PATH_MAX);
		}

		if (!error) {
			DWORD attr;

			/* Initialize directory entry for return */
			entp = &dirp->ent;

			/* Length of file name excluding zero terminator */
			entp->d_namlen = n - 1;

			/* File attributes */
			attr = datap->dwFileAttributes;
			if ((attr & FILE_ATTRIBUTE_DEVICE) != 0) {
				entp->d_type = DT_CHR;
			} else if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				entp->d_type = DT_DIR;
			} else {
				entp->d_type = DT_REG;
			}

			/* Reset dummy fields */
			entp->d_ino = 0;
			entp->d_reclen = sizeof (struct dirent);

		} else {
			/*
			 * Cannot convert file name to multi-byte string so construct
			 * an errornous directory entry and return that.  Note that
			 * we cannot return NULL as that would stop the processing
			 * of directory entries completely.
			 */
			entp = &dirp->ent;
			entp->d_name[0] = '?';
			entp->d_name[1] = '\0';
			entp->d_namlen = 1;
			entp->d_type = DT_UNKNOWN;
			entp->d_ino = 0;
			entp->d_reclen = 0;
		}

	} else {
		/* No more directory entries */
		entp = NULL;
	}

	return entp;
}

/*
 * Close directory stream.
 */
static int
closedir(
	DIR *dirp)
{
	int ok;
	if (dirp) {

		/* Close wide-character directory stream */
		ok = _wclosedir (dirp->wdirp);
		dirp->wdirp = NULL;

		/* Release multi-byte character version */
		free (dirp);

	} else {

		/* Invalid directory stream */
		dirent_set_errno (EBADF);
		ok = /*failure*/-1;

	}
	return ok;
}

/*
 * Rewind directory stream to beginning.
 */
static void
rewinddir(
	DIR* dirp)
{
	/* Rewind wide-character string directory stream */
	_wrewinddir (dirp->wdirp);
}

/* Convert multi-byte string to wide character string */
static int
dirent_mbstowcs_s(
	size_t *pReturnValue,
	wchar_t *wcstr,
	size_t sizeInWords,
	const char *mbstr,
	size_t count)
{
	int error;

#if defined(_MSC_VER)  &&  _MSC_VER >= 1400

	/* Microsoft Visual Studio 2005 or later */
	error = mbstowcs_s (pReturnValue, wcstr, sizeInWords, mbstr, count);

#else

	/* Older Visual Studio or non-Microsoft compiler */
	size_t n;

	/* Convert to wide-character string (or count characters) */
	n = mbstowcs (wcstr, mbstr, sizeInWords);
	if (!wcstr  ||  n < count) {

		/* Zero-terminate output buffer */
		if (wcstr  &&  sizeInWords) {
			if (n >= sizeInWords) {
				n = sizeInWords - 1;
			}
			wcstr[n] = 0;
		}

		/* Length of resuting multi-byte string WITH zero terminator */
		if (pReturnValue) {
			*pReturnValue = n + 1;
		}

		/* Success */
		error = 0;

	} else {

		/* Could not convert string */
		error = 1;

	}

#endif

	return error;
}

/* Convert wide-character string to multi-byte string */
static int
dirent_wcstombs_s(
	size_t *pReturnValue,
	char *mbstr,
	size_t sizeInBytes, /* max size of mbstr */
	const wchar_t *wcstr,
	size_t count)
{
	int error;

#if defined(_MSC_VER)  &&  _MSC_VER >= 1400

	/* Microsoft Visual Studio 2005 or later */
	error = wcstombs_s (pReturnValue, mbstr, sizeInBytes, wcstr, count);

#else

	/* Older Visual Studio or non-Microsoft compiler */
	size_t n;

	/* Convert to multi-byte string (or count the number of bytes needed) */
	n = wcstombs (mbstr, wcstr, sizeInBytes);
	if (!mbstr  ||  n < count) {

		/* Zero-terminate output buffer */
		if (mbstr  &&  sizeInBytes) {
			if (n >= sizeInBytes) {
				n = sizeInBytes - 1;
			}
			mbstr[n] = '\0';
		}

		/* Lenght of resulting multi-bytes string WITH zero-terminator */
		if (pReturnValue) {
			*pReturnValue = n + 1;
		}

		/* Success */
		error = 0;

	} else {

		/* Cannot convert string */
		error = 1;

	}

#endif

	return error;
}

/* Set errno variable */
static void
dirent_set_errno(
	int error)
{
#if defined(_MSC_VER)  &&  _MSC_VER >= 1400

	/* Microsoft Visual Studio 2005 and later */
	_set_errno (error);

#else

	/* Non-Microsoft compiler or older Microsoft compiler */
	errno = error;

#endif
}

#ifdef __cplusplus
}
#endif
#endif /*DIRENT_H*/


#else
#   include <dirent.h>
#endif

//#line 1 "giant.hpp"
//#pragma once

#include <cassert>
#include <algorithm>
#include <type_traits>

#if defined (__GLIBC__)
#   include <endian.h>
#endif

namespace giant
{

#if defined(_LITTLE_ENDIAN) \
	|| ( defined(BYTE_ORDER) && defined(LITTLE_ENDIAN) && BYTE_ORDER == LITTLE_ENDIAN ) \
	|| ( defined(_BYTE_ORDER) && defined(_LITTLE_ENDIAN) && _BYTE_ORDER == _LITTLE_ENDIAN ) \
	|| ( defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && __BYTE_ORDER == __LITTLE_ENDIAN ) \
	|| defined(__i386__) || defined(__alpha__) \
	|| defined(__ia64) || defined(__ia64__) \
	|| defined(_M_IX86) || defined(_M_IA64) \
	|| defined(_M_ALPHA) || defined(__amd64) \
	|| defined(__amd64__) || defined(_M_AMD64) \
	|| defined(__x86_64) || defined(__x86_64__) \
	|| defined(_M_X64)
	enum { xinu_type = 0, unix_type = 1, nuxi_type = 2, type = xinu_type, is_little = 1, is_big = 0 };
#elif defined(_BIG_ENDIAN) \
	|| ( defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN ) \
	|| ( defined(_BYTE_ORDER) && defined(_BIG_ENDIAN) && _BYTE_ORDER == _BIG_ENDIAN ) \
	|| ( defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && __BYTE_ORDER == __BIG_ENDIAN ) \
	|| defined(__sparc) || defined(__sparc__) \
	|| defined(_POWER) || defined(__powerpc__) \
	|| defined(__ppc__) || defined(__hpux) \
	|| defined(_MIPSEB) || defined(_POWER) \
	|| defined(__s390__)
	enum { xinu_type = 0, unix_type = 1, nuxi_type = 2, type = unix_type, is_little = 0, is_big = 1 };
#else
#   error <giant/giant.hpp> says: Middle endian/NUXI order is not supported
	enum { xinu_type = 0, unix_type = 1, nuxi_type = 2, type = nuxi_type, is_little = 0, is_big = 0 };
#endif

	template<typename T>
	T swap( T out )
	{
		static union autodetect {
			int word;
			char byte[ sizeof(int) ];
			autodetect() : word(1) {
				assert(( "<giant/giant.hpp> says: wrong endianness detected!", (!byte[0] && is_big) || (byte[0] && is_little) ));
			}
		} _;

		if( !std::is_pod<T>::value ) {
			return out;
		}

		char *ptr;

		switch( sizeof( T ) ) {
			case 0:
			case 1:
				break;
			case 2:
				ptr = reinterpret_cast<char *>(&out);
				std::swap( ptr[0], ptr[1] );
				break;
			case 4:
				ptr = reinterpret_cast<char *>(&out);
				std::swap( ptr[0], ptr[3] );
				std::swap( ptr[1], ptr[2] );
				break;
			case 8:
				ptr = reinterpret_cast<char *>(&out);
				std::swap( ptr[0], ptr[7] );
				std::swap( ptr[1], ptr[6] );
				std::swap( ptr[2], ptr[5] );
				std::swap( ptr[3], ptr[4] );
				break;
			case 16:
				ptr = reinterpret_cast<char *>(&out);
				std::swap( ptr[0], ptr[15] );
				std::swap( ptr[1], ptr[14] );
				std::swap( ptr[2], ptr[13] );
				std::swap( ptr[3], ptr[12] );
				std::swap( ptr[4], ptr[11] );
				std::swap( ptr[5], ptr[10] );
				std::swap( ptr[6], ptr[9] );
				std::swap( ptr[7], ptr[8] );
				break;
			default:
				assert( !"<giant/giant.hpp> says: POD type bigger than 256 bits (?)" );
				break;
		}

		return out;
	}

	template<typename T>
	T letobe( const T &in ) {
		return swap( in );
	}
	template<typename T>
	T betole( const T &in ) {
		return swap( in );
	}

	template<typename T>
	T letoh( const T &in ) {
		return type == xinu_type ? in : swap( in );
	}
	template<typename T>
	T htole( const T &in ) {
		return type == xinu_type ? in : swap( in );
	}

	template<typename T>
	T betoh( const T &in ) {
		return type == unix_type ? in : swap( in );
	}
	template<typename T>
	T htobe( const T &in ) {
		return type == unix_type ? in : swap( in );
	}
}



//#line 1 "fmstream.h"
#ifndef FILE_MAPPING_STREAM_H_
#define FILE_MAPPING_STREAM_H_

#include <istream>

/**
 * At this time, move constructor and move assignment for streams are only implements in Microsoft Visual Studio 2010 and Intel C++ Compiler 12
 */
#if ((__cplusplus > 199711L) || (_HAS_CPP0X > 0)) && ((_MSC_VER >= 1600) || (__INTEL_COMPILER >= 1200))
#define _HAS_CPP11_ 1
#endif

/**
 * File mapping utility class.
 */
class filemapping
{
public:
	/**
	 * Get memory offset granularity.
	 * Return the offset granularity of the system.
	 * @return Return the offset granularity of the system.
	 * @see filemappingbuf::open()
	 * @see ifmstream::open()
	 * @see fmstream::open()
	 */
	static std::streamoff offset_granularity();
};

/**
 * File mapping stream buffer.
 * This class applies the functionality of the std::streambuf class to read and write from/to memory-mapped files.
 * By calling member open, a physical file is associated to the file buffer as its associated character sequence.
 * Depending on the mode used in this operation, the access to the controlled input sequence or the controlled output sequence may be restricted.
 * The state of the filemappingbuf object -i.e. whether a file is open or not- may be tested by calling member function is_open.
 * Internally, filemappingbuf objects operate as defined in the std::streambuf class.
 * The class overrides some virtual members inherited from streambuf to provide a specific functionality for memory-mapped files.
 */
class filemappingbuf : public std::streambuf
{
public:
	/**
	 * Construct object.
	 * A filemappingbuf object is constructed, initializing all its pointers to null pointers and initializing the object's locale.
	 */
	filemappingbuf();

	/**
	 * Destructs the filemappingbuf object.
	 */
	virtual ~filemappingbuf();

#ifdef _HAS_CPP11_
	/** @name C++11
	 * The following methods requires some features introduced by the latest revision of the C++ standard (2011). Older compilers may not support it.
	 */
	///@{
	/**
	 * Move constructor (requires C++11).
	 * Acquires the contents of rhs_buf, by move-assigning its members and base classes.
	 * @param rhs_buf filemappingbuf to move. rhs_buf becomes of invalid state after the operation.
	 * @see swap()
	 * @see operator=()
	 */
	filemappingbuf(filemappingbuf&& rhs_buf);

	/**
	 * Move assignment (requires C++11).
	 * Closes the source filemappingbuf (as if member close was called), and then acquires the contents of rhs_buf.
	 * @param rhs_buf filemappingbuf to move. rhs_buf becomes of invalid state after the operation.
	 * @return *this.
	 * @see swap()
	 */
	filemappingbuf& operator=(filemappingbuf&& rhs_buf);

	/**
	 * Swap internals (requires C++11).
	 * Exchanges the state of the filemappingbuf with those of other.
	 * @param buf filemappingbuf to exchange the state with.
	 * @see operator=()
	 */
	void swap(filemappingbuf& buf);
	///@}
#endif // _HAS_CPP11_

	/**
	 * Check if a file is open.
	 * The function returns true if a previous call to open succeeded and there have been no calls to the member close since,
	 * meaning that the filemappingbuf object is currently associated with a file.
	 * @return true if a file is open, i.e. associated to this stream buffer object. false otherwise.
	 * @see open()
	 * @see close()
	 */
	bool is_open() const;

	/**
	 * Open file.
	 * Opens a file, associating its content with the stream buffer object to perform input/output operations on it.
	 * The operations allowed and some operating details depend on parameter mode.
	 * If the object already has a file associated (open), this function fails.
	 * If the i/o mode is input only and the file do not exist, this function fails.
	 * If the i/o mode is output and the file do not exist, the file is created with the 'offset + max_length'  size.
	 * If the size of the opened file is less than 'offset + max_length', the file growing.
	 * If the size of the opened file is greater than max_length, the file is not truncated.
	 * An attempt to map a file with a length of 0 fails.
	 * @param path_name C-string contains the name of the file to be opened.
	 * @param mode Flags describing the requested i/o mode for the file. This is an object of type std::ios_base::openmode.
	 * It consists of a combination of the following member constants:
	 *  - std::ios_base::ate (at end) Set the stream's position indicator to the end of the file on opening.
	 *  - std::ios_base::in (input)   Allow input operations on the stream.
	 *  - std::ios_base::out (output) Allow output operations on the stream.
	 * @param max_length Maximum length of the file mapping.  If this parameter is 0, the mapping extends from the specified offset to the end of the file.
	 * @param offset File offset where the mapping begins. They must also match the memory allocation granularity of the system.
	 * That is, the offset must be a multiple of the allocation granularity. To obtain the offset granularity of the system, use filemapping::offset_granularity().
	 * @return The function returns this if successful. In case of failure, close is called and a null pointer is returned.
	 * @see is_open()
	 * @see close()
	 * @see filemapping::offset_granularity()
	 */
	filemappingbuf* open(const char* path_name, std::ios_base::openmode mode, std::streamsize max_length = 0, std::streamoff offset = 0);

	/**
	 * Close file.
	 * Closes the file currently associated with the object and disassociates it.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, the function returns this. In case of failure, a null pointer is returned.
	 * @see open()
	 * @see is_open()
	 */
	filemappingbuf* close();

	/**
	 * Set internal position pointer to absolute position.
	 * Calls protected virtual member seekptr, which sets a new position value for one or both of the internal position pointers.
	 * The parameter which determines which of the position pointers is affected: either the get pointer or the put pointer, or both.
	 * The function fails if no file is currently open (associated) with this object.
	 * @param ptr New absolute position for the position pointer.
	 * @param which Determines which of the internal position pointers shall be modified: the input pointer, the output pointer, or both. This is an object of type std::ios_base::openmode.
	 * @return In case of success, return the new position value of the modified position pointer. In case of failure, a null pointer is returned.
	 * @see std::streambuf::pubseekpos()
	 * @see std::streambuf::pubseekoff()
	 */
	void* pubseekptr(void* ptr, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

 	/**
	 * Get the read-only base address of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the constant base address of the mapping. In case of failure, a null pointer is returned.
	 * @see size()
	 */
	const void* data() const;

	/**
	 * Get the base address of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the base address of the mapping. In case of failure, a null pointer is returned.
	 * @see size()
	 */
	void* data();

	/**
	 * Get the maximum byte length of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the length of the mapping. In case of failure, 0 is returned.
	 * @see data()
	 */
	std::streamsize size() const;

protected:
	virtual int sync();
	virtual std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which);
	virtual std::streampos seekpos(std::streampos sp, std::ios_base::openmode which);
	virtual void* seekptr(void* ptr, std::ios_base::openmode which);

private:
	/**
	 * Copy constructor is private: this class is not copyable.
	 */
	filemappingbuf(const filemappingbuf&);

	/**
	 * Copy operator is private: this class is not copyable.
	 */
	filemappingbuf& operator=(const filemappingbuf&);

private:
	char* m_pAddress;            //!< Base address of the mapping
	std::streamsize m_MapLength; //!< Length of the mapping
#ifdef _WIN32
	void* m_pFile;              //!< Windows handle to the file mapping object
	void* m_pFileMapping;       //!< Windows handle to the opened file
#else // If not Windows, this is a POSIX system !
	int m_fd;                    //!< File descriptor to the opened file
#endif
};

/**
 * ifmstream provides an interface to read data from memory-mapped files as input streams.
 * The objects of this class maintain internally a pointer to a filemappingbuf object that can be obtained by calling member rdbuf.
 * The file to be associated with the stream can be specified either as a parameter in the constructor or by calling member open.
 * After all necessary operations on a file have been performed, it can be closed (or disassociated) by calling member close. Once closed, the same file stream object may be used to open another file.
 * The member function is_open can be used to determine whether the stream object is currently associated with a file.
 * ifmstream can be used in place of std::ifstream.
 */
class ifmstream : public std::istream
{
public:
	/**
	 * Construct object.
	 * Constructs an object of the ifstream class.
	 * This implies the initialization of the associated filemappingbuf object and the call to the constructor of its base class with the filemappingbuf object as parameter.
	 */
	ifmstream();

	/**
	 * Construct object and open a file.
	 * Constructs an object of the ifstream class.
	 * This implies the initialization of the associated filemappingbuf object and the call to the constructor of its base class with the filemappingbuf object as parameter.
	 * The stream is associated with a physical file as if a call to the member function open with the same parameters was made.
	 * If the constructor is not successful in opening the file, the object is still created although no file is associated to the stream buffer and the stream's failbit is set (which can be checked with inherited member fail).
	 * @param path_name C-string contains the name of the file to be opened.
	 * @param max_length Maximum length of the file mapping. If this parameter are 0, the maximum length of the file mapping object is equal to the current size of the file.
	 * @param offset File offset where the mapping begins. They must also match the memory allocation granularity of the system.
	 * That is, the offset must be a multiple of the allocation granularity. To obtain the offset granularity of the system, use filemapping::offset_granularity().
	 * @see open()
	 * @see is_open()
	 * @see close()
	 * @see filemapping::offset_granularity()
	 */
	explicit ifmstream(const char* path_name, std::streamsize max_length = 0, std::streamoff offset = 0);

	/**
	 * Destructs the ifmstream object.
	 */
	virtual ~ifmstream() {}

#ifdef _HAS_CPP11_
	/** @name C++11
	 * The following methods requires some features introduced by the latest revision of the C++ standard (2011). Older compilers may not support it.
	 */
	///@{
	/**
	 * Move constructor (requires C++11).
	 * Acquires the contents of rhs_stream, by move-assigning its members and base classes.
	 * @param rhs_stream File stream to move. rhs_stream becomes of invalid state after the operation.
	 * @see swap()
	 * @see operator=()
	 */
	ifmstream(ifmstream&& rhs_stream);

	/**
	 * Move assignment (requires C++11).
	 * Closes the source stream (as if member close was called), and then acquires the contents of rhs_stream.
	 * @param rhs_stream File stream to move. rhs_stream becomes of invalid state after the operation.
	 * @return *this.
	 * @see swap()
	 */
	ifmstream& operator=(ifmstream&& rhs_stream);

	/**
	 * Swap internals (requires C++11).
	 * Exchanges the state of the stream with those of other.
	 * @param stream ifmstream to exchange the state with.
	 * @see operator=()
	 */
	void swap(ifmstream& stream);
	///@}
#endif // _HAS_CPP11_

	/**
	 * Get the associated filemappingbuf object.
	 * Returns a pointer to the filemappingbuf object associated with the stream.
	 * @return A pointer to the filemappingbuf object associated with the stream.
	 * Notice that for any successfully constructed ifmstream object this pointer is never a null pointer, even if no files have been opened or if the stream is unbuffered.
	 */
	filemappingbuf* rdbuf() const;

	/**
	 * Check if a file is open.
	 * Returns true if the stream is currently associated with a file, and false otherwise.
	 * The stream is associated with a file if either a previous call to member open succeeded or if the object was successfully constructed using the parameterized constructor, and close has not been called since.
	 * @return true if a file is open, i.e. associated to this stream object. false otherwise.
	 * @see open()
	 * @see close()
	 */
	bool is_open() const;

	/**
	 * Open file.
	 * Opens a file whose name is path_name, associating its content with the stream object to perform input/output operations on it.
	 * The operations allowed and some operating details depend on parameter mode.
	 * If the object already has a file associated (open), this function fails.
	 * If the file do not exist, this function fails.
	 * An attempt to map a file with a length of 0 fails.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @param path_name C-string contains the name of the file to be opened.
	 * @param max_length Maximum length of the file mapping. If this parameter are 0, the maximum length of the file mapping object is equal to the current size of the file.
	 * @param offset File offset where the mapping begins. They must also match the memory allocation granularity of the system.
	 * That is, the offset must be a multiple of the allocation granularity. To obtain the offset granularity of the system, use filemapping::offset_granularity().
	 * @see is_open()
	 * @see close()
	 * @see filemapping::offset_granularity()
	 */
	void open(const char* path_name, std::streamsize max_length = 0, std::streamoff offset = 0);

	/**
	 * Close file.
	 * Closes the file currently associated with the object and disassociates it.
	 * The function fails if no file is currently open (associated) with this object.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @see open()
	 * @see is_open()
	 */
	void close();

	/**
	 * Get position of the get pointer.
	 * The get pointer determines the next location in the input sequence to be read by the next input operation.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return Return the address of the get pointer. In case of failure, a null pointer is returned.
	 * @see pseekg()
	 * @see std::istream::tellg()
	 */
	const void* ptellg();

	/**
	 * Sets the position of the get pointer.
	 * The get pointer determines the next location to be read in the source associated to the stream.
	 * The function fails if no file is currently open (associated) with this object.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @param ptr New absolute position for the input pointer.
	 * @return The function returns *this.
	 * @see ptellg()
	 * @see std::istream::seekg()
	 */
	std::istream& pseekg(const void* ptr);

	/**
	 * Get the read-only base address of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the constant base address of the mapping. In case of failure, a null pointer is returned.
	 * @see size()
	 */
	const void* data() const;

	/**
	 * Get the maximum byte length of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the length of the mapping. In case of failure, 0 is returned.
	 * @see data()
	 */
	std::streamsize size() const;

private:
	/**
	 * Copy constructor is private: this class is not copyable.
	 */
	ifmstream(const ifmstream&);

	/**
	 * Copy operator is private: this class is not copyable.
	 */
	ifmstream& operator=(const ifmstream&);

private:
	filemappingbuf m_rdbuf; //!< filemappingbuf object
};

/**
 * fmstream provides an interface to read/write data from/to memory-mapped files as input/output streams.
 * The objects of this class maintain internally a pointer to a filemappingbuf object that can be obtained by calling member rdbuf.
 * The file to be associated with the stream can be specified either as a parameter in the constructor or by calling member open.
 * After all necessary operations on a file have been performed, it can be closed (or disassociated) by calling member close.
 * Once closed, the same file stream object may be used to open another file.
 * The member function is_open can be used to determine whether the stream object is currently associated with a file.
 * fmstream can be used in place of std::fstream.
 */
class fmstream : public std::iostream
{
public:
	/**
	 * Construct object.
	 * Constructs an object of the fstream class.
	 * This implies the initialization of the associated filemappingbuf object and the call to the constructor of its base class with the filemappingbuf object as parameter.
	 */
	fmstream();

	/**
	 * Construct object and open or create a file.
	 * Constructs an object of the fstream class.
	 * This implies the initialization of the associated filemappingbuf object and the call to the constructor of its base class with the filemappingbuf object as parameter.
	 * The stream is associated with a physical file as if a call to the member function open with the same parameters was made.
	 * If the constructor is not successful in opening the file, the object is still created although no file is associated to the stream buffer and the stream's failbit is set (which can be checked with inherited member fail).
	 * @param path_name C-string contains the name of the file to be opened or created.
	 * @param max_length Maximum length of the file mapping. If this parameter are 0, the maximum length of the file mapping object is equal to the current size of the file.
	 * @param offset File offset where the mapping begins. They must also match the memory allocation granularity of the system.
	 * That is, the offset must be a multiple of the allocation granularity. To obtain the offset granularity of the system, use filemapping::offset_granularity().
	 * @see open()
	 * @see is_open()
	 * @see close()
	 * @see filemapping::offset_granularity()
	 */
	explicit fmstream(const char* path_name, std::streamsize max_length = 0, std::streamoff offset = 0);

	/**
	 * Destructs the fmstream object.
	 */
	virtual ~fmstream() {}

#ifdef _HAS_CPP11_
	/** @name C++11
	 * The following methods requires some features introduced by the latest revision of the C++ standard (2011). Older compilers may not support it.
	 */
	///@{
	/**
	 * Move constructor (requires C++11).
	 * Acquires the contents of rhs_stream, by move-assigning its members and base classes.
	 * @param rhs_stream File stream to move. rhs_stream becomes of invalid state after the operation.
	 * @see swap()
	 * @see operator=()
	 */
	fmstream(fmstream&& rhs_stream);

	/**
	 * Move assignment (requires C++11).
	 * Closes the source stream (as if member close was called), and then acquires the contents of rhs_stream.
	 * @param rhs_stream File stream to move. rhs_stream becomes of invalid state after the operation.
	 * @return *this.
	 * @see swap()
	 */
	fmstream& operator=(fmstream&& rhs_stream);

	/**
	 * Swap internals (requires C++11).
	 * Exchanges the state of the stream with those of other.
	 * @param stream fmstream to exchange the state with.
	 * @see operator=()
	 */
	void swap(fmstream& stream);
	///@}
#endif // _HAS_CPP11_

	/**
	 * Get the associated filemappingbuf object.
	 * Returns a pointer to the filemappingbuf object associated with the stream.
	 * @return A pointer to the filemappingbuf object associated with the stream.
	 * Notice that for any successfully constructed fmstream object this pointer is never a null pointer, even if no files have been opened or if the stream is unbuffered.
	 */
	filemappingbuf* rdbuf() const;

	/**
	 * Check if a file is open.
	 * Returns true if the stream is currently associated with a file, and false otherwise.
	 * The stream is associated with a file if either a previous call to member open succeeded or if the object was successfully constructed using the parameterized constructor, and close has not been called since.
	 * @return true if a file is open, i.e. associated to this stream object. false otherwise.
	 * @see open()
	 * @see close()
	 */
	bool is_open() const;

	/**
	 * Open file.
	 * Opens a file whose name is path_name, associating its content with the stream object to perform input/output operations on it.
	 * The operations allowed and some operating details depend on parameter mode.
	 * If the object already has a file associated (open), this function fails.
	 * If the file do not exist, the file is created with the 'offset + max_length' size.
	 * If the size of the opened file is less than 'offset + max_length', the file growing.
	 * If the size of the opened file is greater than 'offset + max_length', the file is not truncated.
	 * An attempt to map a file with a length of 0 fails.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @param path_name C-string contains the name of the file to be opened.
	 * @param max_length Maximum length of the file mapping. If this parameter are 0, the maximum length of the file mapping object is equal to the current size of the file.
	 * @param offset File offset where the mapping begins. They must also match the memory allocation granularity of the system.
	 * That is, the offset must be a multiple of the allocation granularity. To obtain the offset granularity of the system, use filemapping::offset_granularity().
	 * @see is_open()
	 * @see close()
	 * @see filemapping::offset_granularity()
	 */
	void open(const char* path_name, std::streamsize max_length = 0, std::streamoff offset = 0);

	/**
	 * Close file.
	 * Closes the file currently associated with the object and disassociates it.
	 * The function fails if no file is currently open (associated) with this object.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @see open()
	 * @see is_open()
	 */
	void close();

	/**
	 * Get position of the get pointer.
	 * The get pointer determines the next location in the input sequence to be read by the next input operation.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return Return the address of the get pointer. In case of failure, a null pointer is returned.
	 * @see ptellp()
	 * @see pseekg()
	 * @see std::istream::tellg()
	 */
	const void* ptellg();

	/**
	 * Get position of the put pointer.
	 * The put pointer determines the location in the output sequence where the next output operation is going to take place.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return Return the address of the put pointer. In case of failure, a null pointer is returned.
	 * @see ptellg()
	 * @see pseekp()
	 * @see std::istream::tellp()
	 */
	void* ptellp();

	/**
	 * Sets the position of the get pointer.
	 * The get pointer determines the next location to be read in the source associated to the stream.
	 * The function fails if no file is currently open (associated) with this object.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @param ptr New absolute position for the input pointer.
	 * @return The function returns *this.
	 * @see pseekp()
	 * @see ptellg()
	 * @see std::istream::seekg()
	 */
	std::istream& pseekg(const void* ptr);

	/**
	 * Sets the position of the put pointer.
	 * The put pointer determines the location in the output sequence where the next output operation is going to take place.
	 * The function fails if no file is currently open (associated) with this object.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @param ptr New absolute position for the output pointer.
	 * @return The function returns *this.
	 * @see pseekg()
	 * @see ptellp()
	 * @see std::istream::seekp()
	 */
	std::ostream& pseekp(void* ptr);

	/**
	 * Get the read-only base address of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the constant base address of the mapping. In case of failure, a null pointer is returned.
	 * @see size()
	 */
	const void* data() const;

	/**
	 * Get the base address of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the base address of the mapping. In case of failure, a null pointer is returned.
	 * @see size()
	 */
	void* data();

	/**
	 * Get the maximum byte length of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the length of the mapping. In case of failure, 0 is returned.
	 * @see data()
	 */
	std::streamsize size() const;

private:
	/**
	 * Copy constructor is private: this class is not copyable.
	 */
	fmstream(const fmstream&);

	/**
	 * Copy operator is private: this class is not copyable.
	 */
	fmstream& operator=(const fmstream&);

private:
	filemappingbuf m_rdbuf; //!< filemappingbuf object
};

#ifdef _HAS_CPP11_
/** @name C++11
 * The following methods requires some features introduced by the latest revision of the C++ standard (2011). Older compilers may not support it.
 */
///@{
/**
 * Swap two filemappingbuf (requires C++11).
 * Overloads the std::swap algorithm for filemappingbuf. Exchanges the state of lhs with that of rhs.
 * Effectively calls lhs.swap(rhs).
 * @param lhs filemappingbuf to exchange the state with.
 * @param rhs filemappingbuf to exchange the state with.
 */
void swap(filemappingbuf& lhs, filemappingbuf& rhs);

/**
 * Swap two ifmstream (requires C++11).
 * Overloads the std::swap algorithm for ifmstream. Exchanges the state of lhs with that of rhs.
 * Effectively calls lhs.swap(rhs).
 * @param lhs ifmstream to exchange the state with.
 * @param rhs ifmstream to exchange the state with.
 */
void swap(ifmstream& lhs, ifmstream& rhs);

/**
 * Swap two fmstream (requires C++11).
 * Overloads the std::swap algorithm for fmstream. Exchanges the state of lhs with that of rhs.
 * Effectively calls lhs.swap(rhs).
 * @param lhs fmstream to exchange the state with.
 * @param rhs fmstream to exchange the state with.
 */
void swap(fmstream& lhs, fmstream& rhs);
///@}
#endif // _HAS_CPP11_

#endif /* FILE_MAPPING_STREAM_H_ */


//#line 1 "fmstream.cpp"
#ifdef _WIN32
#include <windows.h>
#else
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

std::streamoff filemapping::offset_granularity()
{
#ifdef _WIN32
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	return static_cast<std::streamoff>(SystemInfo.dwAllocationGranularity);
#else // If not Windows, this is a POSIX system !
	return static_cast<std::streamoff>(sysconf(_SC_PAGE_SIZE));
#endif
}

filemappingbuf::filemappingbuf() :
	m_pAddress(NULL),
	m_MapLength(0),
#ifdef _WIN32
	m_pFile(INVALID_HANDLE_VALUE),
	m_pFileMapping(NULL)
#else // If not Windows, this is a POSIX system !
	m_fd(-1)
#endif
{
}

filemappingbuf::~filemappingbuf()
{
	close();
}

#ifdef _HAS_CPP11_
filemappingbuf::filemappingbuf(filemappingbuf&& rhs_buf) :
	m_pAddress(NULL),
	m_MapLength(0),
#ifdef _WIN32
	m_pFile(INVALID_HANDLE_VALUE),
	m_pFileMapping(NULL)
#else // If not Windows, this is a POSIX system !
	m_fd(-1)
#endif
{
	swap(rhs_buf);
}

filemappingbuf& filemappingbuf::operator=(filemappingbuf&& rhs_buf)
{
	if(this != &rhs_buf)
	{
		close();
		swap(rhs_buf);
	}

	return *this;
}

void filemappingbuf::swap(filemappingbuf& buf)
{
	if(this != &buf)
	{
		std::streambuf::swap(buf);
		std::swap(m_pAddress, buf.m_pAddress);
		std::swap(m_MapLength, buf.m_MapLength);
#ifdef _WIN32
		std::swap(m_pFile, buf.m_pFile);
		std::swap(m_pFileMapping, buf.m_pFileMapping);
#else // If not Windows, this is a POSIX system !
		std::swap(m_fd, buf.m_fd);
#endif
	}
}

void swap(filemappingbuf& lhs, filemappingbuf& rhs)
{
	lhs.swap(rhs);
}
#endif // _HAS_CPP11_

bool filemappingbuf::is_open() const
{
	return (m_pAddress && m_MapLength);
}

void* filemappingbuf::pubseekptr(void* ptr, std::ios_base::openmode which)
{
	if(!is_open())
		return NULL;

	return seekptr(ptr, which);
}

std::streampos filemappingbuf::seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which)
{
	switch(way)
	{
	case std::ios_base::beg:
		break;

	case std::ios_base::cur:
		if(which & std::ios_base::in)
			off += static_cast<std::streamoff>(gptr() - m_pAddress);
		else
			off += static_cast<std::streamoff>(pptr() - m_pAddress);
		break;

	case std::ios_base::end:
		off = m_MapLength - off;
		break;

	default:
		return -1;
	}

	if(off < 0)
		return -1;

	return seekpos(off, which);
}

std::streampos filemappingbuf::seekpos(std::streampos sp, std::ios_base::openmode which)
{
	if(sp < 0 || !seekptr(m_pAddress + static_cast<ptrdiff_t>(sp), which))
		return -1;

	return sp;
}

void* filemappingbuf::seekptr(void* ptr, std::ios_base::openmode which)
{
	char* pcPtr = static_cast<char*>(ptr);

	if((which & std::ios_base::in) && pcPtr != gptr())
	{
		if(pcPtr >= m_pAddress && pcPtr < egptr())
			setg(eback(), pcPtr, egptr());
		else
			return NULL;
	}

	if((which & std::ios_base::out) && pcPtr != pptr())
	{
		if(pcPtr >= m_pAddress && pcPtr < epptr())
			setp(pcPtr, epptr());
		else
			return NULL;
	}

	return ptr;
}

filemappingbuf* filemappingbuf::open(const char* path_name, std::ios_base::openmode mode, std::streamsize max_length, std::streamoff offset)
{
	if(is_open() || max_length < 0 || offset < 0) // Check if a file is already opened and parameters
		return NULL;

	std::streamsize FileSize = 0;

#ifdef _WIN32
	DWORD dwDesiredAccess = GENERIC_READ;
	DWORD flProtect = PAGE_READONLY;
	DWORD dwMapAccess = FILE_MAP_READ;
	DWORD dwCreationDisposition = OPEN_EXISTING;
	DWORD dwFileSizeHigh = 0;
	DWORD dwFileOffsetHigh = 0;

	if(mode & std::ios_base::out)
	{
		dwDesiredAccess |= GENERIC_WRITE;
		flProtect = PAGE_READWRITE;
		dwMapAccess |= FILE_MAP_WRITE;
		dwCreationDisposition = OPEN_ALWAYS;
	}

	m_pFile = CreateFileA(path_name, dwDesiredAccess, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_pFile == INVALID_HANDLE_VALUE)
	{
		filemappingbuf::close();
		return NULL;
	}

	FileSize = GetFileSize(m_pFile, &dwFileSizeHigh);
#ifdef _WIN64
	FileSize |= static_cast<std::streamsize>(dwFileSizeHigh) << 32;
#endif

	if(max_length)
		m_MapLength = max_length;
	else
		m_MapLength = FileSize - static_cast<std::streamsize>(offset);

	if(!(mode & std::ios_base::out) && (static_cast<std::streamsize>(offset) + m_MapLength) > FileSize)
		m_MapLength = FileSize - static_cast<std::streamsize>(offset);

	if(m_MapLength <= 0)
	{
		filemappingbuf::close();
		return NULL;
	}

	std::streamsize NewFileSize = static_cast<std::streamsize>(offset) + m_MapLength;

#ifdef _WIN64
	dwFileSizeHigh = static_cast<DWORD>(NewFileSize >> 32);
	dwFileOffsetHigh = static_cast<DWORD>(offset >> 32);
#else
	dwFileSizeHigh = 0;
#endif

	m_pFileMapping = CreateFileMappingA(m_pFile, NULL, flProtect, dwFileSizeHigh, static_cast<DWORD>(NewFileSize), NULL);
	if(!m_pFileMapping)
	{
		filemappingbuf::close();
		return NULL;
	}

	m_pAddress = static_cast<char*>(MapViewOfFile(m_pFileMapping, dwMapAccess, dwFileOffsetHigh, static_cast<DWORD>(offset), static_cast<SIZE_T>(m_MapLength)));
	if(!m_pAddress)
	{
		filemappingbuf::close();
		return NULL;
	}

#else // If not Windows, this is a POSIX system !
	int oflag = O_RDONLY;
	int flags = PROT_READ;
	mode_t ar = 0;

	if(mode & std::ios_base::out)
	{
		oflag = O_RDWR | O_CREAT;
		flags |= PROT_WRITE;
		ar = S_IRUSR | S_IWUSR;
	}

	m_fd = ::open(path_name, oflag, ar);
	if(m_fd == -1)
		return NULL;

	struct stat statbuf;

	// Get the file size
	if(fstat(m_fd, &statbuf) != 0)
	{
		filemappingbuf::close();
		return NULL;
	}

	FileSize = statbuf.st_size;

	if(max_length)
		m_MapLength = max_length;
	else
		m_MapLength = FileSize - static_cast<std::streamsize>(offset);

	if(static_cast<std::streamsize>(offset) + m_MapLength > FileSize)
	{
		if(mode & std::ios_base::out)
		{
			/* Something needs to be written at the end of the file to
			 * have the file actually have the new size.
			 * Just writing an empty string at the current file position will do.
			 *
			 * Note:
			 *  - The current position in the file is at the end of the stretched
			 *    file due to the call to lseek().
			 *  - An empty string is actually a single '\0' character, so a zero-byte
			 *    will be written at the last byte of the file.
			 */
			if(lseek(m_fd, static_cast<std::streamsize>(offset) + m_MapLength - 1, SEEK_SET) == -1)
			{
				filemappingbuf::close();
				return NULL;
			}

			if(write(m_fd, "", 1) != 1)
			{
				filemappingbuf::close();
				return NULL;
			}
		}
		else
		{
			m_MapLength = FileSize - static_cast<std::streamsize>(offset);

			if(m_MapLength <= 0)
			{
				filemappingbuf::close();
				return NULL;
			}
		}
	}

	m_pAddress = static_cast<char*>(mmap(NULL, static_cast<size_t>(m_MapLength), flags, MAP_SHARED, m_fd, offset));
	if(m_pAddress == MAP_FAILED)
	{
		filemappingbuf::close();
		return NULL;
	}
#endif

	char* pEnd = m_pAddress + static_cast<ptrdiff_t>(m_MapLength);

	setg(m_pAddress, m_pAddress, pEnd);

	if(mode & std::ios_base::ate) // At end
		setp(m_pAddress + static_cast<ptrdiff_t>(FileSize - offset), pEnd);
	else
		setp(m_pAddress, pEnd);

	return this;
}

int filemappingbuf::sync()
{
	int nRet = -1;

#ifdef _WIN32
	if(m_pAddress && m_MapLength)
		nRet = (FlushViewOfFile(m_pAddress, static_cast<SIZE_T>(m_MapLength)) != FALSE)? 0: -1;
#else // If not Windows, this is a POSIX system !
	if(m_pAddress && m_MapLength)
		nRet = msync(m_pAddress, static_cast<size_t>(m_MapLength), MS_ASYNC);
#endif

	return nRet;
}

const void* filemappingbuf::data() const
{
	return m_pAddress;
}

void* filemappingbuf::data()
{
	return m_pAddress;
}

std::streamsize filemappingbuf::size() const
{
	return m_MapLength;
}

filemappingbuf* filemappingbuf::close()
{
#ifdef _WIN32
	if(m_pAddress)
		UnmapViewOfFile(m_pAddress);

	if(m_pFileMapping)
	{
		CloseHandle(m_pFileMapping);
		m_pFileMapping = NULL;
	}

	if(m_pFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_pFile);
		m_pFile = INVALID_HANDLE_VALUE;
	}
#else // If not Windows, this is a POSIX system !
	if(m_pAddress && m_MapLength)
		munmap(m_pAddress, static_cast<size_t>(m_MapLength));

	if(m_fd != -1)
	{
		::close(m_fd);
		m_fd = -1;
	}
#endif

	if(!is_open())
		return NULL;

	m_pAddress = NULL;
	m_MapLength = 0;

	setg(NULL, NULL, NULL);
	setp(NULL, NULL);

	return this;
}

/* ifmstream class */
ifmstream::ifmstream() : std::istream(&m_rdbuf)
{}

ifmstream::ifmstream(const char* path_name, std::streamsize max_length, std::streamoff offset) : std::istream(&m_rdbuf)
{
	open(path_name, max_length, offset);
}

#ifdef _HAS_CPP11_
ifmstream::ifmstream(ifmstream&& rhs_stream) : std::istream(&m_rdbuf)
{
	swap(rhs_stream);
}

ifmstream& ifmstream::operator=(ifmstream&& rhs_stream)
{
	if(this != &rhs_stream)
	{
		m_rdbuf.close();
		swap(rhs_stream);
	}

	return *this;
}

void ifmstream::swap(ifmstream& stream)
{
	if(this != &stream)
	{
		std::istream::swap(stream);
		m_rdbuf.swap(stream.m_rdbuf);
	}
}

void swap(ifmstream& lhs, ifmstream& rhs)
{
	lhs.swap(rhs);
}
#endif // _HAS_CPP11_

filemappingbuf* ifmstream::rdbuf() const
{
	return const_cast<filemappingbuf*>(&m_rdbuf);
}

bool ifmstream::is_open() const
{
	return m_rdbuf.is_open();
}

void ifmstream::open(const char* path_name, std::streamsize max_length, std::streamoff offset)
{
	if(m_rdbuf.open(path_name, std::ios_base::in, max_length, offset) == NULL)
		setstate(std::ios_base::failbit);
	else
		clear();
}

void ifmstream::close()
{
	if(m_rdbuf.close() == NULL)
		setstate(std::ios_base::failbit);
}

const void* ifmstream::ptellg()
{
	// Return input stream position
	if(!fail())
		return static_cast<char*>(m_rdbuf.data()) + static_cast<ptrdiff_t>(m_rdbuf.pubseekoff(0, std::ios_base::cur, std::ios_base::in));
	else
		return NULL;
}

std::istream& ifmstream::pseekg(const void* ptr)
{
	// Set input stream position to ptr
	if(!fail() && !m_rdbuf.pubseekptr(const_cast<void*>(ptr), ios_base::in))
		setstate(std::ios_base::failbit);
	return *this;
}

const void* ifmstream::data() const
{
	if(!fail())
		return m_rdbuf.data();
	else
		return NULL;
}

std::streamsize ifmstream::size() const
{
	if(!fail())
		return m_rdbuf.size();
	else
		return 0;
}

/* fmstream class */
fmstream::fmstream() : std::iostream(&m_rdbuf)
{}

fmstream::fmstream(const char* path_name, std::streamsize max_length, std::streamoff offset) : std::iostream(&m_rdbuf)
{
	open(path_name, max_length, offset);
}

#ifdef _HAS_CPP11_
fmstream::fmstream(fmstream&& rhs_stream) : std::iostream(&m_rdbuf)
{
	swap(rhs_stream);
}

fmstream& fmstream::operator=(fmstream&& rhs_stream)
{
	if(this != &rhs_stream)
	{
		m_rdbuf.close();
		swap(rhs_stream);
	}

	return *this;
}

void fmstream::swap(fmstream& stream)
{
	if(this != &stream)
	{
		std::iostream::swap(stream);
		m_rdbuf.swap(stream.m_rdbuf);
	}
}

void swap(fmstream& lhs, fmstream& rhs)
{
	lhs.swap(rhs);
}
#endif // _HAS_CPP11_

filemappingbuf* fmstream::rdbuf() const
{
	return const_cast<filemappingbuf*>(&m_rdbuf);
}

bool fmstream::is_open() const
{
	return m_rdbuf.is_open();
}

void fmstream::open(const char* path_name, std::streamsize max_length, std::streamoff offset)
{
	if(m_rdbuf.open(path_name, std::ios_base::in | std::ios_base::out, max_length, offset) == NULL)
		setstate(std::ios_base::failbit);
	else
		clear();
}

void fmstream::close()
{
	if(m_rdbuf.close() == NULL)
		setstate(std::ios_base::failbit);
}

const void* fmstream::ptellg()
{
	// Return input stream position
	if(!fail())
		return static_cast<char*>(m_rdbuf.data()) + static_cast<ptrdiff_t>(m_rdbuf.pubseekoff(0, std::ios_base::cur, std::ios_base::in));
	else
		return NULL;
}

void* fmstream::ptellp()
{
	// Return input stream position
	if(!fail())
		return static_cast<char*>(m_rdbuf.data()) + static_cast<ptrdiff_t>(m_rdbuf.pubseekoff(0, std::ios_base::cur, std::ios_base::out));
	else
		return NULL;
}

std::istream& fmstream::pseekg(const void* ptr)
{
	// Set input stream position to ptr
	if(!fail() && !m_rdbuf.pubseekptr(const_cast<void*>(ptr), std::ios_base::in))
		setstate(std::ios_base::failbit);
	return *this;
}

std::ostream& fmstream::pseekp(void* ptr)
{
	// Set output stream position to ptr
	if(!fail() && !m_rdbuf.pubseekptr(ptr, std::ios_base::out))
		setstate(std::ios_base::failbit);
	return *this;
}

const void* fmstream::data() const
{
	if(!fail())
		return m_rdbuf.data();
	else
		return NULL;
}

void* fmstream::data()
{
	if(!fail())
		return m_rdbuf.data();
	else
		return NULL;
}

std::streamsize fmstream::size() const
{
	if(!fail())
		return m_rdbuf.size();
	else
		return 0;
}

#ifdef _MSC_VER
#   ifndef _CRT_SECURE_NO_WARNINGS
#       define _CRT_SECURE_NO_WARNINGS
#   endif
#   pragma warning(disable: 4996)
#endif

// api

namespace apathy
{
	namespace detail
	{
		std::deque<std::string> split( const std::string &str, char sep )
		{
			std::deque<std::string> tokens;
			tokens.push_back( std::string() );

			for( std::string::const_iterator it = str.begin(), end = str.end(); it != end; ++it )
			{
				if( *it == sep )
				{
					tokens.push_back( std::string() + sep );
					tokens.push_back( std::string() );
				}
				else
				{
					tokens.back() += *it;
				}
			}

			return tokens;
		}

		class sbb : public std::streambuf
		{
			public:

			typedef void (*proc)( bool open, bool feed, bool close, const std::string &text );
			typedef std::set< proc > set;
			set cb;

			sbb()
			{}

			sbb( const sbb &other ) {
				operator=(other);
			}

			sbb &operator=( const sbb &other ) {
				if( this != &other ) {
					cb = other.cb;
				}
				return *this;
			}

			sbb( void (*cbb)( bool, bool, bool, const std::string & ) ) {
				insert( cbb );
			}

			~sbb() {
				clear();
			}

			void log( const std::string &line ) {
				if( !line.size() )
					return;

				std::deque<std::string> lines = split( line, '\n' );

				for( set::iterator jt = cb.begin(), jend = cb.end(); jt != jend; ++jt )
					for( std::deque<std::string>::iterator it = lines.begin(), end = lines.end(); it != end; ++it )
					{
						if( *it != "\n" )
							(**jt)( false, false, false, *it );
						else
							(**jt)( false, true, false, std::string() );
					}
			}

			virtual int_type overflow( int_type c = traits_type::eof() ) {
				return log( std::string() + (char)(c) ), 1;
			}

			virtual std::streamsize xsputn( const char *c_str, std::streamsize n ) {
				return log( std::string( c_str, (unsigned)n ) ), n;
			}

			void clear() {
				for( const auto &jt : cb ) {
					(*jt)( false, false, true, std::string() );
				}
				cb.clear();
			}

			void insert( proc p ) {
				if( !p )
					return;

				// make a dummy call to ensure any static object of this callback are deleted after ~sbb() call (RAII)
				p( 0, 0, 0, std::string() );
				p( true, false, false, std::string() );

				// insert into map
				cb.insert( p );
			}

			void erase( proc p ) {
				p( false, false, true, std::string() );
				cb.erase( p );
			}
		};
	}
}

namespace
{
	struct captured_ostream {
		captured_ostream() : copy(0) {}
		std::streambuf *copy;
		apathy::detail::sbb sb;
	};

	std::map< std::ostream *, captured_ostream > loggers;
}

namespace apathy
{
namespace ostream
{
	void attach( std::ostream &_os, void (*custom_stream_callback)( bool open, bool feed, bool close, const std::string &line ) )
	{
		std::ostream *os = &_os;

		( loggers[ os ] = loggers[ os ] );

		if( !loggers[ os ].copy )
		{
			// capture ostream
			loggers[ os ].copy = os->rdbuf( &loggers[ os ].sb );
		}

		loggers[ os ].sb.insert( custom_stream_callback );
	}

	void detach( std::ostream &_os, void (*custom_stream_callback)( bool open, bool feed, bool close, const std::string &line ) )
	{
		std::ostream *os = &_os;

		attach( _os, custom_stream_callback );

		loggers[ os ].sb.erase( custom_stream_callback );

		if( !loggers[ os ].sb.cb.size() )
		{
			// release original stream
			os->rdbuf( loggers[ os ].copy );
		}
	}

	void detach( std::ostream &_os )
	{
		std::ostream *os = &_os;

		( loggers[ os ] = loggers[ os ] ).sb.clear();

		// release original stream
		os->rdbuf( loggers[ os ].copy );
	}

	std::ostream &make( void (*proc)( bool open, bool feed, bool close, const std::string &line ) )
	{
		static struct container
		{
			std::map< void (*)( bool open, bool feed, bool close, const std::string &text ), apathy::detail::sbb > map;
			std::vector< std::ostream * > list;

			container()
			{}

			~container()
			{
				for( std::vector< std::ostream * >::const_iterator
						it = list.begin(), end = list.end(); it != end; ++it )
					delete *it;
			}

			std::ostream &insert( void (*proc)( bool open, bool feed, bool close, const std::string &text ) )
			{
				( map[ proc ] = map[ proc ] ) = apathy::detail::sbb(proc);

				list.push_back( new std::ostream( &map[proc] ) );
				return *list.back();
			}
		} _;

		return _.insert( proc );
	}
} // ostream::
} // apathy::

//--------------

namespace {
	/*
	void sleep( double seconds ) {
		std::chrono::microseconds duration( (int)(seconds * 1000000) );
		std::this_thread::sleep_for( duration );
	}*/
	/*
	std::string replace( const std::string &on, const std::string &find, const std::string &repl ) {
		std::string s = on;
		for( auto found = std::string::npos - std::string::npos; ( found = s.find( find, found ) ) != std::string::npos ; ) {
			s.replace( found, find.length(), repl );
			found += repl.length();
		}
		return s;
	}
	*/
	/*
	std::string sanitize( const std::string &path ) {
		$win32( std::string from = "/", to = "\\" );
		$welse( std::string from = "\\", to = "/" );
		return replace(path, from, to );
	} */
	const std::string get_temp_path() {
		// \st823.1 becomes .\st823.1, or w:\users\user9612\temp~1\st823.1
		$win32( static const std::string temp = getenv("TEMP") ? getenv("TEMP") : ( getenv("TMP") ? getenv("TMP") : "" ) );
		$welse( static const std::string temp );
		return temp;
	}
	/*
	const char &at( const std::string &self, signed pos ) {
		signed size = (signed)(self.size());
		if( size )
			return self[ pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) ];
		static std::map< const std::string *, char > map;
		return ( ( map[ &self ] = map[ &self ] ) = '\0' );
	}
	char &at( std::string &self, signed pos ) {
		signed size = (signed)(self.size());
		if( size )
			return self[ pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) ];
		static std::map< const std::string *, char > map;
		return ( ( map[ &self ] = map[ &self ] ) = '\0' );
	} */
	std::vector< std::string > tokenize( const std::string &self, const std::string &delimiters ) {
		std::string map( 256, '\0' );
		for( auto &ch : delimiters )
			map[ ch ] = '\1';
		std::vector< std::string > tokens(1);
		for( const unsigned char &ch : self ) {
			/**/ if( !map.at(ch)          ) tokens.back().push_back( ch );
			else if( tokens.back().size() ) tokens.push_back( std::string() );
		}
		while( tokens.size() && !tokens.back().size() ) tokens.pop_back();
		return tokens;
	}
	bool matches( const std::string &text, const std::string &pattern ) {
		struct local {
			static bool match( const char *pattern, const char *str ) {
				if( *pattern=='\0' ) return !*str;
				if( *pattern=='*' )  return match(pattern+1, str) || (*str && match(pattern, str+1));
				if( *pattern=='?' )  return *str && (*str != '.') && match(pattern+1, str+1);
				return (*str == *pattern) && match(pattern+1, str+1);
		} };
		return local::match( pattern.c_str(), text.c_str() );
	}
	bool recurse( apathy::folder &self, const std::string &sDir, const std::vector<std::string> &masks, bool recursive ) {
		apathy::file path( sDir );
		if( path.is_file() ) {
			self.insert( path );
			return false;
		}
		$win32(
			WIN32_FIND_DATAA fdFile;
			std::string spath = sDir + "\\*"; //sDir;
			HANDLE hFind = FindFirstFileA(spath.c_str(), &fdFile);

			if( hFind == INVALID_HANDLE_VALUE )
				return /*"path not found", */false;

			do {
				// Ignore . and .. folders
				if( !strcmp(fdFile.cFileName, ".") )
					continue;
				if( !strcmp(fdFile.cFileName, "..") )
					continue;

				// Rebuild path
				spath = std::string(sDir) + "\\" + std::string(fdFile.cFileName); //= std::string(fdFile.cFileName);

				// Scan recursively if needed
				if( !(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
					for( auto &mask : masks ) {
						if( matches(spath, mask) ) {
							self.insert( apathy::file( spath ) );
							break;
						}
					}
				}
				else {
					if( !recursive ) {
						for( auto &mask : masks )
							if( matches(spath, mask) ) {
								self.insert( apathy::file( spath ) );
								break;
							}
					}
					else {
						recurse( self, spath, masks, recursive );
					}
				}
			}
			while( FindNextFileA( hFind, &fdFile ) );

			FindClose( hFind );
			return true;
		)
		$welse(
			for( auto &mask : masks ) {
				FILE *fp;
				std::string out;
				/**/ if( recursive) fp = popen( (std::string()+"find "+sDir+" -type d -or -type f -name '"+mask+"'" ).c_str(), "r" );
				else                fp = popen( (std::string()+"find "+sDir+" -maxdepth 1 -type d -or -type f -name '"+mask+"'" ).c_str(), "r" );
				if( fp ) {
					while( !feof(fp) ) out += (unsigned char)(fgetc(fp));
					fclose(fp);
				}
				auto split = tokenize( out, "\n\r" );
				for( auto &found : split ) {
					apathy::file entry( found );
					if( entry.exists() )
						self.insert( entry );
				}
			}
			return true;
		)
	}
	bool rmrf( const std::string &path, bool subdirs ) {
		/*
		if( path.empty() ) return true;
		if( path.back() != '/' ) path += "/";

		for( DIR *pdir = opendir( path.c_str() ); 0 != pdir; closedir( pdir ) ) {
			for( struct dirent *pent = 0; pent = readdir( pdir ); ) {
				std::string name = pent->name;
				if( name == "." || name == ".." ) continue;
				name = path + name;
				if( apathy::file( name ).is_file() ) {
					apathy::file( name ).remove();
				} else {
					if( subdirs ) {
						rmrf( name, subdirs );
					}
				}
			}
		}
		*/
		return 0 == $win32( _rmdir ) $welse( rmdir ) ( path.c_str() ) ? true : false;
	}
}

namespace apathy
{
	namespace {
		std::map< std::string, fmstream > *chunks = 0;
	}

	file::file( const std::string &_pathfile ) :
		is_temp_name( _pathfile.size() ? false : true ),
		pathfile( _pathfile.size() ? _pathfile : std::tmpnam(0) )
	{
		static bool once = true;
		if( once ) {
			once = false;
			static std::map< std::string, fmstream > map;
			chunks = &map;
			chunk(0,0);
		}

		if( is_temp_name )
		{
			pathfile = get_temp_path() + pathfile;
		}

		// metadata here
		// ie, committed_by, date_created, last_open, last_access, vertex_info, bitrate, etc

		// valid prefixes:
		// b_ = bool
		// d_ = double
		// s_ = std::string
		// z_ = size_t
	}

	file::~file() {
		munmap();
		if( is_temp_name )
			remove();
	}

	std::string file::name() const {
		return pathfile;
	}

	std::string file::ext() const {
		std::string n = name();

		auto npos = n.end();

		for( auto it = n.begin(), end = n.end(); it != end; ++it )
		{
			if( *it == '.' )
				npos = ++it;
			else
			if( *it == '/' || *it == '\\' )
				npos = n.end();
		}

		return npos == n.end() ? std::string() : &n.c_str()[ npos - n.begin() ];
	}

	std::string file::path() const {
		return pathfile;
	}

	bool file::matches( const std::string &wildcard ) const {
		return ::matches( pathfile, wildcard );
	}

	size_t file::size() const {
		//should we return std::streamoff instead and avoid casting?
		std::ifstream is( pathfile, std::ios::in|std::ios::binary|std::ios::ate );

		//is.seekg(0, std::ios::end);
		size_t length = static_cast<size_t>( is.tellg() );
		is.seekg(0, std::ios::beg);
		length -= static_cast<size_t>( is.tellg() );

		return length;
	}

	std::string file::read() const {
		size_t length = size();

		std::string buffer;

		if( length > 0 )
		{
			buffer.resize( length );

			std::ifstream is( pathfile, std::ios::in|std::ios::binary );
			is.read( reinterpret_cast< char * >( &buffer[0] ), length );

			// std::ifstream ifs( pathfile, std::ios::in|std::ios::binary );
			// buffer = std::string( std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() );

			//std::ifstream is( pathfile, std::ios::in | std::ios::binary );
			//std::stringstream buffer;
			//buffer << is.rdbuf();
			//return buffer.str();
		}

		return buffer;
	}

	bool file::overwrite( const std::string &content ) const {
		return overwrite( content.c_str(), content.size() );
	}

	bool file::overwrite( const void *data, size_t size ) const
	{
		// is trunc flag needed?
		std::ofstream is( pathfile, std::ios::out|std::ios::binary|std::ios::trunc );

		if( is.is_open() && is.good() )
			is.write( reinterpret_cast<const char *>( data ), size );

		return !is.fail();
	}

	bool file::append( const std::string &content ) const {
		return append( content.c_str(), content.size() );
	}

	bool file::append( const void *data, size_t size ) const {
		std::fstream is( pathfile, std::ios::out|std::ios::binary|std::ios::app|std::ios::ate );

		if( is.is_open() && is.good() )
			is.write( reinterpret_cast<const char *>( data ), size );

		return !is.fail();
	}

	bool file::remove() const {
		for( int i = 0; i < 512; ++i ) {
			if( !exists() )
				return true;
			if( is_file() ) {
				if( !std::remove( pathfile.c_str() ) ) {
					return true;
				}
			} else {
				if( rmrf( pathfile, false ) ) {
					return true;
				}
			}
			sleep(0.001);
		}
		return false;
	}

	bool file::rename( const std::string &new_pathfile ) {
		for( int i = 0; i < 512; ++i ) {
			if( !exists() )
				return false;
			if( !std::rename( pathfile.c_str(), new_pathfile.c_str() ) )
				{ pathfile = new_pathfile; return true; }
			sleep(0.001);
		}
		return false;
	}

	bool file::exists() const { // may fail due to permissions (check errno)
		// struct stat fileInfo;
		// return stat( pathfile.c_str(), &fileInfo ) == 0 ? true : false;
#ifdef _WIN32
		return 0 == _access(pathfile.c_str(), 0);
#else
		return 0 == access(pathfile.c_str(), F_OK);
#endif
	}

	bool file::is_dir() const { // may return false due to permissions (check errno)
#ifdef _WIN32
		return FALSE != PathIsDirectory( pathfile.c_str() );
#else
		struct stat fileInfo;
		if( stat( pathfile.c_str(), &fileInfo ) < 0 )
			return false;
#ifdef S_ISDIR
		return ( S_ISDIR(fileInfo.st_mode) ) != 0;
#else
		return (( fileInfo.st_mode & S_IFMT ) == S_IFDIR );
#endif
#endif
	}

	bool file::is_file() const { // may return true due to permissions (check errno)
		return !is_dir();
	}

	bool file::has_data() const {
		return exists() && is_file() && size() > 0;
	}

	time_t file::date() const {
		struct stat fileInfo;
		if( stat( pathfile.c_str(), &fileInfo ) < 0 )
			return ::time(0);
		return fileInfo.st_mtime;
	}

	std::string file::timestamp( const char *format ) const { // defaults to mysql date format
		struct stat fileInfo;
		time_t mtime = stat( pathfile.c_str(), &fileInfo ) < 0 ? (::time( 0 )) : fileInfo.st_mtime;

		struct tm *ts = localtime( &mtime );

		char buffer[64];
		strftime(buffer, sizeof(buffer), format, ts);
		return buffer;
	}

	bool file::touch( const time_t &modtime ) { // const // may fail due to sharing violations (check errno)
		struct stat fileInfo;
		if( stat( pathfile.c_str(), &fileInfo ) < 0 )
			return false;

		struct utimbuf tb;
		tb.actime = fileInfo.st_atime;  /* keep atime unchanged */
		tb.modtime = modtime;           /* set mtime to current time */

		return utime( pathfile.c_str(), &tb ) != -1 ? true : false;
	}

	bool file::has_changed() { // check for external changes on file in runtime, first call on a file is always true
		static std::map< std::string, time_t > cache;
		if( cache.find( pathfile ) == cache.end() ) {
			( cache[ pathfile ] = cache[ pathfile ] ) = date();
			return false;
		}

		time_t modtime = cache[ pathfile ];
		time_t curtime = date();

		auto diff = std::difftime( modtime, curtime );
		bool changed = ( diff > 0 ? diff : -diff ) > 2.5;  // fat32 minimal lapse is 2 seconds; others filesystems are close to zero

		if( changed )
			cache[ pathfile ] = curtime;

		return changed;
	}

	// metadata api

	std::string &file::meta( const std::string &property ) {
		return metadata[ property ] = metadata[ property ];
	}
	const std::string &file::meta( const std::string &property ) const {
		auto found = metadata.find( property );
		if( found != metadata.end() ) {
			return found->second;
		}
		static std::string invalid;
		return invalid = std::string();
	}
	std::map< std::string, std::string > &file::metas() {
		return metadata;
	}
	const std::map< std::string, std::string > &file::metas() const {
		return metadata;
	}

	bool file::patch( const std::string &patch_data, bool delete_tempfile ) const {
		if( !patch_data.size() )
			return false;

		bool success = false;

		std::string unpatched_file = pathfile;
		std::string tempfile = unpatched_file + ".$old"; // @todo: use tmpname()

		//try to remove
		file( tempfile ).remove();

		//try to rename & smash
		if( file( unpatched_file ).rename( tempfile ) ) {
			// try to patch
			if( file( unpatched_file ).overwrite( patch_data ) ) {
				success = true;
			}
			else {
				// try to rollback
				if( !file( tempfile ).rename( unpatched_file ) ) {
					//argh! what do we do now? :(
				}
			}
		}

		//try to remove
		if( delete_tempfile )
			file( tempfile ).remove();

		return success;
	}

	const std::vector<std::string> folder::default_masks = []()->std::vector<std::string> {
		std::vector<std::string> d(1);
		return d[0] = "*", d;
	}();

	folder::folder()
	{}

	void folder::include( const std::string &path, const std::vector<std::string> &masks, bool scan_subdirs ) { // addition: beware duplicates (set!)
		recurse( *this, path, masks, scan_subdirs );
	}

	void folder::exclude( const std::string &path, const std::vector<std::string> &masks, bool scan_subdirs ) { // subtraction: beware duplicates (set!)
		folder target, to_remove;
		recurse( to_remove, path, masks, scan_subdirs );
		for( const_iterator it = this->begin(); it != this->end(); ++it )
			if( to_remove.find( file( it->name() ) ) == to_remove.end() )
				target.insert( file( it->name() ) );
		std::swap( *this, target );
	}

	std::string folder::str( const char *format1 ) const {
		std::string out;
		for( const_iterator it = this->begin(); it != this->end(); ++it )
			out += replace( format1, "\1", it->name() );
		return out;
	}
}

#undef $win32
#undef $welse

// todo
// platform = { roots[], separators[] }
// unix = { {"/*"}, {"/"} }
// win = { {"?:\*"}, {"\\"} }
// http = { {"http://*"}, {'/', '&', '?'} }

// absolute
// relative

// up
// cwd
// cd

// path, stem
// file
// ext
// args
// fileext
// pathfile
// pathfileextargs

// pop,push,at,size()
// split,join,range

// sane
// os
// directory
// is_dir, is_file
// exists
// has_trail
// parent
// root

// operations
// rm
// rmr
// md
// mv
// ls
// lsr
// touch

/* A class for path manipulation */
namespace apathy {

	/**************************************************************************
	 * Operators
	 *************************************************************************/
	path &path::operator<<( const path &segment) {
		return append(segment);
	}

	path path::operator+( const path &segment) const {
		path result(m_path);
		result.append(segment);
		return result;
	}

	bool path::equivalent( const path &other) const {
		/* Make copies of both paths, normalize, and ensure they're equal */
		std::string A = path(m_path).normalize().absolute().os();
		std::string B = path(other).normalize().absolute().os();
		bool ok = A == B;
		if( !ok ) {
			std::cout << "A:" << A << ',' << std::endl;
			std::cout << "B:" << B << ',' << std::endl;
		}
		return ok;
	}

	std::string path::filename() const {
		size_t pos = m_path.rfind(separator);
		if (pos != std::string::npos) {
			return m_path.substr(pos + 1);
		}
		return std::string();
	}

	std::string path::extension() const {
		/* Make sure we only look in the filename, and not the path */
		std::string name = filename();
		size_t pos = name.rfind('.');
		if (pos != std::string::npos) {
			return name.substr(pos + 1);
		}
		return std::string();
	}

	path path::stem() const {
		size_t sep_pos = m_path.rfind(separator);
		size_t dot_pos = m_path.rfind('.');
		if (dot_pos == std::string::npos) {
			return path(*this);
		}

		if (sep_pos == std::string::npos || sep_pos < dot_pos) {
			return path(m_path.substr(0, dot_pos));
		} else {
			return path(*this);
		}
	}

	/**************************************************************************
	 * Manipulators
	 *************************************************************************/
	path &path::append( const path &segment) {
		/* First, check if the last character is the separator character.
		 * If not, then append one and then the segment. Otherwise, just
		 * the segment */
		if (!trailing_slash()) {
			m_path.push_back(separator);
		}
		m_path.append(segment.m_path);
		return *this;
	}

	path &path::relative( const path &rel) {
		if (!rel.is_absolute()) {
			return append(rel);
		} else {
			operator=(rel);
			return *this;
		}
	}

	path &path::up() {
		/* Make sure we turn this into an absolute url if it's not already
		 * one */
		if (m_path.size() == 0) {
			m_path = "..";
			return directory();
		}

		append(path("..")).normalize();
		if (m_path.size() == 0) {
			return *this;
		}
		return directory();
	}

	path path::absolute() const {
		return path( is_absolute() ? *this : path( join(cwd(), m_path) ) );
	}

	path &path::normalize() {
		/* Split the path up into segments */
		std::vector<Segment> segments(split());
		/* We may have to test this repeatedly, so let's check once */
		bool relative = !is_absolute();

		/* Now, we'll create a new set of segments */
		std::vector<Segment> pruned;
		for (size_t pos = 0; pos < segments.size(); ++pos) {
			/* Skip over empty segments and '.' */
			if (segments[pos].segment.size() == 0 ||
				segments[pos].segment == ".") {
				continue;
			}

			/* If there is a '..', then pop off a parent directory. However, if
			 * the path was relative to begin with, if the '..'s exceed the
			 * stack depth, then they should be appended to our m_path. If it was
			 * absolute to begin with, and we reach root, then '..' has no
			 * effect */
			if (segments[pos].segment == "..") {
				if (relative) {
					if (pruned.size() && pruned.back().segment != "..") {
						pruned.pop_back();
					} else {
						pruned.push_back(segments[pos]);
					}
				} else if (pruned.size()) {
					pruned.pop_back();
				}
				continue;
			}

			pruned.push_back(segments[pos]);
		}

		bool was_directory = trailing_slash();
		if (!relative) {
			m_path = std::string(1, separator) + path::join(pruned).m_path;
			if (was_directory) {
				return directory();
			}
			return *this;
		}

		/* It was a relative path */
		m_path = path::join(pruned).m_path;
		if (m_path.length() && was_directory) {
			return directory();
		}
		return *this;
	}

	std::string path::os() const {
#ifdef _WIN32
		return path( *this ).normalize().absolute().win();
#else
		return path( *this ).normalize().absolute().posix();
#endif
	}

	std::string path::win() const {
		std::string copy = this->m_path, path = copy;

		if( path.size() >= 2 && path[2] == ':' ) {
			path = path.substr( 1 );
		}
		if( path.size() >= 1 && path[1] == ':' ) {
			path = path;
		}
		if( path.size() >= 2 && path[0] == separator && path[2] == separator ) {
			std::swap( path[0], path[1] );
			path[1] = ':';
		}
		for( auto &in : path ) {
			if( in == separator ) in = separator_alt;
		}
		while( path.size() && path.back() == separator_alt ) {
			path.pop_back();
		}
		if( copy.size() && (copy.back() == separator || copy.back() == separator_alt) ) {
			path.push_back( separator_alt );
		}
		return path;
	}

	std::string path::posix() const {
		std::string path = win();
		if( path[1] == ':' ) {
			std::swap( path[0], path[1] );
			path[0] = separator;
		}
		for( auto &in : path ) {
			if( in == separator_alt ) in = separator;
		}
		return path;
	}

	path &path::directory() {
		trim();
		m_path.push_back(separator);
		return *this;
	}

	path &path::trim() {
		if (m_path.length() == 0) { return *this; }

		size_t p = m_path.find_last_not_of(separator);
		if (p != std::string::npos) {
			m_path.erase(p + 1, m_path.size());
		} else {
			m_path = std::string();
		}
		return *this;
	}

	/**************************************************************************
	 * Member Utility Methods
	 *************************************************************************/

	/* Returns a vector of each of the path segments in this path */
	std::vector<path::Segment> path::split() const {
		std::stringstream stream(m_path);
		std::istream_iterator<path::Segment> start(stream);
		std::istream_iterator<path::Segment> end;
		std::vector<path::Segment> results(start, end);
		if (trailing_slash()) {
			results.push_back(path::Segment(""));
		}
		return results;
	}

	/**************************************************************************
	 * Tests
	 *************************************************************************/
	bool path::is_absolute() const {
		return ( m_path.size() > 2 && m_path[1] == ':' && m_path[2] == separator_alt )
		|| ( m_path.size() && ( m_path[0] == separator || m_path[0] == separator_alt ) );
	}

	bool path::trailing_slash() const {
		return m_path.size() && (m_path.back() == separator || m_path.back() == separator_alt);
	}

	bool path::exists() const {
		struct stat buf;
		if (stat(os().c_str(), &buf) != 0) {
			return false;
		}
		return true;
	}

	bool path::is_file() const {
		struct stat buf;
		if (stat(os().c_str(), &buf) != 0) {
			return false;
		} else {
			return S_ISREG(buf.st_mode);
		}
	}

	bool path::is_directory() const {
		struct stat buf;
		if (stat(os().c_str(), &buf) != 0) {
			return false;
		} else {
			return S_ISDIR(buf.st_mode);
		}
	}

	size_t path::size() const {
		struct stat buf;
		if (stat(os().c_str(), &buf) != 0) {
			return 0;
		} else {
			return buf.st_size;
		}
	}

	/**************************************************************************
	 * Static Utility Methods
	 *************************************************************************/
	path path::join( const path &a, const path &b) {
		path p(a);
		p.append(b);
		return p;
	}

	path path::join( const std::vector<Segment>& segments) {
		std::string p;
		/* Now, we'll go through the segments, and join them with
		 * separator */
		std::vector<Segment>::const_iterator it(segments.begin());
		for(; it != segments.end(); ++it) {
			p += it->segment;
			if (it + 1 != segments.end()) {
				p += std::string(1, separator);
			}
		}
		return path(p);
	}

	path path::cwd() {
		path p;

		char * buf = getcwd(NULL, 0);
		if (buf != NULL) {
			p = std::string(buf);
			free(buf);
		} else {
			perror("cwd");
		}

		/* Ensure this is a directory */
		p.directory();

#ifdef _WIN32
		p = path( path( p.os() ). posix() );
#endif

		return p;
	}

	bool path::touch( const path &p, unsigned mode_ ) {
		mode_t mode = mode_;
		int fd = open(p.os().c_str(), O_RDONLY | O_CREAT, mode);
		if (fd == -1) {
			md(p);
			fd = open(p.os().c_str(), O_RDONLY | O_CREAT, mode);
			if (fd == -1) {
				return false;
			}
		}
		if (close(fd) == -1) {
			perror("touch close");
			return false;
		}
		return true;
	}

	bool path::mv( const path &source, const path &dest, bool mkdirs) {
		int result = rename(source.os().c_str(), dest.os().c_str());
		if (result == 0) {
			return true;
		}
		/* Otherwise, there was an error */
		if (errno == ENOENT && mkdirs) {
			md(dest.parent());
			return rename(source.os().c_str(), dest.os().c_str()) == 0;
		}
		return false;
	}

	bool path::md( const path &p, unsigned mode_ ) {
		mode_t mode = mode_;
		/* We need to make a copy of the path, that's an absolute path */
		path abs = path(p).absolute();
		auto dirs = abs.split();

		/* Now, we'll try to make the directory / ensure it exists */
		if (mkdir(abs.string().c_str(), mode) == 0) {
			return true;
		}

		/* Otherwise, there was an error. There are some errors that
		 * may be recoverable */
		if (errno == EEXIST) {
			return abs.is_directory();
		} else if(errno == ENOENT) {

			/* We'll need to try to recursively make this directory. We
			 * don't need to worry about reaching the '/' path, and then
			 * getting to this point, because / always exists */
			md(abs.parent(), mode);

			if (mkdir(abs.string().c_str(), mode) == 0) {
				return true;
			} else {
				perror("makedirs");
				return false;
			}
		} else {
			perror("makedirs");
		}

		/* If it's none of these cases, then it's one of unrecoverable
		 * errors described in mkdir(2) */
		return false;
	}

	// remove file or directory
	bool path::rm( const path &p) {
		if( !p.exists() || 0 == remove(p.os().c_str()) || 0 == ::rmdir(p.os().c_str())) {
			errno = 0;
			return true;
		} else {
			perror("Remove");
			return false;
		}
	}

	bool path::rmrf( const path &p, bool ignore_errors) {

		bool ok = true;
		if( p.is_directory() ) {
			/* First, we list out all the members of the path, and anything
			 * that's a directory, we rmrf(...) it. If it's a file, then we
			 * remove it */
			std::vector<path> subdirs(ls(p));
			std::vector<path>::iterator it(subdirs.begin());
			for (; it != subdirs.end(); ++it) {
				if (it->is_directory() && !rmrf(*it) && !ignore_errors) {
					ok = false;
					//std::cout << "Failed rmdirs " << it->string() << std::endl;
				} else if (it->is_file() && !rm(*it) && !ignore_errors) {
					ok = false;
					//std::cout << "Failed remove " << it->string() << std::endl;
				}
			}
		}

		/* Lastly, try to remove the directory, or file, itself */
		bool result = rm(p) && ok;
		return result;
	}

	/* List all the paths in a directory
	 *
	 * @param p - path to list items for */
	std::vector<path> path::ls( const path &p ) {
#ifdef _WIN32
		path base(p.win());
#else
		path base(p.posix());
#endif

		std::vector<path> results;
		DIR* dir = opendir(base.string().c_str());
		if (dir == NULL) {
			/* If there was an error, return an empty vector */
			return results;
		}

		/* Otherwise, go through everything */
		for (dirent* ent = readdir(dir); ent != NULL; ent = readdir(dir)) {
			/* Skip the parent directory listing */
			if (!strcmp(ent->d_name, "..")) {
				continue;
			}

			/* Skip the self directory listing */
			if (!strcmp(ent->d_name, ".")) {
				continue;
			}

			path cpy = path(base).relative(path(ent->d_name)).posix();
			results.push_back( cpy );
		}

		errno = 0;
		closedir(dir);
		return results;
	}

	std::vector<path> path::glob( const std::string& pattern ) {

		std::string where = path( pattern ).normalize().string();
		std::string mask = pattern;

		auto found = where.find_last_of(separator);
		if( found != std::string::npos ) {
			//mask = where.substr( found +1 );
			mask = where;
			where = where.substr( 0, found +1 );
		} else {
			mask = pattern;
			where = std::string();
		}

		struct local {
			static bool match( const char *pattern, const char *str ) {
				if( *pattern=='\0' ) return !*str;
				if( *pattern=='*' )  return match(pattern+1, str) || (*str && match(pattern, str+1));
				if( *pattern=='?' )  return *str && (*str != '.') && match(pattern+1, str+1);
				return (*str == *pattern) && match(pattern+1, str+1);
			}
		};

		std::vector<path> results;
		for( auto &file : path::ls( where ) ) {
			if( local::match(mask.c_str(), file.m_path.c_str()) ) {
				results.push_back( file );
			}
		}
		return results;
	}
}

//
namespace apathy {
		bool file::mmap() {
			return chunk( 0, ~0 ).good();
		}
		bool file::munmap() {
			return chunk( 0, 0 ), true;
		}
		stream file::chunk( size_t offset, size_t length ) {
			//try {
				auto found = chunks->find( name() );
				if( found == chunks->end() ) {
					(*chunks)[ name() ];
					found = chunks->find( name() );
				}
				auto &fm = found->second;
				if( offset < size() && length ) {
					fm.open( name().c_str(), length == ~0u ? 0 : length, offset );
					if( fm.is_open() ) {
						return stream( fm.data(), length == ~0u ? fm.size() : length );
					}
				}
				/*
			} catch(std::exception &e) {
				std::cout << "exception: " << e.what() << std::endl;
			} catch(...) {
				std::cout << "unknown exception" << std::endl;
			} */
			return stream();
		}
}

// extras

// @todo
// interleaving
// utf8 path/file cases
// embed zip tocs

namespace apathy {

	// utils

		std::ostream &print( const uris &uris_, std::ostream &out ) {
			for( auto &uri : uris_ ) {
				out << uri << std::endl;
			}
			return out;
		}

		std::string replace( const std::string &str_, const std::string &from, const std::string &to ) {
			// Michael Mrozek's
			std::string str( str_ );
			if( !from.empty() ) {
				size_t start_pos = 0;
				while((start_pos = str.find(from, start_pos)) != std::string::npos) {
					str.replace(start_pos, from.length(), to);
					start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
				}
			}
			return str;
		}

		std::vector<std::string> split( const std::string &s, char sep ) {
			std::stringstream ss( s );
			std::vector<std::string> vector;
			for( std::string join; std::getline( ss, join, sep ); vector.push_back( join ) )
			{}
			return vector;
		}

		std::vector<std::string> wildcards( const std::string &mask ) {
			return split( replace( replace( replace( mask, "|", "\1" ), ";", "\1" ), ",", "\1" ), '\1' );
		}

		bool is_dir( const std::string &uri ) {
			return apathy::file( uri ).is_dir(); //also, we could opendir() instead
		}

		bool exists( const std::string &uri ) {
			return apathy::file( uri ).exists();
		}

		bool has_changed( const std::string &uri ) {
			return apathy::file( uri ).has_changed();
		}

		bool matches( const std::string &text, const std::string &pattern ) {
			struct local {
				static bool match( const char *pattern, const char *str ) {
					if( *pattern=='\0' ) return !*str;
					if( *pattern=='*' )  return match(pattern+1, str) || (*str && match(pattern, str+1));
					if( *pattern=='?' )  return *str && (*str != '.') && match(pattern+1, str+1);
					return (*str == *pattern) && match(pattern+1, str+1);
			} };
			return local::match( pattern.c_str(), text.c_str() );
		}

		std::string normalize( const std::string &uri_ ) {
			std::string uri( uri_ );
			for( auto &ch : uri ) {
				/**/ if( ch >= 'A' && ch <= 'Z' ) ch = (ch - 'A') + 'a';
				else if( ch == ' '  ) ch = '_';
				else if( ch == '\\' ) ch = '/';
			}
			return uri;
		}

		std::string notrails( const std::string &uri ) {
			assert( "uri must be normalized" && uri.find_first_of('\\') + 1 == 0 );
			unsigned from = uri.size() > 2 && uri[0] == '.' && uri[1] == '/' ? 2 : 0;
			unsigned to = uri.find_last_of('/') + 1;
			return uri.substr( from, uri.size() - from + to );
		}

		void sleep( double t ) {
			std::this_thread::sleep_for( std::chrono::milliseconds(int(t * 1000)) );
		}

	// working directories

		namespace {
			std::vector<std::string> &_cwd() {
				static std::vector<std::string> list = []() -> std::vector<std::string> {
				  std::vector<std::string> l( 1 );
				  return l[0] = "./", l;
				}();
				return list;
			}
		}
		void chdir( const std::string &path ) {
			_cwd().back() = path;
		}
		void pushd() {
			_cwd().push_back( _cwd().back() );
		}
		void popd() {
			if( _cwd().size() > 1 ) _cwd().pop_back();
		}
		std::string cwd() {
			return _cwd().back(); //apathy::path::cwd().string();
		}

	//

#       ifdef mkdir
#           undef mkdir
#       endif
		bool mkdir( const std::string &path, unsigned mode ) {
			return path::md( path, mode );
		}
		bool rmdir( const std::string &path ) {
			return path::rmrf( path, true );
		}

	// glob files and directories

		bool globr( const std::string &uri_, uris &files, uris &dirs, unsigned recurse ) {
			std::string uri = normalize(uri_), pretty = uri;

			// pretty might be '/home/user/dir////', or 'c:\' at this point
			// ensure it does not convert into 'c:' (that would retrieve files at cwd instead of root files)
			if( pretty.size() && (pretty.back() != ':' && pretty.back() != '/') ) pretty += '/';

			DIR *opened = 0;
			for( DIR *dir = opened = opendir( uri.c_str() ); dir; closedir(dir), dir = 0 ) {
				for( struct dirent *ent = readdir(dir); ent ; ent = readdir(dir) ) {
					std::string name( ent->d_name );
					if( apathy::file( pretty + name ).is_dir() ) {
						if( name[0] != '.' && name != "svn" ) { // bypass recursion of . .. .hg .git svn dirs
							dirs.insert( notrails( normalize( pretty + name ) ) );
							if( recurse ) globr( pretty + name, files, dirs, recurse - 1 );
						}
					} else {
						files.insert( normalize( pretty + name ) );
					}
				}
			}
			return true; //opened ? true : false;
		}

		bool glob( const std::string &uri, uris &fs, uris &dirs ) {
			return globr( uri, fs, dirs, 0 );
		}

		uris &filter( uris &set, const std::string &includes_, const std::string &excludes_ ) {
			auto includes = wildcards( includes_ );
			auto excludes = wildcards( excludes_ );
			for( auto it = set.begin(); it != set.end(); ) {
				apathy::file file( *it );
				bool included = false;
				for( auto &include : includes ) {
					if( file.matches(include) ) {
						included = true;
						break;
					}
				}
				if( included )
				for( auto &exclude : excludes ) {
					if( file.matches(exclude) ) {
						included = false;
						break;
					}
				}
				it = included ? ++it : set.erase( it );
			}
			return set;
		}

		uris lsf( const std::string &includes, const std::string &excludes ) {
			uris files, ignore;
			return glob( cwd(), files, ignore ) ? filter( files, includes, excludes ) : uris();
		}
		uris lsfr( const std::string &includes, const std::string &excludes ) {
			uris files, ignore;
			return globr( cwd(), files, ignore ) ? filter( files, includes, excludes ) : uris();
		}
		uris lsd( const std::string &includes, const std::string &excludes ) {
			uris dirs, ignore;
			return glob( cwd(), ignore, dirs ) ? filter( dirs, includes, excludes ) : uris();
		}
		uris lsdr( const std::string &includes, const std::string &excludes ) {
			uris dirs, ignore;
			return globr( cwd(), ignore, dirs ) ? filter( dirs, includes, excludes ) : uris();
		}
		uris ls( const std::string &includes, const std::string &excludes ) {
			uris any;
			return glob( cwd(), any, any ) ? filter( any, includes, excludes ) : uris();
		}
		uris lsr( const std::string &includes, const std::string &excludes ) {
			uris any;
			return globr( cwd(), any, any ) ? filter( any, includes, excludes ) : uris();
		}

	// folder monitoring

		void watch( const std::string &uri_, const watcher_callback &callback ) {
			std::string uri( notrails( normalize( uri_ ) ) );
			if( 1 ) { //:( apathy::file(uri).is_dir() ) {
				watcher wt;
				wt.folder = uri;
				wt.callback = callback;
				wt.reload();
			}
		}

		void watcher::operator()() {
			in = out = modif = uris();
			uris copy = previous;

			for( auto &file : previous ) {
				if( exists(file) ) {
					if( has_changed(file) ) {
						modif.insert( file );
					}
				} else {
					out.insert( file );
					copy.erase( file );
				}
			}

			uris now;
			if( globr( folder, now, now ) )
			for( auto &file : now ) {
				if( copy.find( file ) == copy.end() ) {
					in.insert( file );
					copy.insert( file );
				}
			}

			previous = copy;
			callback( *this );
		}

		void watcher::reload() {
			std::thread( *this ).detach();
		}

		void watcher::sleep( double t ) {
			apathy::sleep( t );
		}

	// uri

		uri2::uri2()
		{}

		uri2::uri2( const std::string &uri ) {
			std::string full( uri );

			protocol = left_of( full, "://" );
			if( !protocol.empty() ) protocol += "://";
			full = remove( full, protocol );

			options = "?" + right_of( full, "?" );
			full = remove( full, options );

			path = full;
			file = path.substr( path.find_last_of("/") + 1 );
			path = remove(path, file);
		}

		std::string uri2::left_of( const std::string &input, const std::string &substring ) const {
			std::string::size_type pos = input.find( substring );
			return pos == std::string::npos ? std::string() : input.substr(0, pos);
		}

		std::string uri2::right_of( const std::string &input, const std::string &substring ) const {
			std::string::size_type pos = input.find( substring );
			return pos == std::string::npos ? std::string() : input.substr(pos + substring.size());
		}

		std::string uri2::remove( const std::string &input, const std::string &substring ) const {
			return replace( input, substring, "" );
		}

	// table of contents

		bool filesystem::mount( const std::string &phys, const std::string &virt ) {
			std::string mask = phys.substr(phys.find_first_of("*"));
			std::string path = replace(phys, mask, "");
			if( path.find_last_of('/') != std::string::npos ) path.resize( path.find_last_of('/') );
			if( mask.empty() ) mask = "*";
			if (path.empty()) path = "."; path += "/";

			auto mount = [=]( const std::string &phys, const std::string &virt_, const std::string &mask ) {
				std::string virt(virt_);
				while( virt.size() && virt.front() == '/' ) virt = virt.substr(1);
				pushd();
				chdir(phys);
					bool recurse_subdirs = (mask.find("**") != std::string::npos);
					auto found = recurse_subdirs ? lsr( mask ) : ls( mask );
					for( const auto &uri : found ) {
						std::string virt2( virt + replace( replace(uri, cwd(), "" ) ,phys,"") );
						table[ virt2 ] = uri;
					}
				popd();
				return found.size() ? true : false;
			};

			return mount( path, virt, mask );
		}

		bool filesystem::supported( const std::string &virt ) const {
			uri2 uri( virt );
			for( auto &protocol : protocols ) {
				if( protocol == uri.protocol || ( protocol.empty() && uri.protocol.empty() ) ) {
					return true;
				}
			}
			return false;
		}

		std::string filesystem::translate( const std::string &virt ) const {
			if( supported( virt ) ) {
				uri2 uri(virt);
				auto found = table.find(uri.path + uri.file);
				if( found != table.end() ) {
					return found->second;
				}
			}
			return std::string();
		}

		bool filesystem::exists( const std::string &virt ) const {
			std::string phys = translate( virt );
			return !phys.empty();
		}

		bool filesystem::symlink( const std::string &from_virt, const std::string &to_virt ) {
			return false;
		}

		bool filesystem::rename( const std::string &virt_mask, const std::string &as_virt ) {
			auto copy = table;
			unsigned replaced = 0;
			for( auto &it : table ) {
				if( matches(it.first, virt_mask) ) {
					copy.erase( it.first );
					copy[ as_virt ] = it.second;
					replaced++;
				}
			}
			return replaced > 0 ? ( table = copy, true ) : false;
		}

		std::string filesystem::head() const {
			std::string head;
			for( auto &protocol : protocols ) {
				head += "[" + protocol + "]";
			}
			return head;
		}

		std::string filesystem::print() const {
			std::string toc, head = this->head();
			toc += "## filesystem: " + name() + "\n";
			for( auto &entry : table ) {
				toc += "\t" + head + entry.first + " -> " + entry.second + "\n";
			}
			return toc;
		}

	// virtual filesystem

		vfilesystem::vfilesystem()
		{}

		vfilesystem::~vfilesystem()  {
			for( auto &fs : filesystems ) {
				delete fs, fs = 0;
			}
		}

		bool vfilesystem::mount( const std::string &phys, const std::string &virt ) {
			bool found = false;
			for( auto &fs : filesystems ) {
				found |= fs->mount( phys, virt );
			}
			return found;
		}

		bool vfilesystem::exists( const std::string &virt ) const {
			return locate( virt ) ? true : false;
		}

		std::string vfilesystem::read( const std::string &virt ) const {
			filesystem *fs = locate( virt );
			if( fs ) {
				std::string phys = fs->translate( virt );
				if( !phys.empty() ) {
					std::string data = fs->read( phys );
					return data;
				}
			}
			return std::string();
		}

		std::string vfilesystem::toc() const {
			std::string toc;
			for( auto &fs : filesystems ) {
				toc += fs->print();
				/*
				for( auto &toc : fs.scan() ) {
					toc += toc.print();
				}
				*/
			}
			return toc;
		}

		filesystem *vfilesystem::locate( const std::string &virt ) const {
			std::string found;
			for( auto &fs : filesystems ) {
				found = fs->translate( virt );
				if( !found.empty() ) {
					return fs;
				}
			}
			return 0;
		}

	// stream api

#       ifdef open
#           undef open
#       endif
#       ifdef close
#           undef close
#       endif

		stream::stream() {
			begin = end = cursor = rw = 0;
		}

		stream::stream( const void *from, const void *to ) {
			begin = (const char *)( from < to ? from : to);
			end = (const char *)( to > from ? to : from);
			cursor = rw = 0;
			open();
		}
		stream::stream( const void *ptr, unsigned len ) {
			begin = (const char *)ptr;
			end = begin + len;
			cursor = rw = 0;
			open();
		}
		stream::stream( void *ptr, unsigned len ) {
			begin = (const char *)ptr;
			end = begin + len;
			cursor = rw = 0;
			rw = (rw+1);
			open();
		}

		bool stream::good( unsigned off ) const {
			return cursor && cursor >= begin && cursor + off < end;
		}

		bool stream::open() {
			cursor = (char *)begin;
			return good();
		}
		bool stream::close() {
			cursor = (char *)end;
			// flush to disk/net here
			return good();
		}

		bool stream::rewind() {
			cursor = (char *)begin;
			return good();
		}
		bool stream::eof() const {
			return cursor >= end;
		}
		unsigned stream::tell() const {
			return cursor - begin;
		}
		unsigned stream::size() const {
			return end - begin;
		}
		unsigned stream::left() const {
			return size() - tell();
		}
		const char *stream::data() const {
			return begin;
		}
		bool stream::seek( unsigned offset ) {
			cursor = (char *)(begin + offset);
			return good();
		}
		bool stream::offset( int offset ) {
			cursor += offset;
			return good();
		}

		bool stream::read( void *ptr, unsigned N ) {
			const unsigned top = left(), max = N < top ? N : top;
			return ptr && good(max) && memcpy( ptr, ( cursor += max ) - max, max );
		}
		bool stream::write( const void *ptr, unsigned N ) {
			const unsigned top = left(), max = N < top ? N : top;
			return rw && ptr && good(max) && memcpy( (char *)( ( cursor += max ) - max ), ptr, max );
		}

		bool stream::read8( int8_t &c ) {
			return read( c ) ? true : false;
		}
		bool stream::read16( int16_t &c ) {
			return read( c ) ? giant::letoh( c ), true : false;
		}
		bool stream::read32( int32_t &c ) {
			return read( c ) ? giant::letoh( c ), true : false;
		}
		bool stream::read64( int64_t &c ) {
			return read( c ) ? giant::letoh( c ), true : false;
		}
		bool stream::write8( const int8_t &c ) {
			return write( c );
		}
		bool stream::write16( const int16_t &c ) {
			return write( giant::htole(c) );
		}
		bool stream::write32( const int32_t &c ) {
			return write( giant::htole(c) );
		}
		bool stream::write64( const int64_t &c ) {
			return write( giant::htole(c) );
		}
		int stream::getc() {
			int8_t n;
			return read8(n) ? n : -1;
		}

		// states
		void stream::push() {
			stack.push_back( *this );
		}
		void stream::pop() {
			*this = stack.back();
			stack.pop_back();
		}
}

//

