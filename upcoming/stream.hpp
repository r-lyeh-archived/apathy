class stream {
    char *begin = 0, *end = 0, *cursor = 0;

    bool open() {
        cursor = begin;
        return cursor != end;
    }
    bool close() {
        cursor = end;
        return cursor != begin;
    }
    bool rewind() {
        cursor = begin;
        return cursor != end;
    }
    bool eof() {
        return cursor >= end;
    }
    unsigned tell() {
        return cursor - begin;
    }
    unsigned size() {
        return end - begin;
    }
    bool seek( unsigned offs ) {
        cursor = begin + amount;
        return cursor >= begin && cursor < end;
    }
    bool offset( int amount ) {
        return seek( tell() + amount );
    }
    bool read( char *ptr, unsigned N ) {
        memcpy( ptr, cursor, N );
        return offset( N );
    }
    bool write( char *ptr, unsigned N ) {
        memcpy( cursor, ptr, N );
        return offset( N );
    }
};




    // stream { void *begin, *end, *cursor }
    // push
    // pop
    // read,
    // overwrite, append
    // compress, uncompress
    // mmap, unmmap
