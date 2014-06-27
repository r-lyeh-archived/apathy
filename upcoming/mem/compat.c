#include "compat.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <sys/mman.h>
#endif

#include <stdio.h>

char* memmap( int fd ) {
  /* create tmp file and get file descriptor */
  char *bp;

#ifdef WIN32
    HANDLE fm;
    HANDLE h = (HANDLE) _get_osfhandle (fd);

    fm = CreateFileMapping(
             h,
             NULL,
             PAGE_READWRITE|SEC_RESERVE,
             0,
             16384,
             NULL);
    if (fm == NULL) {
            fprintf (stderr, "Couldn't access memory space! %s\n", strerror (GetLastError()));
            exit(GetLastError());
    }
    bp = (char*)MapViewOfFile(
              fm,
              FILE_MAP_ALL_ACCESS,
              0,
              0,
              0);
    if (bp == NULL) {
            fprintf (stderr, "Couldn't fill memory space! %s\n", strerror (GetLastError()));
            exit(GetLastError());
    }
#else
    bp = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_FILE|MAP_PRIVATE, fd, 0);
    if (bp == MAP_FAILED) {
            fprintf (stderr, "Couldn't access memory space! %s\n", strerror (errno));
            exit(errno);
    }
#endif
    return bp;
}

char* fmemmap( FILE *fp ) {
  return memmap( fileno(fp) );
}

/*void fflush( FILE *fp ) {
    fflush(fp);
//    rewind(fp);
}*/

void flush( int fd ) {
    FILE *fp = fdopen(fd, "w");
    fflush(fp);
}

#if 1

int main( int argc, char **argv ) {

#if 1
  // this does work...
  FILE *fp = fopen("$", "wb+");
#else
  // so does this too...
  FILE *fp = tmpfile();
#endif

  char *bp = fmemmap( fp );

#if 1
  // this does work...
  sprintf(bp, "hello world");
#else
  // so does this too...
  fprintf(fp, "hello world" );
  fprintf(fp, "hello world" );
  flush(fp);
#endif

  printf("%s", bp);
  return 0;
}

#endif
