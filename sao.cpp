/*
 * Sao is a lightweight IO C++11 library with no dependencies.
 * Copyright (c) 2011,2012,2013 Mario 'rlyeh' Rodriguez

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
 * add stream() << >> chunk()
 * or
 * add subread( from, to ), subwrite( from, to ), chunk(), next(), tell(),

 * - rlyeh ~~ listening to Alice in chains / Nutshell
 */

#if defined(_WIN32) || defined(_WIN64)
#   include <winsock2.h>
#   include <windows.h>
#   include <ctime>
#   include <sys/stat.h>
#   include <sys/utime.h>
#   define $win32(...) __VA_ARGS__
#   define $welse(...)
#else
#   include <sys/time.h>
#   include <sys/stat.h>
#   include <utime.h>
#   define $win32(...)
#   define $welse(...) __VA_ARGS__
#endif

#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "sao.hpp"

// api

namespace sao
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

            sbb( void (*cbb)( bool, bool, bool, const std::string & ) )
            {
                insert( cbb );
            }

            ~sbb()
            {
                clear();
            }

            void log( const std::string &line )
            {
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

            virtual int_type overflow( int_type c = traits_type::eof() )
            {
                log( std::string() + (char)(c) );

                return 1;
            }

            virtual std::streamsize xsputn( const char *c_str, std::streamsize n )
            {
                log( std::string( c_str, (unsigned)n ) );

                return n;
            }

            void clear()
            {
                for( set::iterator jt = cb.begin(), jend = cb.end(); jt != jend; ++jt )
                    (**jt)( false, false, true, std::string() );
                cb.clear();
            }

            void insert( proc p )
            {
                if( !p )
                    return;

                // make a dummy call to ensure any static object of this callback are deleted after ~sbb() call (RAII)
                p( 0, 0, 0, std::string() );
                p( true, false, false, std::string() );

                // insert into map
                cb.insert( p );
            }

            void erase( proc p )
            {
                p( false, false, true, std::string() );
                cb.erase( p );
            }
        };
    }
}

namespace
{
    struct captured_ostream
    {
        std::streambuf *copy;
        sao::detail::sbb sb;

        captured_ostream() : copy(0)
        {}
    };

    std::map< std::ostream *, captured_ostream > loggers;
}

namespace sao
{
namespace stream
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
            std::map< void (*)( bool open, bool feed, bool close, const std::string &text ), sao::detail::sbb > map;
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
                ( map[ proc ] = map[ proc ] ) = sao::detail::sbb(proc);

                list.push_back( new std::ostream( &map[proc] ) );
                return *list.back();
            }
        } _;

        return _.insert( proc );
    }
} // stream::
} // sao::

//--------------

namespace {
    void sleep( double seconds ) {
        std::chrono::microseconds duration( (int)(seconds * 1000000) );
        std::this_thread::sleep_for( duration );
    }
    std::string replace( const std::string &on, const std::string &find, const std::string &repl ) {
        std::string s = on;
        for( unsigned found = 0; ( found = s.find( find, found ) ) != std::string::npos ; ) {
            s.replace( found, find.length(), repl );
            found += repl.length();
        }
        return s;
    }
    std::string sanitize( const std::string &path ) {
        $win32( std::string from = "/", to = "\\" );
        $welse( std::string from = "\\", to = "/" );
        return replace(path, from, to );
    }
    const std::string get_temp_path() {
        // \st823.1 becomes .\st823.1, or w:\users\user9612\temp~1\st823.1
        $win32( static const std::string temp = getenv("TEMP") ? getenv("TEMP") : ( getenv("TMP") ? getenv("TMP") : "" ) );
        $welse( static const std::string temp );
        return temp;
    }
    const char &at( const std::string &self, unsigned pos ) {
        signed size = (signed)(self.size());
        if( size )
            return self[ pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) ];
        static std::map< const std::string *, char > map;
        return ( ( map[ &self ] = map[ &self ] ) = '\0' );
    }
    char &at( std::string &self, unsigned pos ) {
        signed size = (signed)(self.size());
        if( size )
            return self[ pos >= 0 ? pos % size : size - 1 + ((pos+1) % size) ];
        static std::map< const std::string *, char > map;
        return ( ( map[ &self ] = map[ &self ] ) = '\0' );
    }
    std::vector< std::string > tokenize( const std::string &self, const std::string &chars ) {
        std::string map( 256, '\0' );
        for( std::string::const_iterator it = chars.begin(), end = chars.end(); it != end; ++it )
            map[ *it ] = '\1';
        std::vector< std::string > tokens;
        tokens.push_back( std::string() );
        for( unsigned i = 0, end = self.size(); i < end; ++i ) {
            unsigned char c = self.at(i);
            std::string &str = tokens.back();
            if( !map.at(c) )
                str.push_back( c );
            else
            if( str.size() )
                tokens.push_back( std::string() );
        }
        while( tokens.size() && !tokens.back().size() )
            tokens.pop_back();
        return tokens;
    }
    bool matches( const std::string &text, const std::string &pattern ) {
        struct local {
            static bool match( const char *pattern, const char *str ) {
                if( *pattern=='\0' ) return !*str;
                if( *pattern=='*' )  return match(pattern+1, str) || *str && match(pattern, str+1);
                if( *pattern=='?' )  return *str && (*str != '.') && match(pattern+1, str+1);
                return (*str == *pattern) && match(pattern+1, str+1);
        } };
        return local::match( pattern.c_str(), text.c_str() );
    }
    bool recurse( sao::folder &self, const std::string &sDir, const std::vector<std::string> &masks, bool recursive ) {
        sao::file path( sDir );
        if( path.is_file() ) {
            self.insert( path );
            return false;
        }
        $win32(
            WIN32_FIND_DATAA fdFile;
            std::string sPath = sDir + "\\*";
            HANDLE hFind = FindFirstFileA(sPath.c_str(), &fdFile);

            if( hFind == INVALID_HANDLE_VALUE )
                return "Path not found", false;

            do {
                // Ignore . and .. folders
                if( !strcmp(fdFile.cFileName, ".") )
                    continue;
                if( !strcmp(fdFile.cFileName, "..") )
                    continue;

                // Rebuild path
                sPath = std::string(sDir) + "\\" + std::string(fdFile.cFileName);

                // Scan recursively if needed
                if( !(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
                    for( auto &mask : masks ) {
                        if( matches(sPath, mask) ) {
                            self.insert( sao::file( sPath ) );
                            break;
                        }
                    }
                }
                else {
                    if( !recursive ) {
                        for( auto &mask : masks )
                            if( matches(sPath, mask) ) {
                                self.insert( sao::file( sPath ) );
                                break;
                            }
                    }
                    else {
                        recurse( self, sPath, masks, recursive );
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
                    sao::file entry( found );
                    if( entry.exist() )
                        self.insert( entry );
                }
            }
            return true;
        )
    }
}

namespace sao
{
    path::path() : std::string(), subdirs(false)
    {}

    path::path( const std::string &os_path, bool include_subdirs ) : std::string(os_to_host(os_path)), subdirs(include_subdirs)
    {}

    path::path( const std::string &host_extended_path ) : std::string( host_extended_path ), subdirs( host_to_host(host_extended_path).second )
    {}

    std::string path::to_os() const {
        return host_to_os( *this ).first;
    }

    std::string path::to_host() const {
        return *this;
    }

    /* /g/Boot/ -> g:\\Boot\\ with no subfolders ; also /g/Boot// -> g:\Boot\ with subfolders */
    std::pair<std::string,bool> path::host_to_host( const std::string &dir ) const {
        bool subdirs = false;

        int idx = dir.find_last_of('/');
        if( idx != std::string::npos )
            if( idx > 0 && ::at(dir,idx-1) == '/' ) //&& dir.find_last_of('*') > idx && dir.find_last_of('*') < std::string::npos )
                subdirs = true;

        return std::pair<std::string,bool>(dir,subdirs);
    }

    /* /g/Boot/ -> g:\\Boot\\ with no subfolders ; also /g/Boot// -> g:\Boot\ with subfolders */
    std::pair<std::string,bool> path::host_to_os( const std::string &dir ) const {
        bool subdirs = false;

        int idx = dir.find_last_of('/');
        if( idx != std::string::npos )
            if( idx > 0 && ::at(dir,idx-1) == '/' ) //&& dir.find_last_of('*') > idx && dir.find_last_of('*') < std::string::npos )
                subdirs = true;

        $win32(
        if( dir.size() > 2 && ::at(dir,0) == '/' && ::at(dir,2) == '/' &&
         (( ::at(dir,1) >= 'a' && ::at(dir,1) <= 'z' ) || ( ::at(dir,1) >= 'A' && ::at(dir,1) <= 'Z' )) )
            return std::pair<std::string,bool>( std::string() + ::at(dir,1) + ':' + ::replace(dir,"/","\\").substr(2), subdirs );
        else
            return std::pair<std::string,bool>( ::replace(dir,"/", "\\"), subdirs );
        )
        $welse(
        return std::pair<std::string,bool>( dir, subdirs );
        )
    }

    /* g:\\Boot\\ -> /g/Boot/ */
    std::string path::os_to_host( const std::string &dir ) const {
        $win32(
        if( dir.size() > 2 && ::at(dir,1) == ':' && ::at(dir,2) == '\\' &&
         (( ::at(dir,0) >= 'a' && ::at(dir,0) <= 'z' ) || ( ::at(dir,0) >= 'A' && ::at(dir,0) <= 'Z' )) )
            return std::string( "/" ) + ::at(dir,0) + ::replace(dir,"\\","/").substr(2);
        return ::replace(dir,"\\", "/");
        )
        $welse(
        return dir;
        )
    }



    file::file( const std::string &_pathfile, sorting_type defaults ) :
        is_temp_name( _pathfile.size() ? false : true ),
        pathfile( _pathfile.size() ? _pathfile : std::tmpnam(0) ),
        sorting(defaults)
    {
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

    sao::path file::path() const {
        return pathfile;
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
            if( !exist() )
                return true;
            if( !std::remove( pathfile.c_str() ) )
                return true;
            sleep(0.001);
        }
        return false;
    }

    bool file::rename( const std::string &new_pathfile ) {
        for( int i = 0; i < 512; ++i ) {
            if( !exist() )
                return false;
            if( !std::rename( pathfile.c_str(), new_pathfile.c_str() ) )
                { pathfile = new_pathfile; return true; }
            sleep(0.001);
        }
        return false;
    }

    bool file::exist() const { // may fail due to permissions (check errno)
        struct stat fileInfo;
        return stat( pathfile.c_str(), &fileInfo ) == 0 ? true : false;
    }

    bool file::is_dir() const { // may return false due to permissions (check errno)
        struct stat fileInfo;
        if( stat( pathfile.c_str(), &fileInfo ) < 0 )
            return false;
        return (( fileInfo.st_mode & S_IFMT ) == S_IFDIR );
    }

    bool file::is_file() const { // may return true due to permissions (check errno)
        return !is_dir();
    }

    bool file::has_data() const {
        return exist() && is_file() && size() > 0;
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
        if( cache.find( pathfile ) == cache.end() )
        {
            ( cache[ pathfile ] = cache[ pathfile ] ) = date();
            return true;
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
        return metadata[ property ] = metadata[ property ];
    }
    std::map< std::string, std::string > &file::metas() {
        return metadata;
    }
    const std::map< std::string, std::string > &file::metas() const {
        return metadata;
    }

    // sorting

    void file::sort_by( sorting_type sorter ) {
        sorting = sorter;
    }

    int file::compare_with( const file &other, sorting_type sorter ) {
        sorting_type bkp = sorting;
        sorting = sorter;
        if( operator <( other ) ) return sorting = bkp, -1;
        if( operator==( other ) ) return sorting = bkp,  0;
        return sorting = bkp, +1;
    }

    bool file::operator==( const file &other ) const {
        if( sorting == by_extension ) return ext() == other.ext();
        if( sorting == by_size ) return size() == other.size();
        if( sorting == by_date ) return date() == other.date();
        if( sorting == by_type ) return is_dir() == other.is_dir();
        //if( sorting == by_name )
        return pathfile == other.pathfile;
    }

    bool file::operator<( const file &other ) const {
        //@todo: lower!, buggy
        if( sorting == by_extension ) return ext() < other.ext();
        if( sorting == by_size ) return size() < other.size();
        if( sorting == by_date ) return date() < other.date();
        if( sorting == by_type ) return is_dir() < other.is_dir();
        //if( sorting == by_name )
        return pathfile < other.pathfile;
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

    folder folder::sort( file::sorting_type sorting ) const {
        folder result;
        for( const_iterator it = this->begin(); it != this->end(); ++it )
            result.insert( file( it->name(), sorting ) );
        return result;
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
