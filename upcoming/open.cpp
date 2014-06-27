#include <iostream>
#include <string>
#include <stdio.h>
#include <fstream>

namespace fs {
    int open( const char * name, const char * mode ) {
        return 0;
    }
    int close( int fp ) {
        return 0 != fp;
    }
};

extern "C" int ffopen( const char * name, const char * mode ) {
    return fs::open( name, mode );
}
extern "C" int ffclose( int fp ) {
    return fs::close( fp );
}


// process << fopen << fseek << ftell << fclose
// if( process(data) ) ok

// symbols: fsopen, fsclose, fsseek, fstell, fsread, fswrite
// stream[name] << code >> code




int main( int argc, char **argv ) {
    for( FILE *fp = fopen("open.cpp", "rb"); fp; fclose(fp), fp = 0 ) {
        std::cout << "fopen ok" << std::endl;
    }
    for( int fp = ffopen("open.cpp", "rb"); fp; ffclose(fp), fp = 0 ) {
        std::cout << "ffopen ok" << std::endl;
    }
}
