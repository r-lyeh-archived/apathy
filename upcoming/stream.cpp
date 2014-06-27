#include "giant/giant.hpp"

#include <string.h>
#include <stdint.h>
#include <string>
#include <cctype>
#include <vector>

struct stream {
    const char *begin = 0, *end = 0, *cursor = 0, *rw = 0;

    std::vector< stream > stack;
    void push() {
        stack.push_back( *this );
    }
    void pop() {
        *this = stack.back();
        stack.pop_back();
    }

    stream()
    {}

    stream( const void *from, const void *to ) {
        begin = (const char *)( from < to ? from : to);
        end = (const char *)( to > from ? to : from);
        open();
    }
    stream( const void *ptr, unsigned len ) {
        begin = (const char *)ptr;
        end = begin + len;
        open();
    }
    stream( void *ptr, unsigned len ) : rw(rw+1) {
        begin = (const char *)ptr;
        end = begin + len;
        open();
    }

    bool good( unsigned off = 0 ) {
        return cursor && cursor >= begin && cursor + off < end;
    }

    bool open() {
        cursor = (char *)begin;
        return good();
    }
    bool close() {
        cursor = (char *)end;
        // flush to disk/net here
        return good();
    }

    bool rewind() {
        cursor = (char *)begin;
        return good();
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
    unsigned left() {
        return size() - tell();
    }
    bool seek( unsigned offset ) {
        cursor = (char *)(begin + offset);
        return good();
    }
    bool offset( int offset ) {
        cursor += offset;
        return good();
    }

    bool read( void *ptr, unsigned N ) {
        const unsigned top = left(), max = N < top ? N : top;
        return ptr && good(max) && memcpy( ptr, ( cursor += max ) - max, max );
    }
    bool write( const void *ptr, unsigned N ) {
        const unsigned top = left(), max = N < top ? N : top;
        return rw && ptr && good(max) && memcpy( (char *)( ( cursor += max ) - max ), ptr, max );
    }

    template<typename T>
    bool read( T &t ) {
        return read( &t, sizeof t );
    }
    template<typename T>
    bool write( const T &t ) {
        return write( &t, sizeof t );
    }

    bool read8( int8_t &c ) {
        return read( c ) ? true : false;
    }
    bool read16( int16_t &c ) {
        return read( c ) ? giant::letoh( c ), true : false;
    }
    bool read32( int32_t &c ) {
        return read( c ) ? giant::letoh( c ), true : false;
    }
    bool read64( int64_t &c ) {
        return read( c ) ? giant::letoh( c ), true : false;
    }
    bool write8( const int8_t &c ) {
        return write( c );
    }
    bool write16( const int16_t &c ) {
        return write( giant::htole(c) );
    }
    bool write32( const int32_t &c ) {
        return write( giant::htole(c) );
    }
    bool write64( const int64_t &c ) {
        return write( giant::htole(c) );
    }
    int getc() {
        int8_t n;
        return read8(n) ? n : -1;
    }

    template<typename ostream>
    inline friend ostream &operator<<( ostream &os, const stream &self ) {
        const std::string str( self.begin, self.end - self.begin );
        std::string text, text1, text2;
        char buf[ 80 ];

        for( unsigned W = 16, L = 4, line = 0, at; at = line * W, line++ < L && at < str.size(); ) {
            text1 = ( std::sprintf( buf, "<%p+%02x> ", &self, at ), buf );
            text2 = ( buf[0] = '\0', buf);

            for( unsigned count = line * W < str.size() ? line * W : str.size(); at < count; ++at ) {
                char ch = str[at];
                text1 = text1 + ( std::sprintf( buf, "%02X ", (unsigned char)ch ), buf );
                text2 = text2 + ( std::isprint(ch) && ch != 32 ? ch : '.' ) + " ";
            }

            for( ; at < line * W; ++at ) {
                text1 += "?? ";
                text2 +=  ". ";
            }

            text += text1 + ' ' + text2 + '\n';
        }

        return os << text, os;
    }
};

#include <cassert>
#include <iostream>

int main() {
    stream s;
    assert( !s.open() );
    assert( !s.close() );
    assert( s.size() == 0 );
    assert( s.tell() == 0 );
    assert( s.left() == 0 );
    assert( s.eof() == true );
    assert( s.rewind() == false );
    assert( s.offset(0) == false );
    assert( s.seek(0) == false );
    assert( s.offset(~0) == false );
    assert( s.seek(~0) == false );
    int i;
    assert( s.read(i) == false );
    assert( s.write(i) == false );

    {
        const char *hello_world = "hello world";
        s = stream( hello_world, strlen(hello_world) );
        assert( s.open() );
        assert(!s.write32('1234') );
        assert( std::string(hello_world) == "hello world" );
    }
    {
        char hello_world[] = "hello world";
        s = stream( hello_world, strlen(hello_world) );
        assert( s.open() );
        int32_t val = '1234';
        assert( s.write32(val) ); // host to le
        assert( std::string(hello_world) == "4321o world" || std::string(hello_world) == "1234o world" );
        assert( s.rewind() );
        assert( s.read(val) );
        assert( val == '1234' );
    }
    {
        std::string h( "son of a " );
        for (int i = 0; i < 2; ++i) h += h;
        s = stream( &h[0], h.size() );
        std::cout << s << std::endl;
    }

}

// push
// pop
// read,
// overwrite, append
// compress, uncompress
// mmap, unmmap
