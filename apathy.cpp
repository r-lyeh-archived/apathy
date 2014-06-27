/*
 * Apathy is a lightweight stream/file/path IO C++11 library with no dependencies.
 * Copyright (c) 2011,2012,2013,2014 Mario 'rlyeh' Rodriguez
 * Copyright (c) 2013 Dan Lecocq

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
 * add stream() << >> chunk()
 * or
 * add subread( from, to ), subwrite( from, to ), chunk(), next(), tell(),
 *
 * platform = { roots[], separators[] }
 * unix = { {"/*"}, {"/"} }
 * win = { {"?:\*"}, {"\\"} }
 * http = { {"http://*"}, {'/', '&', '?'} }
 *
 * absolute
 * relative
 *
 * up
 * cwd
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
 * exists
 * has_trail
 * parent
 * root
 *
 * operations
 * rm
 * rmr
 * md
 * mv
 * ls
 * lsr
 * touch

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
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fstream>
#include <iostream>
#include <istream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

/* OS includes */
#if defined(_WIN32)
#   include <winsock2.h>
#   include <windows.h>
#   include <ctime>
#   include <direct.h>
#   include <io.h>
#   include <sys/utime.h>
#   include "dirent.hpp"
#   define $win32(...) __VA_ARGS__
#   define $welse(...)
#   define open _open
#   define close _close
#   define mkdir(path_,mode) _mkdir( path(path_).os().c_str() )
    typedef int mode_t;
#else
#   include <dirent.h>
#   include <sys/time.h>
#   include <unistd.h>
#   include <utime.h>
#   define $win32(...)
#   define $welse(...) __VA_ARGS__
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
        apathy::detail::sbb sb;

        captured_ostream() : copy(0)
        {}
    };

    std::map< std::ostream *, captured_ostream > loggers;
}

namespace apathy
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
} // stream::
} // apathy::

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
                return "path not found", false;

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
                    if( entry.exist() )
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
+
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
    file::file( const std::string &_pathfile ) :
        is_temp_name( _pathfile.size() ? false : true ),
        pathfile( _pathfile.size() ? _pathfile : std::tmpnam(0) )
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

    std::string file::path() const {
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
        return "";
    }

    std::string path::extension() const {
        /* Make sure we only look in the filename, and not the path */
        std::string name = filename();
        size_t pos = name.rfind('.');
        if (pos != std::string::npos) {
            return name.substr(pos + 1);
        }
        return "";
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

        append("..").normalize();
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
            m_path = "";
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

            path cpy = path(base).relative(ent->d_name).posix();
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
            where = "";
        }

        struct local {
            static bool match( const char *pattern, const char *str ) {
                if( *pattern=='\0' ) return !*str;
                if( *pattern=='*' )  return match(pattern+1, str) || *str && match(pattern, str+1);
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
