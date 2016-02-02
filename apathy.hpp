// Apathy is a lightweight path/file/mstream/mmap IO library (C++03).
// - rlyeh, zlib/libpng licensed ~~ listening to Alice in chains / Nutshell.

// Features:
// [x] Append, overwrite, create, copy, move, delete, delete recursively, etc
// [x] Binary file patching (including locked binaries)
// [x] C++03
// [x] Compatible in-memory std::istreams/ostreams
// [x] Fast file/dir disk globbing
// [x] Functional API
// [x] Memory-map support
// [x] Semantic URI class
// [x] Tmpdir and tmpnames
// [x] Touch and date functions
// [x] Wildcards support
// [x] Tiny, portable, cross-platform and header-only.
// [x] ZLIB/LibPNG licensed

// Todo:
// [ ] `bool cpr( const pathfile &uri, const path &uri_dst );`

#pragma once

#define APATHY_VERSION "1.0.2" /* (2016/02/02): Fix ext() with dotless files; Fix m/c/adate() on invalid pathfiles; Handle proper Win32 stat() case
#define APATHY_VERSION "1.0.1" // (2015/12/02): Add resize() function
#define APATHY_VERSION "1.0.0" // (2015/11/20): Simplified API, moved vfs/ostream to libraries apart
#define APATHY_VERSION "0.0.0" // (2013/04/16): Initial commit */

#ifndef APATHY_USE_MMAP
#define APATHY_USE_MMAP 1
#endif

#include <cassert>     // assert
#include <cerrno>      // errno, perror
#include <cstdio>      // size_t
#include <cstdlib>     // printf
#include <ctime>       // std::time, std::difftime
#include <fcntl.h>     // O_RDONLY, O_CREAT
#include <string.h>    // memcpy, strerror
#include <sys/stat.h>  // stat, lstat
#include <sys/types.h> //

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#   define  $apathy32(...) __VA_ARGS__
#   define  $apathyXX(...)
#else
#   define  $apathy32(...)
#   define  $apathyXX(...) __VA_ARGS__
#endif

namespace apathy {

	// General constants

	enum {                         // own|grp|any
		default_path_mode = 0755,  // rwx|r-x|r-x
		default_file_mode = 0644,  // rwx|r--|r--
	};

	// These are memory stream classes.
	// During their lifetime you can wrap std::istream or std::ostream classes over them.

	// Usage:
	// { imstream membuf(ptr, len); std::istream is(&membuf); /*...*/ }
	// { omstream membuf(ptr, len); std::ostream os(&membuf); /*...*/ }

	struct imstream : std::streambuf {
		imstream( const char* base, std::ptrdiff_t n ) {
			this->setg( (char *)base, (char *)base, (char *)base + n );
		}
		size_t position() const {
			return this->pptr() - this->pbase();
		}
	};

	struct omstream : std::streambuf {
		omstream( char* base, std::ptrdiff_t n ) {
			this->setp( base, base + n );
		}
		size_t position() const {
			return this->pptr() - this->pbase();
		}
	};

	// This uri class is used to disambiguate path/file cases, specially in API
	// - file is an non-empty uri that does not end with '/'
	// - path is an non-empty uri that ends with '/', or an empty one
	// - pathfile is a concatenation of several paths and/or a single file
	// - concatenation is done by using slash '/' operator: path1/path2/file

	// Examples:
	//    uri | type |  path  | file
	// -------|------|--------|-----
	// ""     | path |  ""    |
	// "./"   | path | "./"   |
	// "a"    | file |        | "a"
	// "a/"   | path | "a/"   |
	// "a/b"  | file | "a/"   | "b"
	// "a/b/" | path | "a/b/" |

	// Invalid configs will trigger an assert:
	// - Providing windows \\ slashes in any file/path/pathfile class
	// - Providing a /valid/path/ in a file class will trigger an assert()
	// - Providing a /valid/file  in a path class will trigger an assert()

	template<int type>
	struct uri : public std::string {
		uri() : std::string()
		{}
		template<int type2>
		uri( const uri<type2> &other ) : std::string(other)
		{}
		template<typename T>
		uri( const T &t ) : std::string(t) {
			fix();
		}
		template<size_t N>
		uri( const char (&t)[N] ) : std::string(t, N) {
			fix();
		}
		uri( const char *t ) : std::string(t ? t : "") {
			fix();
		}
		uri<2> operator /( const std::string &other ) const {
			return this->size() && *this->rbegin() != '/' ? (*this) + "/" + other : (*this) + other;
		}
		operator const char *() const {
			return this->c_str();
		}
		void fix() {
			// ensure no win32 slashes are present anymore
			$apathy32( assert( this->find('\\') == std::string::npos ) );
			// ensure hard constraints are met
			if( type == 0 ) {
				if( !this->empty() && '/' != *this->rbegin() ) {
					this->append("/");
				}
				assert( is_path() );
			}
			if( type == 1 ) {
				assert( is_file() );
			}
		}
		bool is_path() const {
			return type != 1 && (this->empty() || '/' == *this->rbegin());
		}
		bool is_file() const {
			return type != 0 && (!this->empty() && '/' != *this->rbegin());
		}
	};

	typedef uri<0> path    ;
	typedef uri<1> file    ;
	typedef uri<2> pathfile;

	// From this point, any of following functions may fail due to permissions, non-existing path, etc
	// Check apathy::why() to retrieve reason for last error.

	// Read/write API

	void  *map( const file &uri, size_t size, size_t offset = 0 );
	void unmap( void *ptr, size_t size );

	bool read( const file &uri, std::string &buffer );
	bool read( const file &uri, void *data, size_t &size );

	bool append( const file &uri, const std::string &data );
	bool append( const file &uri, const void *data, size_t size );

	bool overwrite( const file &uri, const std::string &data );
	bool overwrite( const file &uri, const void *data, size_t size );

	bool resize( const file &uri, size_t new_size );

	// Info API (RO)

	bool   exists( const pathfile &uri );

	bool  is_path( const pathfile &uri );
	bool  is_file( const pathfile &uri );
	bool  is_link( const pathfile &uri );

	size_t   size( const pathfile &uri ); // num bytes (file) or number of items(path)
	bool    empty( const pathfile &uri ); // true if null (file) or no items (path)

	int       gid( const pathfile &uri ); // group id
	int       uid( const pathfile &uri ); // user id

	path     stem( const pathfile &uri ); // d:/prj/test.png -> d:/prj/
	file     name( const pathfile &uri ); // d:/prj/test.png -> test.png
	file     base( const     file &uri ); // d:/prj/test.png -> test
	pathfile  ext( const     file &uri ); // d:/prj/test.png -> .png

	// Info API (RW)

	bool    chown( const file &uri, int uid, int gid );             // change owner and group ids
	bool    chmod( const file &uri, int mode = default_file_mode ); // change file permissions

	// Folder API

	bool pushd( const path &uri );
	bool  popd();
	path   cwd();
	bool    cd( const path &uri );
	bool    rd( const path &uri );
	bool    md( const path &uri, size_t mode = default_path_mode );

	// Disk operations API
	// move, copy, copy (recursive), remove, remove (recursive), listing, listing (recursive)

	bool   mv( const pathfile &uri, const pathfile &uri_dst );
	bool   cp( const pathfile &uri, const pathfile &uri_dst );
	bool   rm( const pathfile &uri );
	bool rmrf( const pathfile &uri );
	bool   ls( std::vector<std::string> &list, const path &uri = "", const std::string &masks = "*" );
	bool  lsr( std::vector<std::string> &list, const path &uri = "", const std::string &masks = "*" );

	// File patching API (will patch locked binaries too)

	bool patch( const file &uri, const file &patchdata );

	// Date & modif API

	bool touch( const pathfile &uri, const time_t &date = std::time(0) );
	bool touched( const pathfile &uri ); // check for external changes, first call returns false always

	std::time_t adate( const pathfile &uri );
	std::time_t cdate( const pathfile &uri );
	std::time_t mdate( const pathfile &uri );
	std::string stamp( const std::time_t &date, const char *format = "%Y-%m-%d %H:%M:%S" ); // defaults to MySQL date format

	// Temp API

	path tmpdir();
	file tmpname();

	// Handy aliases (for convenience)

	std::string read( const file &uri );
	std::vector<std::string> ls( const path &uri = "", const std::string &masks = "*" );
	std::vector<std::string> lsr( const path &uri = "", const std::string &masks = "*" );

	// Error retrieval API

	std::string why();

	// Diverse utils

	void  sleep( double t );
	bool  match( const char *uri, const char *pattern );
	size_t glob( std::vector<std::string> &out, const path &uri, const std::string &masks, bool recursive = false, bool skip_dotdirs = false );
	size_t glob( std::vector<std::string> &out, const path &uri, const std::vector<std::string> &masks, bool recursive = false, bool skip_dotdirs = false );
	size_t glob( std::map<std::string, bool> &out, const path &uri, const std::string &masks, bool recursive = false, bool skip_dotdirs = false );
	size_t glob( std::map<std::string, bool> &out, const path &uri, const std::vector<std::string> &masks, bool recursive = false, bool skip_dotdirs = false );
	std::string replace( const std::string &str, const std::string &from, const std::string &to );
	std::string normalize( const std::string &str );
	std::vector<std::string> split( const std::string &str, char sep );
	std::vector<std::string> wildcards( const std::string &mask );
}

// select ms or posix/mingw route

#ifndef _WIN32
#   if APATHY_USE_MMAP
#       include <sys/mman.h>
#   endif
#   include <dirent.h>
#   include <utime.h>
#   include <unistd.h>
#else
#   if APATHY_USE_MMAP

//#line 1 "mman.h"
/* References:
 * CreateFileMapping: http://msdn.microsoft.com/en-us/library/aa366537(VS.85).aspx
 * CloseHandle:       http://msdn.microsoft.com/en-us/library/ms724211(VS.85).aspx
 * MapViewOfFile:     http://msdn.microsoft.com/en-us/library/aa366761(VS.85).aspx
 * UnmapViewOfFile:   http://msdn.microsoft.com/en-us/library/aa366882(VS.85).aspx
 */

#pragma once
#if defined(__MINGW32__) || defined(_WIN32)

#include <io.h>
#include <windows.h>
#include <sys/types.h>

#define PROT_READ     0x1
#define PROT_WRITE    0x2
/* This flag is only available in WinXP+ */
#ifdef FILE_MAP_EXECUTE
#define PROT_EXEC     0x4
#else
#define PROT_EXEC        0x0
#define FILE_MAP_EXECUTE 0
#endif

#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define MAP_ANON      MAP_ANONYMOUS
#define MAP_FAILED    ((void *) -1)

void* mmap(void* start, size_t length, int prot, int flags, int fd, off_t offset);
void munmap(void* addr, size_t length);

#ifdef __USE_FILE_OFFSET64
# define DWORD_HI(x) (x >> 32)
# define DWORD_LO(x) ((x) & 0xffffffff)
#else
# define DWORD_HI(x) (0)
# define DWORD_LO(x) (x)
#endif

static
void* mmap(void* start, size_t length, int prot, int flags, int fd, off_t offset)
{
	if (prot & ~(PROT_READ | PROT_WRITE | PROT_EXEC))
		return MAP_FAILED;
	if (fd == -1) {
		if (!(flags & MAP_ANON) || offset)
			return MAP_FAILED;
	} else if (flags & MAP_ANON)
		return MAP_FAILED;

	DWORD flProtect;
	if (prot & PROT_WRITE) {
		if (prot & PROT_EXEC)
			flProtect = PAGE_EXECUTE_READWRITE;
		else
			flProtect = PAGE_READWRITE;
	} else if (prot & PROT_EXEC) {
		if (prot & PROT_READ)
			flProtect = PAGE_EXECUTE_READ;
		else if (prot & PROT_EXEC)
			flProtect = PAGE_EXECUTE;
	} else
		flProtect = PAGE_READONLY;

	off_t end = length + offset;
	HANDLE mmap_fd, h;
	if (fd == -1)
		mmap_fd = INVALID_HANDLE_VALUE;
	else
		mmap_fd = (HANDLE)_get_osfhandle(fd);
	h = CreateFileMapping(mmap_fd, NULL, flProtect, DWORD_HI(end), DWORD_LO(end), NULL);
	if (h == NULL)
		return MAP_FAILED;

	DWORD dwDesiredAccess;
	if (prot & PROT_WRITE)
		dwDesiredAccess = FILE_MAP_WRITE;
	else
		dwDesiredAccess = FILE_MAP_READ;
	if (prot & PROT_EXEC)
		dwDesiredAccess |= FILE_MAP_EXECUTE;
	if (flags & MAP_PRIVATE)
		dwDesiredAccess |= FILE_MAP_COPY;
	void *ret = MapViewOfFile(h, dwDesiredAccess, DWORD_HI(offset), DWORD_LO(offset), length);
	if (ret == NULL) {
		ret = MAP_FAILED;
	}
	// since we are handling the file ourselves with fd, close the Windows Handle here
	CloseHandle(h);
	return ret;
}

static
void munmap(void* addr, size_t length)
{
	UnmapViewOfFile(addr);
}

#undef DWORD_HI
#undef DWORD_LO

#endif


#   endif

//#line 1 "dirent.h"
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

/* File type and permission flags for stat(), general mask */
#if !defined(S_IFMT)
#   define S_IFMT _S_IFMT
#endif

/* Directory bit */
#if !defined(S_IFDIR)
#   define S_IFDIR _S_IFDIR
#endif

/* Character device bit */
#if !defined(S_IFCHR)
#   define S_IFCHR _S_IFCHR
#endif

/* Pipe bit */
#if !defined(S_IFFIFO)
#   define S_IFFIFO _S_IFFIFO
#endif

/* Regular file bit */
#if !defined(S_IFREG)
#   define S_IFREG _S_IFREG
#endif

/* Read permission */
#if !defined(S_IREAD)
#   define S_IREAD _S_IREAD
#endif

/* Write permission */
#if !defined(S_IWRITE)
#   define S_IWRITE _S_IWRITE
#endif

/* Execute permission */
#if !defined(S_IEXEC)
#   define S_IEXEC _S_IEXEC
#endif

/* Pipe */
#if !defined(S_IFIFO)
#   define S_IFIFO _S_IFIFO
#endif

/* Block device */
#if !defined(S_IFBLK)
#   define S_IFBLK 0
#endif

/* Link */
#if !defined(S_IFLNK)
#   define S_IFLNK 0
#endif

/* Socket */
#if !defined(S_IFSOCK)
#   define S_IFSOCK 0
#endif

/* Read user permission */
#if !defined(S_IRUSR)
#   define S_IRUSR S_IREAD
#endif

/* Write user permission */
#if !defined(S_IWUSR)
#   define S_IWUSR S_IWRITE
#endif

/* Execute user permission */
#if !defined(S_IXUSR)
#   define S_IXUSR 0
#endif

/* Read group permission */
#if !defined(S_IRGRP)
#   define S_IRGRP 0
#endif

/* Write group permission */
#if !defined(S_IWGRP)
#   define S_IWGRP 0
#endif

/* Execute group permission */
#if !defined(S_IXGRP)
#   define S_IXGRP 0
#endif

/* Read others permission */
#if !defined(S_IROTH)
#   define S_IROTH 0
#endif

/* Write others permission */
#if !defined(S_IWOTH)
#   define S_IWOTH 0
#endif

/* Execute others permission */
#if !defined(S_IXOTH)
#   define S_IXOTH 0
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
#define DT_UNKNOWN 0
#define DT_REG S_IFREG
#define DT_DIR S_IFDIR
#define DT_FIFO S_IFIFO
#define DT_SOCK S_IFSOCK
#define DT_CHR S_IFCHR
#define DT_BLK S_IFBLK
#define DT_LNK S_IFLNK

/* Macros for converting between st_mode and d_type */
#define IFTODT(mode) ((mode) & S_IFMT)
#define DTTOIF(type) (type)

/*
 * File type macros.  Note that block devices, sockets and links cannot be
 * distinguished on Windows and the macros S_ISBLK, S_ISSOCK and S_ISLNK are
 * only defined for compatibility.  These macros should always return false
 * on Windows.
 */
#if !defined(S_ISFIFO)
#   define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#endif
#if !defined(S_ISDIR)
#   define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif
#if !defined(S_ISREG)
#   define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISLNK)
#   define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)
#endif
#if !defined(S_ISSOCK)
#   define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
#endif
#if !defined(S_ISCHR)
#   define S_ISCHR(mode) (((mode) & S_IFMT) == S_IFCHR)
#endif
#if !defined(S_ISBLK)
#   define S_ISBLK(mode) (((mode) & S_IFMT) == S_IFBLK)
#endif

/* Return the exact length of d_namlen without zero terminator */
#define _D_EXACT_NAMLEN(p) ((p)->d_namlen)

/* Return number of bytes needed to store d_namlen */
#define _D_ALLOC_NAMLEN(p) (PATH_MAX)

#ifdef __cplusplus
extern "C" {
#endif

/* Wide-character version */
struct _wdirent {
	/* Always zero */
	long d_ino;

	/* Structure size */
	unsigned short d_reclen;

	/* Length of name without \0 */
	size_t d_namlen;

	/* File type */
	int d_type;

	/* File name */
	wchar_t d_name[PATH_MAX];
};
typedef struct _wdirent _wdirent;

struct _WDIR {
	/* Current directory entry */
	struct _wdirent ent;

	/* Private file data */
	WIN32_FIND_DATAW data;

	/* True if data is valid */
	int cached;

	/* Win32 search handle */
	HANDLE handle;

	/* Initial directory name */
	wchar_t *patt;
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
	/* Always zero */
	long d_ino;

	/* Structure size */
	unsigned short d_reclen;

	/* Length of name without \0 */
	size_t d_namlen;

	/* File type */
	int d_type;

	/* File name */
	char d_name[PATH_MAX];
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


#   include <direct.h>    // _mkdir, _rmdir
#   include <sys/utime.h> // (~) utime.h
#   include <io.h>        // (~) unistd.h
	typedef int mode_t;
#endif

// implementation

namespace apathy {

	inline int stat32( const pathfile &uri, struct stat *info ) {
		$apathy32(
		/* make win32-friendly stat() call (no trailing slashes) */
		if( uri.is_path() ) {
			path uri_ = uri;
			while( (!uri_.empty()) && uri_.back() == '/' ) {
				uri_.resize( uri_.size() - 1 );
			}
			return stat( uri_, info );
		});
		return stat( uri, info );
	}

	// size in bytes
	inline size_t size( const pathfile &uri ) {
		if( uri.is_path() ) {
			return ls(uri).size();
		}
		if( uri.is_file() ) {
			struct stat info;
			if( stat32( uri, &info ) < 0 ) {
				return 0;
			}
			return info.st_size;
		}
		return 0;
	}

	// true if exist
	inline bool exists( const pathfile &uri ) {
		if( uri.is_file() ) {
			$apathy32( return 0 == _access( uri, 0      /*04*/ ) );
			$apathyXX( return 0 ==  access( uri, F_OK /*R_OK*/ ) );
		}
		if( uri.is_path() ) {
			if( !uri.empty() ) {
				struct stat info;
				if( stat32(uri, &info) < 0 ) {
					return false;
				}
			}
			return true;
		}
		return false;
	}

	// true if directory
	inline bool is_path( const pathfile &uri ) {
		struct stat info;
		if( stat32( uri, &info ) < 0 ) {
			return false;
		}
		return S_IFDIR == ( info.st_mode & S_IFMT );
	}

	// true if file
	inline bool is_file( const pathfile &uri ) {
		struct stat info;
		if( stat32( uri, &info ) < 0 ) {
			return false;
		}
		return S_IFREG == ( info.st_mode & S_IFMT );
	}

	// true if link
	inline bool is_link( const pathfile &uri ) {
		$apathyXX(
			struct stat info;
			if( lstat( uri, &info ) < 0 ) {
				return false;
			}
			return S_IFLNK == ( info.st_mode & S_IFMT );
		);
		return false;
	}

	// true if file or directory is empty
	inline bool is_empty( const pathfile &uri ) {
		if( exists(uri) ) {
			if( uri.is_path() ) {
				return ls(uri).empty();
			}
			if( uri.is_file() ) {
				return apathy::size(uri) == 0;
			}
		}
		return false;
	}

	// access date (last time the file was read)
	inline time_t adate( const pathfile &uri ) {
		struct stat info;
		if( stat32( uri, &info ) < 0 ) {
			return 0;
		}
		return info.st_atime;
	}

	// modification date (last time file contents were modified)
	inline time_t mdate( const pathfile &uri ) {
		struct stat info;
		if( stat32( uri, &info ) < 0 ) {
			return 0;
		}
		return info.st_mtime;
	}

	// change date (last time meta data of file was changed)
	inline time_t cdate( const pathfile &uri ) {
		struct stat info;
		if( stat32( uri, &info ) < 0 ) {
			return 0;
		}
		return info.st_ctime;
	}

	// user id
	inline int uid( const pathfile &uri ) {
		struct stat info;
		if( stat32( uri, &info ) < 0 ) {
			return 0;
		}
		return info.st_uid;
	}

	// group id
	inline int gid( const pathfile &uri ) {
		struct stat info;
		if( stat32( uri, &info ) < 0 ) {
			return 0;
		}
		return info.st_gid;
	}

	// name, without path
	inline file name( const pathfile &uri ) {
		if( uri.is_path() ) {
			pathfile found = uri.substr( 0, uri.size() - 1 );
			found = found.substr( found.find_last_of('/') + 1 );
			return found != uri ? found : uri;
		}
		if( uri.is_file() ) {
			pathfile found = uri.substr( uri.find_last_of('/') + 1 );
			return found != uri ? found : uri;
		}
		return uri;
	}

	// path, without name
	inline path stem( const pathfile &uri ) {
		return uri.substr( 0, uri.size() - name(uri).size() - uri.is_path() );
	}

	// base of name
	inline file base( const file &uri_ ) {
		file uri = name(uri_);
		return uri.substr( 0, uri.size() - ext(uri).size() );
	}

	// extension of name
	inline pathfile ext( const file &uri_ ) {
		file uri = name(uri_);
		file found = uri.substr( uri.find_last_of('.') + 1 );
		return found != uri ? "." + found : "";
	}

#if APATHY_USE_MMAP
	// memory-map data from file
	inline void *map( const file &uri, size_t size, size_t offset ) {
		int fd = $apathy32(_open) $apathyXX(::open) ( uri, O_RDONLY );
		if( fd == -1 ) {
			return 0;
		}
#ifdef  __linux__
		void *ptr = mmap( (void *)0, size, PROT_READ, MAP_POPULATE | MAP_SHARED, fd, offset );
#else
		void *ptr = mmap( (void *)0, size, PROT_READ, 0 | MAP_SHARED, fd, offset );
#endif
		$apathy32(_close) $apathyXX(::close) (fd);
		if( ptr == MAP_FAILED ) {
			return 0;
		}
		return ptr;
	}

	// unmemory-map data from file
	inline void unmap( void *ptr, size_t size ) {
		munmap( ptr, size );
	}
#else
	// memory-map data from file
	inline void *map( const file &uri, size_t size, size_t offset ) {
		return 0;
	}

	// unmemory-map data from file
	inline void unmap( void *ptr, size_t size ) {
	}
#endif

	// read data from file
	inline bool read( const file &uri, std::string &buffer ) {
		struct stat info;
		if( stat32( uri, &info ) < 0 ) {
			return buffer.clear(), false;
		}
		size_t len = info.st_size;
		buffer.resize( len );
		if( len > 0 ) {
#if APATHY_USE_MMAP
			void *ptr = map( uri, len, 0 );
			if( ptr ) {
				memcpy( &buffer[0], ptr, len );
				unmap( ptr, len );
			}
			return ptr != 0;
#else
			std::ifstream ifs( uri, std::ios::in | std::ios::binary );
			ifs.read( reinterpret_cast< char * >( &buffer[0] ), len );
			return ifs.good();
#endif
		}
		return true;
	}

	// read data from file
	inline std::string read( const file &uri ) {
		std::string data;
		return read(uri, data) ? data : std::string();
	}

	// convert date to stamp
	inline std::string stamp( const std::time_t &date, const char *format ) { // defaults to mysql date format
		char buffer[128];
		struct tm *ts = localtime( &date );
		strftime(buffer, sizeof(buffer), format, ts);
		return buffer;
	}

	// change owner/group ids
	inline bool chown( const file &uri, int uid, int gid ) {
		$apathy32( return false );
		$apathyXX( return 0 == ::chown( uri, uid, gid ) );
	}

	// change permissions
	inline bool chmod( const file &uri, int mode ) {
		int fd = $apathy32(_open) $apathyXX(open) ( uri, O_RDONLY | O_CREAT, (mode_t)mode );
		if( fd < 0 ) {
			return false;
		}
		if( $apathy32(_close) $apathyXX(close) (fd) < 0 ) {
			return false;
		}
		return true;
	}

	// change modification date
	inline bool touch( const pathfile &uri, const std::time_t &date ) {
		struct stat info;
		if( stat32( uri, &info ) < 0 ) {
			return false;
		}

		struct utimbuf tb;
		tb.actime = info.st_atime;  /* keep atime unchanged */
		tb.modtime = date;          /* set mtime to given time */

		return utime( uri, &tb ) != -1 ? true : false;
	}

	// check for modifications
	inline bool touched( const pathfile &uri ) {
		static std::map< std::string, time_t > cache;
		if( cache.find( uri ) == cache.end() ) {
			cache[ uri ] = mdate( uri );
			return false;
		}

		time_t modtime = cache[ uri ];
		time_t curtime = mdate( uri );

		// fat32 minimal lapse is ~2 seconds; others filesystems are close to zero
		double diff = std::difftime( modtime, curtime );
		bool changed = ( diff > 0 ? diff : -diff ) > 0;

		if( changed ) {
			cache[ uri ] = curtime;
		}

		return changed;
	}

	// get current working directory
	inline path cwd() {
		path p;
		char *buf = getcwd(0, 0);
		if( buf ) {
			p = normalize(buf);
			free(buf);
		}
		return p;
	}

	namespace {
		std::vector<std::string> &stack() {
			static std::vector<std::string> list;
			if( list.empty() ) {
				list.push_back( cwd() );
			}
			return list;
		}
	}

	// change working directory
	inline bool cd( const path &uri ) {
		if( chdir( uri ) >= 0 ) {
			stack().back() = cwd();
			return true;
		}
		return false;
	}

	// delete directory
	inline bool rd( const path &uri ) {
		if( !exists(uri) || 0 == ::rmdir( uri ) ) {
			errno = 0;
			return true;
		}
		return false;
	}

	// create directory
	inline bool md( const path &uri, size_t mode ) {
		std::string p;
		std::vector<std::string> dirs = split( uri, '/' );
		typedef std::vector<std::string>::const_iterator iter;
		for( iter it = dirs.begin(), end = dirs.end(); it != end; ++it ) {
			const std::string &dir = *it;
			if( !dir.empty() ) {
				p += dir + "/";
				if( $apathy32(  _mkdir( p.c_str()       ) )
					$apathyXX( ::mkdir( p.c_str(), mode ) )
					< 0 ) {
					if( errno == EEXIST && is_path(p) ) {
						continue;
					}
					return false;
				}
			}
		}
		return true;
	}

	// push current working directory into stack
	inline bool pushd() {
		stack().push_back( stack().back() );
		return true;
	}

	// pop directory from stack, and return to previous one
	inline bool popd() {
		if( stack().size() > 1 ) stack().pop_back();
		return cd( stack().back() );
	}

	// check if uri matches pattern wildcard
	inline bool match( const char *uri, const char *pattern ) {
		if( *pattern=='\0' ) return !*uri;
		if( *pattern=='*' )  return match(uri, pattern+1) || (*uri && match(uri+1, pattern));
		if( *pattern=='?' )  return *uri && (*uri != '.') && match(uri+1, pattern+1);
		return (*uri == *pattern) && match(uri+1, pattern+1);
	}

	// glob items from disk, with options
	template<typename T, typename INSERTER>
	inline size_t glob( T &out, const INSERTER &insert, const path &uri, const std::vector<std::string> &masks, bool recursive, bool skip_dotdirs ) {
		size_t count = 0;
		std::vector<std::string>::const_iterator it, end = masks.end();
		for( DIR *dir = opendir( uri.empty() ? "./" : uri.c_str() ); dir; closedir(dir), dir = 0 ) {
			for( struct dirent *ent = readdir(dir); ent ; ent = readdir(dir) ) {
				bool ignored = ent->d_name[0] == '.' && ( ent->d_name[1] == 0 || ent->d_name[1] == '.' ); // skip ./ ../
				bool skipped = ent->d_name[0] == '.' && skip_dotdirs;                                     // skip .hg/ .git/ [...]
				if( !ignored && !skipped ) {
					bool is_path = ent->d_type == DT_DIR;
					bool is_file = ent->d_type == DT_REG; // Also, DT_LNK, DT_SOCK, DT_FIFO, DT_CHR, DT_BLK
					if( is_path || is_file ) {
						std::string full = uri + ent->d_name + (is_path ? "/" : "");
						if( masks.empty() ) {
							insert( out, full, !!is_path );
							++count;
						} else {
							for( it = masks.begin(); it != end; ++it ) {
								if( match(full.c_str(), it->c_str()) ) {
									insert( out, full, !!is_path );
									++count;
									break;
								}
							}
						}
						if( is_path && recursive ) {
							count += glob( out, insert, full, masks, recursive, skip_dotdirs );
						}
					}
				}
			}
		}
		return count;
	}

	// specialized globber
	inline size_t glob( std::map<std::string, bool> &out, const path &uri, const std::vector<std::string> &masks, bool recursive, bool skip_dotdirs ) {
		struct inserter {
			void operator()( std::map<std::string, bool> &out, const std::string &uri, bool is_dir ) const {
				out[ uri ] = is_dir;
			}
		};
		return glob( out, inserter(), uri, masks, recursive, skip_dotdirs );
	}

	// specialized globber
	inline size_t glob( std::map<std::string, bool> &out, const path &uri, const std::string &masks, bool recursive, bool skip_dotdirs ) {
		struct inserter {
			void operator()( std::map<std::string, bool> &out, const std::string &uri, bool is_dir ) const {
				out[ uri ] = is_dir;
			}
		};
		return glob( out, inserter(), uri, wildcards(masks), recursive, skip_dotdirs );
	}

	// specialized globber
	inline size_t glob( std::vector<std::string> &out, const path &uri, const std::vector<std::string> &masks, bool recursive, bool skip_dotdirs ) {
		struct inserter {
			void operator()( std::vector<std::string> &out, const std::string &uri, bool is_dir ) const {
				out.push_back( uri );
			}
		};
		return glob( out, inserter(), uri, masks, recursive, skip_dotdirs );
	}

	// specialized globber
	inline size_t glob( std::vector<std::string> &out, const path &uri, const std::string &masks, bool recursive, bool skip_dotdirs ) {
		struct inserter {
			void operator()( std::vector<std::string> &out, const std::string &uri, bool is_dir ) const {
				out.push_back( uri );
			}
		};
		return glob( out, inserter(), uri, wildcards(masks), recursive, skip_dotdirs );
	}

	// overwrite data into file
	inline bool overwrite( const file &uri, const void *data, size_t size ) {
		std::ofstream ofs( uri, std::ios::out|std::ios::binary|std::ios::trunc );
		if( ofs.is_open() && ofs.good() ) {
			ofs.write( reinterpret_cast<const char *>( data ), size );
		}
		return !ofs.fail();
	}

	// overwrite data into file
	inline bool overwrite( const file &uri, const std::string &content ) {
		return overwrite( uri, content.c_str(), content.size() );
	}

	// append data to file
	inline bool append( const file &uri, const void *data, size_t size ) {
		std::fstream ofs( uri, std::ios::out|std::ios::binary|std::ios::app|std::ios::ate );
		if( ofs.is_open() && ofs.good() ) {
			ofs.write( reinterpret_cast<const char *>( data ), size );
		}
		return !ofs.fail();
	}

	// append data to file
	inline bool append( const file &uri, const std::string &content ) {
		return append( uri, content.c_str(), content.size() );
	}

	// resize file to size
	inline bool resize( const file &uri, size_t new_size ) {
		bool ok = false;
		FILE *fp = fopen(uri, "a+b");
		if( fp ) {
			int fd = fileno(fp);
			if( fd != -1 ) {
				$apathyXX(
					ok = 0 == ftruncate( fd, (off_t)new_size );
				)
				$apathy32(
					ok = 0 == _chsize_s( fd, new_size );
				)
			}
			fflush(fp);
			fclose(fp);
		}
		return ok;
	}

	// directory listing
	inline bool ls( std::vector<std::string> &list, const path &uri, const std::string &masks ) {
		return glob( list, uri, wildcards(masks), false, false ) > 0;
	}

	// directory listing
	inline std::vector<std::string> ls( const path &uri, const std::string &masks ) {
		std::vector<std::string> list;
		return ls( list, uri, masks ) ? list : (list.clear(), list);
	}

	// directory listing (recursive)
	inline bool lsr( std::vector<std::string> &list, const path &uri, const std::string &masks ) {
		return glob( list, uri, wildcards(masks),  true, false ) > 0;
	}

	// directory listing (recursive)
	inline std::vector<std::string> lsr( const path &uri, const std::string &masks ) {
		std::vector<std::string> list;
		return lsr( list, uri, masks ) ? list : (list.clear(), list);
	}

	// file op

	// /!\ remove file or directory /!\.
	inline bool rm( const pathfile &uri ) {
		for( int i = 0; i < 512; ++i ) {
			if( !exists(uri) || 0 == std::remove( uri ) || 0 == ::rmdir( uri )) {
				errno = 0;
				return true;
			}
			sleep(0.001);
		}
		return false;
	}

	// /!\ remove file or directory, recursively /!\.
	inline bool rmrf( const pathfile &uri ) {
		bool ok = true;
		if( uri.is_path() ) {
			std::vector<std::string> list( ls(uri) );
			std::vector<pathfile> subdirs( list.begin(), list.end() );
			typedef std::vector<pathfile>::const_iterator iter;
			for( iter it = subdirs.begin(), end = subdirs.end(); it != end; ++it ) {
				if( it->is_path() && !rmrf(*it) ) {
					ok = false; // "rmdirs failed"
				} else if( it->is_file() && !rm(*it) ) {
					ok = false; // "remove failed"
				}
			}
		}
		return rm(uri) && ok;
	}

	// move to a different location
	inline bool mv( const pathfile &uri, const pathfile &uri_dst) {
		if( !exists(uri) ) {
			return false;
		}
		if( 0 == std::rename(uri, uri_dst) ) {
			return true;
		}
		if( errno == ENOENT ) {
			md( uri_dst.is_file() ? stem(uri_dst) : path(uri_dst) );
			return std::rename(uri, uri_dst) == 0;
		}
		return false;
	}

	// copy to a different location
	inline bool cp( const pathfile &uri, const pathfile &uri_dst) {
		std::string data;
		if( !read(uri, data) ) {
			return false;
		}
		if( overwrite(uri_dst, data) ) {
			return true;
		}
		if( errno == ENOENT ) {
			if( !md( uri_dst.is_file() ? stem(uri_dst) : path(uri_dst) ) ) {
				return false;
			}
			return overwrite(uri_dst, data);
		}
		return false;
	}

	// patch file
	inline bool patch( const file &uri, const std::string &patchdata ) {
		bool success = false;
		if( patchdata.size() ) {
			file tempfile = uri + ".bak"; // @todo: use tmpname()
			// try to remove previous attempts
			rm( tempfile );
			// try to rename & patch
			if( mv( uri, tempfile ) ) {
				if( overwrite( uri, patchdata ) ) {
					success = true;
				} else {
					// try to rollback
					if( !mv( tempfile, uri ) ) {
						//argh! what do we do now? :(
					}
				}
			}
			rm( tempfile );
		}
		return success;
	}

	// returns temp dir name (does not create directory)
	inline path tmpdir() {
		struct testdir {
			static bool test_tempdir( const std::string &temp_dir ) {
				file fp( temp_dir + "/tst-tmp.XXXXXX" );
				if( mktemp((char *)fp.c_str()) >= 0 ) {
					if( overwrite(fp, "!") ) {
						return true;
					}
				}
				return false;
			}
			static std::string find() {
				const char *dirs[] = { "/tmp", "/usr/tmp", "/var/tmp" $apathy32(,"c:/temp")
#               ifdef P_tmpdir
				,P_tmpdir
#               endif
				};
				const char *envs[] = { "TMP", "TMPDIR", "TEMP" };
				std::string norm;
				for( int i = 0; i < (sizeof(envs) / sizeof(const char *)); i++ ) {
					if( const char *env = getenv(envs[i]) ) {
						norm = normalize(env);
						if( test_tempdir(norm) ) {
							return norm;
						}
					}
				}
				for( int i = 0; i < (sizeof(dirs) / sizeof(const char *)); i++ ) {
					norm = normalize(dirs[i]);
					if( test_tempdir( norm ) ) {
						return norm;
					}
				}
				return std::string();
			}
		};
		static const path st( testdir::find() );
		return st;
	}

	// returns temp file name (does not create file)
	inline file tmpname() {
		$apathyXX(
			file fp( "tst-tmp.XXXXXX" );
			if( mktemp((char *)fp.c_str()) >= 0 ) {
			}
			return name( fp );
		)
		$apathy32(
			return name( normalize( std::tmpnam(0) ) );
		)
	}

	// returns last error string
	inline std::string why() {
		return strerror(errno);
	}

	// utils {
	inline std::string normalize( const std::string &str_ ) {
		std::string str( str_ );
		typedef std::string::iterator iter;
		for( iter it = str.begin(), end = str.end(); it != end; ++it ) {
			char &ch = *it;
			if( ch == '\\' ) ch = '/';
		}
		return str;
	}

	inline std::string replace( const std::string &str_, const std::string &from, const std::string &to ) {
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

	inline std::vector<std::string> split( const std::string &str, char sep ) {
		std::stringstream ss( str );
		std::vector<std::string> vector;
		for( std::string join; std::getline( ss, join, sep ); vector.push_back( join ) )
		{}
		return vector;
	}

	inline std::vector<std::string> wildcards( const std::string &mask ) {
		return split( replace( replace( replace( mask, "|", "\1" ), ";", "\1" ), ",", "\1" ), '\1' );
	}

	inline void sleep( double t ) {
		$apathy32(  Sleep(  int(t * 1000 ) ) );
		$apathyXX( usleep( t * 1000 * 1000 ) );
	}
	// } utils
}

#ifdef APATHY_BUILD_TESTS

// unittest suite
#define suite(...) if(printf("------ " __VA_ARGS__),puts(""),true)
#define test(...)  (errno=0,++tst,err+=!(ok=!!(__VA_ARGS__))),printf("[%s] %d %s (%s)\n",ok?" OK ":"FAIL",__LINE__,#__VA_ARGS__,strerror(errno))
unsigned tst=0,err=0,ok=atexit([]{ suite("summary"){ printf("[%s] %d tests, %d passed, %d errors\n",err?"FAIL":" OK ",tst,tst-err,err); }});

// benchmark suite
#include <chrono>
static double now() {
	static auto const epoch = std::chrono::steady_clock::now(); // milli ms > micro us > nano ns
	return std::chrono::duration_cast< std::chrono::microseconds >( std::chrono::steady_clock::now() - epoch ).count() / 1000000.0;
}
template<typename T> double bench_s ( const T &t ) { auto t0 = -now(); return (t(), t0+now()); }
template<typename T> double bench_ms( const T &t ) { return bench_s( t ) * 1000.0; }
#define benchmark(...) printf("[ OK ] %d %gms %s\n", __LINE__, bench_ms([&]{ __VA_ARGS__ ;}), #__VA_ARGS__ )

int main() {
	using namespace apathy;

	suite( "test utils" ) {
		test( normalize("/\\/") == "///" );
		test( match("blabla/image.bmp", "*.bmp" ) );
		test( replace("blbl", "bl", "bla") == "blabla" );
		test( split("/bla/bla/", '/') == std::vector<std::string> { "", "bla", "bla" } );
		test( wildcards("*.bmp;*.png|*.jpg") == std::vector<std::string> { "*.bmp", "*.png", "*.jpg" } );
		read( "nonexisting" );
		test( !why().empty() );
	}

	suite( "test path, file and pathfile classes" ) {
		file file("image.bmp");
		path empty;
		path bin("bin");
		path binusr("bin/usr");
		path folder("folder");

		test( file == "image.bmp" );
		test( empty == "" );
		test( bin == "bin/" );
		test( bin/bin == "bin/bin/" );
		test( binusr/folder/"user"/file == "bin/usr/folder/user/image.bmp" );
	}

	suite( "test file infos" ) {
		auto self = normalize(__FILE__);
		test( exists(self) );
		test( apathy::size(self) > 0 );
		test( !is_empty(self) );
		test(  is_file(self) );
		test( !is_path(self) );
		test( !is_link(self) );

		$apathyXX(
		test( gid(self) > 0 );
		test( uid(self) > 0 );
		)

		test( adate(self) > 0 );
		test( cdate(self) > 0 );
		test( mdate(self) > 0 );

		test( self == stem(self) + name(self) );
		test( self == stem(self) + base(self) + ext(self) );

		test( base(self) == "apathy" );
		test( name(self) == "apathy.hpp" );
		test(  ext(self) == ".hpp" );

		test(  base("LICENSE") == "LICENSE" );
		test(  base("LICENSE.md") == "LICENSE" );
		test(   ext("LICENSE") == "" );
		test(   ext("LICENSE.md") == ".md" );
	};

	suite( "test path infos" ) {
		path self = "././";
		test( exists(self) );
		test( apathy::size(self) > 0 );
		test( !is_empty(self) );
		test( !is_file(self) );
		test(  is_path(self) );
		test( !is_link(self) );

		$apathyXX(
		test( gid(self) > 0 );
		test( uid(self) > 0 );
		)

		test( adate(self) > 0 );
		test( cdate(self) > 0 );
		test( mdate(self) > 0 );

		test( self == stem(self) + name(self) + "/" );

		test( stem(self) == "./" );
		test( name(self) == "." );
	};

	suite( "test create/rm regular files" ) {
		std::string file = "$tmp1";
		test( !exists(file) );
		test( overwrite(file, "hello") );
		test( exists(file) );
		test(  is_file(file) );
		test( !is_path(file) );
		test( !is_link(file) );
		test( read(file) == "hello" );
		test( append(file, "hello") );
		test( exists(file) );
		test( read(file) == "hellohello" );
		test( rm(file) );
		test( !exists(file) );
	}

	suite( "test md/rm regular paths" ) {
		std::string path = "$tmp1/";
		test( !exists(path) );
		test( md(path) );
		test( exists(path) );
		test( rm(path) );
		test( !exists(path) );
	}

	suite( "test cp/mv/rmrf operations") {
		file f = "$tmp1";
		path p = "tests/";

		test( !exists( f ) );
		test( overwrite(f, "test") );

		test( exists( f ) );
		test( !exists(p) );
		test( !exists(p/p) );
		test( !exists(p/p/f) );
		test( cp(f, p/p/f) );
		test( exists(f) );
		test( exists(p) );
		test( exists(p/p) );
		test( exists(p/p/f) );
		test( read(p/p/f) == "test" );
		test( rmrf(p) );
		test( !exists(p) );
		test( !exists(p/p) );
		test( !exists(p/p/f) );

		test( exists( f ) );
		test( mv(f, p/p/f) );
		test( !exists(f) );
		test( exists(p) );
		test( exists(p/p) );
		test( exists(p/p/f) );
		test( read(p/p/f) == "test" );

		test( exists(p) );
		test( rmrf(p) );
		test( !exists(p) );
		test( !exists(p/p) );
		test( !exists(p/p/f) );
	}

	suite( "resize operation" ) {
		file f = "$tmp1";
		test( !exists(f) );
		test( resize(f, 1000) );
		test( exists(f) );
		test( apathy::size(f) == 1000 );
		test( overwrite(f, "hello12hello12") );
		test( resize(f, 5) );
		test( apathy::size(f) == 5 );
		test( read(f) == "hello" );
		test( rm(f) );
		test( !exists(f) );
	}

	suite( "test cd/md/rd/pushd/popd/cwd dir stack" ) {
		path dir = cwd();
		path subdir = "tests/";
		test( md(subdir) );
		test( cd(subdir) );
		test( dir/subdir == cwd() );
		test( cd("../") );
		test( rd(subdir) );
		test( dir == cwd() );

		test( pushd() );
		test( md(subdir/subdir) );
		test( cd(subdir/subdir) );
		test( popd() );
		test( dir == cwd() );
		test( rd(subdir/subdir) );
		test( rd(subdir) );
	}

	suite( "test touch/modification date" ) {
		auto self = normalize(__FILE__);
		test( exists(self) );
		test( apathy::size(self) > 0 );

		test( !touched(self) );
		test(  touch(self) );
		test(  touched(self) );
		test( !touched(self) );
		test( !touched(self) );
	}

	suite( "test tmpdir" ) {
		test( tmpdir().back() == '/' );
		test( tmpdir() != "" );
		test( tmpdir() != "./" );
		test( overwrite(tmpdir()/"apathy_was_here.txt", "hello") );
		test( !exists("./apathy_was_here.txt") );
		test( read(tmpdir()/"apathy_was_here.txt") == "hello" );
	}

	suite( "test tmpfiles" ) {
		test( tmpname().back() != '/' );
		std::string tmp1 = tmpname();
		test( tmp1.size() > 0 );
		test( !exists(tmp1) );
		test( overwrite(tmp1, "hello") );
		test( exists(tmp1) );

		std::string tmp2 = tmpname();
		test( tmp2.size() > 0 );
		test( !exists(tmp2) );
		test( overwrite(tmp2, "hello") );
		test( exists(tmp2) );

		test( tmp1 != tmp2 );

		test(  is_file(tmp1) );
		test( !is_path(tmp1) );
		test( !is_link(tmp1) );
		test( read(tmp1) == "hello" );
		test( read(tmp2) == "hello" );
		test( append(tmp1, "hello") );
		test( append(tmp2, "hello") );
		test( read(tmp1) == "hellohello" );
		test( read(tmp2) == "hellohello" );
		test( rm(tmp1) );
		test( rm(tmp2) );
		test( !exists(tmp1) );
		test( !exists(tmp2) );
	}

	suite( "test imstream") {
		std::string data = read(normalize(__FILE__));

		imstream membuf( data.c_str(), data.size() );
		std::istream in(&membuf);

		test( in.good() );

		std::stringstream ss;
		test( ss << in.rdbuf() );

		test( ss.str() == data );
		test( in.good() );
	}

	suite( "test file/dir globbing" ) {
		auto list1 = ls( "", "*.cc;*.hpp;*.md" );
		test( list1.size() > 0 );
		auto list2 = ls( "./", "*.hpp;" );
		test( list2.size() == 1 );
	}

	suite( "benchmark disk globbing" ) {
		auto root = ls(), subs = ls();
		benchmark(
			root = ls("./");
		);
		benchmark(
			subs = lsr("./");
		);
		test( !root.empty() );
		test( !subs.empty() );
		test( subs.size() > root.size() );
		std::cout << subs.size() << " files found" << std::endl;
	}
}

#endif

#undef $apathy32
#undef $apathyXX

