/*
 * Apathy is a lightweight stream/file/mmap/path/virtual-filesystem IO C++11 library.
 * Copyright (c) 2011,2012,2013,2014 Mario 'rlyeh' Rodriguez
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See license copy at http://www.boost.org/LICENSE_1_0.txt)

 * Copyright (c) 2013 Dan Lecocq
 *
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
 * - see apathy.cpp

 * - rlyeh ~~ listening to Alice in chains / Nutshell
 */

#pragma once

#include <cassert>
#include <ctime>
#include <cctype>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include <chrono>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

namespace apathy
{
    /* ostream manipulation */
    namespace ostream
    {
        // stream api
        void attach( std::ostream &os, void (*custom_stream_callback)( bool open, bool feed, bool close, const std::string &line ) );
        void detach( std::ostream &os, void (*custom_stream_callback)( bool open, bool feed, bool close, const std::string &line ) );
        void detach( std::ostream &os );

        // stream api (extra)
        std::ostream &make( void (*custom_stream_callback)( bool open, bool feed, bool close, const std::string &line ) );
    }

    /* in-memory stream manipulation */
    struct stream {
        const char *begin, *end, *cursor, *rw;

        stream();
        stream( const void *from, const void *to );
        stream( const void *ptr, unsigned len );
        stream( void *ptr, unsigned len );

        bool good( unsigned off = 0 ) const;

        bool open();
        bool close();

        bool rewind();
        bool eof() const;
        unsigned tell() const;
        unsigned size() const;
        unsigned left() const;
        bool seek( unsigned offset );
        bool offset( int offset );
        const char *data() const;

        bool read( void *ptr, unsigned N );
        bool write( const void *ptr, unsigned N );

        template<typename T>
        bool read( T &t ) {
            return read( &t, sizeof t );
        }
        template<typename T>
        bool write( const T &t ) {
            return write( &t, sizeof t );
        }

        bool read8( int8_t &c );
        bool read16( int16_t &c );
        bool read32( int32_t &c );
        bool read64( int64_t &c );
        bool write8( const int8_t &c );
        bool write16( const int16_t &c );
        bool write32( const int32_t &c );
        bool write64( const int64_t &c );
        int getc();

        // states
        std::vector< stream > stack;
        void push();
        void pop();

        // inspection
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

    /* pipe manipulation */
    template< typename data, typename meta = std::string >
    struct pipe {
        enum state { FAIL, KEEP, STOP };

        typedef std::function<bool(data&,meta&)> filter;
        std::vector< filter > filters;

        // basics
        pipe &chain( const filter &filt ) {
            return filters.push_back( filt ), *this;
        }
        bool process( data &d, meta &m ) const {
            auto st = KEEP;
            for( auto fn : filters ) {
                if( KEEP != ( st = (state)fn( d, m ) ) )
                    break;
            }
            return FAIL != st;
        }
        bool process( data &d ) const {
            meta m;
            return process( d, m );
        }

        // aliases
        pipe &operator <<( const filter &filt ) {
            return chain( filt );
        }
        bool operator()( data &d, meta &m ) const {
            return process( d, m );
        }
        bool operator()( data &d ) const {
            return process( d );
        }
    };

    /* file manipulation */
    class file {
    public:

         file( const std::string &_pathfile = std::string() );
        ~file();

        std::string name() const;
        std::string ext() const;
        std::string path() const;

        bool matches( const std::string &wildcard ) const;

        size_t size() const;

        std::string read() const;

        std::thread preload( const std::function<void()> &ok = []{}, const std::function<void()> &fail = []{} ) const {
            return std::thread(
                []( std::string name, std::function<void()>ok, std::function<void()> fail ){
                    bool has_data = apathy::file( name ).has_data();
                    std::string data = apathy::file( name ).read();
                    if( has_data && data.size() ) {
                        ok();
                    } else {
                        fail();
                    }
                }, name(), ok, fail );
        }

        bool overwrite( const std::string &content ) const;
        bool overwrite( const void *data, size_t size ) const;

        bool append( const std::string &content ) const;
        bool append( const void *data, size_t size ) const;

        bool remove() const;
        bool rename( const std::string &pathfile );

        bool exists() const; // may fail due to permissions (check errno)
        bool is_dir() const; // may return false due to permissions (check errno)
        bool is_file() const; // may return true due to permissions (check errno)
        bool has_data() const;

        std::time_t date() const;
        std::string timestamp( const char *format = "%Y-%m-%d %H:%M:%S" ) const; // defaults to mysql date format

        bool touch( const std::time_t &modtime = (std::time(0)) ); // const // may fail due to sharing violations (check errno)
        bool has_changed(); // check for external changes on file

        bool mmap();
        bool munmap();
        stream chunk( size_t offset = 0, size_t len = ~0 );

        // metadata api

        const std::string &meta( const std::string &property ) const;
              std::string &meta( const std::string &property );

        const std::map<std::string,std::string> &metas() const;
              std::map<std::string,std::string> &metas();

        // patching

        bool patch( const std::string &patch_data, bool delete_tempfile = true ) const;

        bool operator<( const file &other ) const {
            return pathfile < other.pathfile;
        }

    private:

        bool is_temp_name;
        std::string pathfile;
        std::map< std::string /*property*/, std::string /*value*/ > metadata;
    };

    /* folder watcher and globber */
    class folder : public std::set< apathy::file > {
    public:

        folder();

        void include( const std::string &path, const std::vector<std::string> &masks = default_masks, bool recurse_subdirs = true );
        void exclude( const std::string &path, const std::vector<std::string> &masks = default_masks, bool recurse_subdirs = true );

        std::string str( const char *format1 = "\1\n" ) const;

    private:

        static const std::vector<std::string> default_masks;
    };

    /* virtual filesystem */
    // oak<std::string, file>

    /* path manipulation */
    class path {
    public:

        /* path separators */
        enum : char { separator = '/', separator_alt = '\\' };

        /* A class meant to contain path segments */
        struct Segment {
            /* The actual string segment */
            std::string segment;

            Segment( const std::string &s = std::string() ) : segment(s)
            {}

            template<typename istream>
            inline friend istream &operator>>( istream &is, Segment& self ) {
                char separator_( separator );
                return std::getline( is, self.segment, separator_ );
            } /*
            template<typename ostream>
            inline friend ostream &operator<<( ostream &os, const Segment &self ) {
                return os << self.segment, os;
            } */
        };

        /**********************************************************************
         * Constructors
         *********************************************************************/

        /* Default constructor
         *
         * Points to current directory */

        path( const std::string &path_ = std::string() ): m_path(path_)
        {}

        /* Our generalized constructor.
         *
         * This enables all sorts of type promotion (like int -> path) for
         * arguments into all the functions below. Anything that
         * std::stringstream can support is implicitly supported as well
         *
         * @param p - path to construct */
        /* Constructor */
        template <class T>
        path( const T& p ) {
            std::stringstream ss;
            if( ss << p ) {
                m_path = ss.str();
            }
        }

        template <size_t N>
        path( const char (&p)[N] ) {
            m_path = p;
        }

        /**********************************************************************
         * Operators
         *********************************************************************/
        /* Checks if the paths are exactly the same */
        bool operator==( const path &other) const { return m_path == other.m_path; }

        /* Check if the paths are not exactly the same */
        bool operator!=( const path &other) const { return !operator==(other); }

        /* Append the provided segment to the path as a directory. This is the
         * same as append(segment)
         *
         * @param segment - path segment to add to this path */
        path &operator<<( const path &segment);

        /* Append the provided segment to the path as a directory. This is the
         * same as append(segment). Returns a /new/ path object rather than a
         * reference.
         *
         * @param segment - path segment to add to this path */
        path operator+( const path &segment) const;

        /* Check if the two paths are equivalent
         *
         * Two paths are equivalent if they point to the same resource, even if
         * they are not exact string matches
         *
         * @param other - path to compare to */
        bool equivalent( const path &other) const;

        /* Return a string version of this path */
        std::string string() const { return m_path; }

        /* Return the name of the file */
        std::string filename() const;

        /* Return the extension of the file */
        std::string extension() const;

        /* Return a path object without the extension */
        path stem() const;

        /**********************************************************************
         * Manipulations
         *********************************************************************/

        /* Append the provided segment to the path as a directory. Alias for
         * `operator<<`
         *
         * @param segment - path segment to add to this path */
        path &append( const path &segment);

        /* Evaluate the provided path relative to this path. If the second path
         * is absolute, then return the second path.
         *
         * @param rel - path relative to this path to evaluate */
        path &relative( const path &rel);

        /* Move up one level in the directory structure */
        path &up();

        /* Normalize this path
         *
         * This...
         *
         * 1) replaces runs of consecutive separators with a single separator
         * 2) evaluates '..' to refer to the parent directory, and
         * 3) strips out '/./' as referring to the current directory
         *
         * If the path was absolute to begin with, it will be absolute
         * afterwards. If it was a relative path to begin with, it will only be
         * converted to an absolute path if it uses enough '..'s to refer to
         * directories above the current working directory */
        path &normalize();

        /* Return /an/absolute/path
         *
         * If the path is already absolute, it has no effect. Otherwise, it is
         * evaluated relative to the current working directory */
        path absolute() const;

        std::string os() const;
        std::string win() const;
        std::string posix() const;

        /* Make this path a directory
         *
         * If this path does not have a trailing directory separator, add one.
         * If it already does, this does not affect the path */
        path &directory();

        /* Trim this path of trailing separators, up to the leading separator.
         * For example, on *nix systems:
         *
         *   assert(path("///").trim() == "/");
         *   assert(path("/foo//").trim() == "/foo");
         */
        path &trim();

        /**********************************************************************
         * Copiers
         *********************************************************************/

        /* Return parent path
         *
         * Returns a new path object referring to the parent directory. To
         * move _this_ path to the parent directory, use the `up` function */
        path parent() const { return path(path(*this).up()); }

        /**********************************************************************
         * Member Utility Methods
         *********************************************************************/

        /* Returns a vector of each of the path segments in this path */
        std::vector<Segment> split() const;

        /**********************************************************************
         * Type Tests
         *********************************************************************/
        /* Is the path an absolute path? */
        bool is_absolute() const;

        /* Does the path have a trailing slash? */
        bool trailing_slash() const;

        /* Does this path exist?
         *
         * Returns true if the path can be `stat`d */
        bool exists() const;

        /* Is this path an existing file?
         *
         * Only returns true if the path has stat.st_mode that is a regular
         * file */
        bool is_file() const;

        /* Is this path an existing directory?
         *
         * Only returns true if the path has a stat.st_mode that is a
         * directory */
        bool is_directory() const;

        /* How large is this file?
         *
         * Returns the file size in bytes. If the file doesn't exist, it
         * returns 0 */
        size_t size() const;

        /**********************************************************************
         * Static Utility Methods
         *********************************************************************/

        /* Return a brand new path as the concatenation of the two provided
         * paths
         *
         * @param a - first part of the path to join
         * @param b - second part of the path to join
         */
        static path join( const path &a, const path &b);

        /* Return a branch new path as the concatenation of each segments
         *
         * @param segments - the path segments to concatenate
         */
        static path join( const std::vector<Segment>& segments );

        /* Current working directory */
        static path cwd();

        /* Create a file if one does not exist
         *
         * @param p - path to create
         * @param mode - mode to create with */
        static bool touch( const path &p, unsigned mode = 0777 );

        /* Move / rename a file
         *
         * @param source - original path
         * @param dest - new path
         * @param mkdirs - recursively make any needed directories? */
        static bool mv( const path &source, const path &dest, bool mkdirs = false );

        /* Recursively make directories
         *
         * @param p - path to recursively make
         * @returns true if it was able to, false otherwise */
        static bool md( const path &p, unsigned mode = 0777 );

        /* Remove a file or directory
         *
         * @param p - path to remove */
        static bool rm( const path &p );

        /* Recursively remove files and directories
         *
         * @param p - path to recursively remove */
        static bool rmrf( const path &p, bool ignore_errors = false );

        /* List all the paths in a directory
         *
         * @param p - path to list items for */
        static std::vector<path> ls( const path &p );

        /* Returns all matching globs
         *
         * @param pattern - the glob pattern to match */
        static std::vector<path> glob( const std::string& pattern );

        /* So that we can write paths out to ostreams */
        inline friend std::ostream& operator<<( std::ostream& os, const path &self ) {
            return os << self.m_path, os;
        }

    private:
        /* Our current path */
        std::string m_path;
    };

    // extras
    /*
    class uri : public std::string {
    public:
        bool is_file;
        bool is_dir;

        uri() : std::string()
        {}

        template<typename T>
        uri( const T &t ) : std::string(t)
        {}
    }; */

    typedef std::string uri;
    typedef std::set<std::string> uris;
    typedef uris set;
    typedef std::tuple< size_t /*filestamp*/, size_t /*filesize*/, std::string /*filename*/ > hash;
    class watcher;
    typedef std::function<void(watcher &)> watcher_callback;

    std::ostream &print( const uris &uris_, std::ostream &out = std::cout );
    std::string replace( const std::string &str_, const std::string &from, const std::string &to );
    std::vector<std::string> split( const std::string &s, char sep );
    std::vector<std::string> wildcards( const std::string &mask );

    bool is_dir( const std::string &uri );
    bool exists( const std::string &uri );
    bool has_changed( const std::string &uri );
    bool matches( const std::string &text, const std::string &pattern );

    std::string normalize( const std::string &uri );
    std::string tokenize( const std::string &uri );
    std::string notrails( const std::string &uri );

    void chdir( const std::string &path );
    void pushd();
    void popd();
    std::string cwd();

    bool mkdir( const std::string &path, unsigned mode = 0644 );
    bool rmdir( const std::string &path );

    void sleep( double t );

    // glob files and directories

    bool glob( const std::string &uri, uris &files, uris &dirs );
    bool globr( const std::string &uri, uris &files, uris &dirs, unsigned recurse_level = ~0 );

    uris &filter( uris &set, const std::string &includes = "*", const std::string &excludes = "" );

    uris lsf( const std::string &includes = "*", const std::string &excludes = "" );
    uris lsfr( const std::string &includes = "*", const std::string &excludes = "" );
    uris lsd( const std::string &includes = "*", const std::string &excludes = "" );
    uris lsdr( const std::string &includes = "*", const std::string &excludes = "" );
    uris ls( const std::string &includes = "*", const std::string &excludes = "" );
    uris lsr( const std::string &includes = "*", const std::string &excludes = "" );

    // folder monitoring

    class watcher {
    public:
        std::string folder;
        watcher_callback callback;
        uris previous, in, out, modif;

        void operator()();

        void reload();
        void sleep( double t );
    };

    void watch( const std::string &uri_, const watcher_callback &callback );

    class uri2 {

        std::string left_of( const std::string &input, const std::string &substring ) const;
        std::string right_of( const std::string &input, const std::string &substring ) const;
        std::string remove( const std::string &input, const std::string &substring ) const;

    public:

        std::string protocol;
        std::string path;
        std::string file;
        std::string options;

        uri2();
        uri2( const std::string &uri );
    };

    // table of contents

    struct entry {
        size_t size, offset; // size and offset in the container (if any)
    };

    typedef std::map< std::string, std::string > mapping;
    typedef std::map< std::string /* phys */, entry > table_of_contents;

    struct filesystem {
        // virt -> phys table
        mapping table;
        // phys -> size, offset
        table_of_contents toc;
        // supported protocols
        std::vector<std::string> protocols;

        virtual ~filesystem() {}
        virtual std::string name() const = 0;
        virtual std::string read( const std::string &phys, size_t size = ~0, size_t offset = 0 ) const = 0;
        virtual table_of_contents scan() = 0;
        virtual void touch( const std::string &phys ) = 0;
        virtual void refresh() = 0;

        virtual bool mount( const std::string &phys, const std::string &virt );
        virtual bool supported( const std::string &virt ) const;

        virtual std::string translate( const std::string &virt ) const;

        virtual bool exists( const std::string &virt ) const;
        virtual bool symlink( const std::string &from_virt, const std::string &to_virt );
        virtual bool rename( const std::string &virt_mask, const std::string &as_virt );

        virtual std::string head() const;
        virtual std::string print() const;
    };

    struct std_io_filesystem : public filesystem {
        std::string pathmask;
        std_io_filesystem( const std::string &path = "/", const std::string &mask = "*" ) : pathmask( path + mask ) {
            refresh();
        }
        std::string name() const {
            return "stdio";
        }
        std::vector<std::string> protocols() const { 
            std::vector<std::string> p;
            p.push_back("file://");
            p.push_back("");
            return p;
        }
        std::string read( const std::string &phys, size_t size = ~0, size_t offset = 0 ) const {
            return apathy::file( phys ).read();
        }
        table_of_contents scan() {
            return toc;
        }
        void touch( const std::string &phys ) {
            apathy::file( phys ).touch();
        }
        void refresh() {
            auto files = lsr( pathmask );
            toc.clear();
            for( auto &file : files ) {
                toc[ file ].offset = 0;
                toc[ file ].size = apathy::file( file ).size();
            }
        }
    };

    //

    class vfilesystem {
        std::deque< filesystem * > filesystems;

        filesystem *locate( const std::string &virt ) const;

    private:

        vfilesystem( const vfilesystem & );
        vfilesystem &operator=( const vfilesystem & );

    public:

         vfilesystem();
        ~vfilesystem();

        template<typename T>
        bool add_filesystem( const std::vector<std::string> &protocols ) {
            filesystems.push_front( new T() );
            filesystems.front()->protocols = protocols;
            return true;
        }

        bool mount( const std::string &phys, const std::string &virt );

        bool exists( const std::string &virt ) const;

        std::string read( const std::string &virt ) const;

        // construct a valid resource, or a placeholder at least
        template<typename T>
        bool make( T &t, const std::string &virt ) const {
            // construct a valid resource, or a placeholder at least
            std::string data = read( virt );
            if( !data.empty() ) {
                // T() construct
                t = T( (const void *)data.c_str(), (size_t)data.size() );
                return true;
            } else {
                // T() might construct a placeholder here
                t = T( (const void *)0, (size_t)0 );
                return false;
            }
        }

        template<typename T>
        T make( const std::string &virt ) const {
            // construct a valid resource, or a placeholder at least
            T t;
            make( t, virt );
            return t;
        }

        std::string toc() const;
    };
}

//
