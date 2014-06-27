#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <sys/mman.h>
#endif

#include <stdio.h>

void ReserveBottomMemory()
{
#ifdef _WIN64
    static bool s_initialized = false;
    if ( s_initialized )
        return;
    s_initialized = true;

    // Start by reserving large blocks of address space, and then
    // gradually reduce the size in order to capture all of the
    // fragments. Technically we should continue down to 64 KB but
    // stopping at 1 MB is sufficient to keep most allocators out.

    const size_t LOW_MEM_LINE = 0x100000000LL;
    size_t totalReservation = 0;
    size_t numVAllocs = 0;
    size_t numHeapAllocs = 0;
    size_t oneMB = 1024 * 1024;
    for (size_t size = 256 * oneMB; size >= oneMB; size /= 2)
    {
        for (;;)
        {
            void* p = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
            if (!p)
                break;

            if ((size_t)p >= LOW_MEM_LINE)
            {
                // We don't need this memory, so release it completely.
                VirtualFree(p, 0, MEM_RELEASE);
                break;
            }

            totalReservation += size;
            ++numVAllocs;
        }
    }

    // Now repeat the same process but making heap allocations, to use up
    // the already reserved heap blocks that are below the 4 GB line.
    HANDLE heap = GetProcessHeap();
    for (size_t blockSize = 64 * 1024; blockSize >= 16; blockSize /= 2)
    {
        for (;;)
        {
            void* p = HeapAlloc(heap, 0, blockSize);
            if (!p)
                break;

            if ((size_t)p >= LOW_MEM_LINE)
            {
                // We don't need this memory, so release it completely.
                HeapFree(heap, 0, p);
                break;
            }

            totalReservation += blockSize;
            ++numHeapAllocs;
        }
    }

    // Perversely enough the CRT doesn't use the process heap. Suck up
    // the memory the CRT heap has already reserved.
    for (size_t blockSize = 64 * 1024; blockSize >= 16; blockSize /= 2)
    {
        for (;;)
        {
            void* p = malloc(blockSize);
            if (!p)
                break;

            if ((size_t)p >= LOW_MEM_LINE)
            {
                // We don't need this memory, so release it completely.
                free(p);
                break;
            }

            totalReservation += blockSize;
            ++numHeapAllocs;
        }
    }

    // Print diagnostics showing how many allocations we had to make in
    // order to reserve all of low memory, typically less than 200.
    char buffer[1000];
    sprintf_s(buffer, "Reserved %1.3f MB (%d vallocs,"
                      "%d heap allocs) of low-memory.\n",
            totalReservation / (1024 * 1024.0),
            (int)numVAllocs, (int)numHeapAllocs);
    OutputDebugStringA(buffer);
#endif
}

HANDLE fm;

char* memmap( int fd, unsigned size = 4096 ) {
  /* create tmp file and get file descriptor */
  char *bp; // base pointer

#ifdef WIN32
    HANDLE h = (HANDLE) _get_osfhandle (fd);

    fm = CreateFileMapping(
             h,
             NULL,
             PAGE_READWRITE, // | SEC_RESERVE, // | SEC_COMMIT
             0,
             size,
             NULL);
    if (fm == NULL) {
            fprintf (stderr, "Couldn't access memory space! %s (%d)\n", strerror (GetLastError()), GetLastError());
            exit(GetLastError());
    }
    bp = (char*)MapViewOfFile(
              fm,
              FILE_MAP_ALL_ACCESS,
              0,
              0,
              0);
    if (bp == NULL) {
            fprintf (stderr, "Couldn't fill memory space! %s (%d)\n", strerror (GetLastError()), GetLastError());
            exit(GetLastError());
    }
#else
    bp = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_PRIVATE|MAP_NORESERVE, fd, 0);
    if (bp == MAP_FAILED) {
            fprintf (stderr, "Couldn't access memory space! %s\n", strerror (errno));
            exit(errno);
    }
#endif
    return bp;
}

char* memmap( FILE *fp, unsigned size = 4096 ) {
  return memmap( fileno(fp), size );
}

void memunmap( char *p, unsigned size ) {
#ifdef _WIN32
  UnmapViewOfFile(p);
  CloseHandle(fm);
#else
  munmap( p, size );
#endif
}

void flush( FILE *fp ) {
    fflush(fp);
//    rewind(fp);
}

void flush( int fd ) {
    FILE *fp = fdopen(fd, "w");
    return flush(fp);
}

#ifdef _WIN32
int truncate( int fd, __int64 size ) {
  return _chsize( fd, size);
}
int ftruncate( FILE *fp, __int64 size ) {
  return _chsize( fileno(fp), size);
}
#endif

/*
void *append(int fd, char const *data, size_t nbytes, void *map, size_t &len)
{
    // TODO: check for errors here!
    ssize_t written = write(fd, data, nbytes);
    munmap(map, len);
    len += written;
    return mmap(NULL, len, PROT_READ, 0, fd, 0);
} */

int main( int argc, char **argv ) {

  ReserveBottomMemory();

  FILE *fp = fopen("$", "wb+"); // tmpfile();

  char *bp = memmap( fp, 32768 );

#if 1
  char *table = (char *)malloc(32768); memset(table,'L',32768);
  memcpy(bp, table, 32768 );
  bp[16384] = '\0';
//  sprintf(bp, "hello world");
#else
  fprintf(fp, "hello world" );
  fprintf(fp, "hello world" );
  flush(fp);
#endif

  printf("%s", bp);

  ftruncate(fp, strlen(bp)+1 );

  return 0;
}
