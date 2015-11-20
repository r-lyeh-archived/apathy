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

#define APATHY_VERSION "1.0.0" /* (2015/11/20): Simplified API, moved vfs/ostream to libraries apart
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
    file      ext( const     file &uri ); // d:/prj/test.png -> .png

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
#   ifdef APATHY_USE_MMAP
#       include <sys/mman.h>
#   endif
#   include <dirent.h>
#   include <utime.h>
#   include <unistd.h>
#else
#   ifdef APATHY_USE_MMAP
#       include "deps/mman/mman.h"
#   endif
#   include "deps/dirent/dirent.h"
#   include <direct.h>    // _mkdir, _rmdir
#   include <sys/utime.h> // (~) utime.h
#   include <io.h>        // (~) unistd.h
    typedef int mode_t;
#endif

// implementation

namespace apathy {

    // size in bytes
    inline size_t size( const pathfile &uri ) {
        if( uri.is_path() ) {
            return ls(uri).size();
        }
        if( uri.is_file() ) {
            struct stat info;
            if( stat( uri, &info ) < 0 ) {
                return 0;
            }
            return info.st_size;
        }
        return 0;
    }

    // true if exist
    inline bool exists( const pathfile &uri ) {
        if( uri.is_file() ) {
            $apathy32( return 0 == _access( uri,    0 ) );
            $apathyXX( return 0 ==  access( uri, F_OK ) );
        }
        if( uri.is_path() ) {
            if( !uri.empty() ) {
                struct stat info;
                if (stat(uri, &info) < 0) {
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
        if( stat( uri, &info ) < 0 ) {
            return false;
        }
        return S_IFDIR == ( info.st_mode & S_IFMT );
    }

    // true if file
    inline bool is_file( const pathfile &uri ) {
        struct stat info;
        if( stat( uri, &info ) < 0 ) {
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
        if( stat( uri, &info ) < 0 ) {
            return std::time(0);
        }
        return info.st_atime;
    }

    // modification date (last time file contents were modified)
    inline time_t mdate( const pathfile &uri ) {
        struct stat info;
        if( stat( uri, &info ) < 0 ) {
            return std::time(0);
        }
        return info.st_mtime;
    }

    // change date (last time meta data of file was changed)
    inline time_t cdate( const pathfile &uri ) {
        struct stat info;
        if( stat( uri, &info ) < 0 ) {
            return std::time(0);
        }
        return info.st_ctime;
    }

    // user id
    inline int uid( const pathfile &uri ) {
        struct stat info;
        if( stat( uri, &info ) < 0 ) {
            return 0;
        }
        return info.st_uid;
    }

    // group id
    inline int gid( const pathfile &uri ) {
        struct stat info;
        if( stat( uri, &info ) < 0 ) {
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
    inline file ext( const file &uri_ ) {
        file uri = name(uri_);
        file found = uri.substr( uri.find_last_of('.') );
        return found != uri ? found : uri;
    }

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

    // read data from file
    inline bool read( const file &uri, std::string &buffer ) {
        struct stat info;
        if( stat( uri, &info ) < 0 ) {
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
        if( stat( uri, &info ) < 0 ) {
            return false;
        }

        struct utimbuf tb;
        tb.actime = info.st_atime;  /* keep atime unchanged */
        tb.modtime = date;              /* set mtime to given time */

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
        bool changed = ( diff > 0 ? diff : -diff ) > 2.5;  

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
        test( size(self) > 0 );
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
    };

    suite( "test path infos" ) {
        auto self = "././";
        test( exists(self) );
        test( size(self) > 0 );
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
        test( size(self) > 0 );

        test( !touched(self) );
        test(  touch(self) );
        $apathy32( sleep( 3.0 ) );
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
