#ifndef BTREE_COMPAT_H
#define BTREE_COMPAT_H

#if defined(_MSC_VER) && _MSC_VER < 1600
#include "pstdint.h"
#else
#include <stdint.h>
#endif

#ifdef __CHECKER__
#define FORCE           __attribute__((force))
#else
#define FORCE
#endif

#ifdef __CHECKER__
#define BITWISE         __attribute__((bitwise))
#else
#define BITWISE
#endif

#ifdef _MSC_VER
#define PACKED( ... ) __pragma( pack(push, 1) ) __VA_ARGS__ __pragma( pack(pop) )
#else
#define PACKED( ... ) __VA_ARGS__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define INLINE
#else
#define INLINE inline
#endif

#ifdef _WIN32
	#if defined(_FILE_OFFSET_BITS) && (_FILE_OFFSET_BITS == 64)
	#define off_t uint64_t
	#else
	#define off_t uint32_t
	#endif
	#define __be16 uint16_t
	#define __be32 uint32_t
	#define __be64 uint64_t
#else
	#include <bits/types.h>
#endif

#include <stdio.h>

char* memmap( int fd );
char* fmemmap( FILE *fp );
void flush( int fd );
//void fflush( FILE *fp );

#endif

#ifdef BTREE_COMPAT_EXTRA_DEFINES
	#ifdef _WIN32
	#include <winsock.h>
	#pragma comment(lib, "ws2_32.lib")
	#else
	/* Unix */
	#include <unistd.h>
	#include <arpa/inet.h> /* htonl/ntohl */
	#define O_BINARY 0
	#endif

	#ifdef _MSC_VER
	#define ssize_t signed
	#define open64 open
	#define lseek64 lseek
	#define fdatasync(fd) do { FILE *fp = fdopen(fd, "w"); fflush( fp ); /*fclose(fp);*/ } while(0)
	#endif
#endif
