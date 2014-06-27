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
 * - see apathy.cpp

 * - rlyeh ~~ listening to Alice in chains / Nutshell
 */

#pragma once

#include <ctime>

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace apathy
{
    /* stream manipulation */
    namespace stream
    {
        // stream api
        void attach( std::ostream &os, void (*custom_stream_callback)( bool open, bool feed, bool close, const std::string &line ) );
        void detach( std::ostream &os, void (*custom_stream_callback)( bool open, bool feed, bool close, const std::string &line ) );
        void detach( std::ostream &os );

        // stream api (extra)
        std::ostream &make( void (*custom_stream_callback)( bool open, bool feed, bool close, const std::string &line ) );
    }

    /* file manipulation */
    class file {
    public:

         file( const std::string &_pathfile = std::string() );
        ~file();

        std::string name() const;
        std::string ext() const;
        std::string path() const;

        size_t size() const;

        std::string read() const;

        bool overwrite( const std::string &content ) const;
        bool overwrite( const void *data, size_t size ) const;

        bool append( const std::string &content ) const;
        bool append( const void *data, size_t size ) const;

        bool remove() const;
        bool rename( const std::string &pathfile );

        bool exist() const; // may fail due to permissions (check errno)

        bool is_dir() const; // may return false due to permissions (check errno)
        bool is_file() const; // may return true due to permissions (check errno)
        bool has_data() const;

        std::time_t date() const;
        std::string timestamp( const char *format = "%Y-%m-%d %H:%M:%S" ) const; // defaults to mysql date format

        bool touch( const std::time_t &modtime = (std::time(0)) ); // const // may fail due to sharing violations (check errno)
        bool has_changed(); // check for external changes on file

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
        mutable std::map< std::string /*property*/, std::string /*value*/ > metadata;
    };

    /* stream manipulation */
    class folder : public std::set< apathy::file > {
    public:

        folder();

        void include( const std::string &path, const std::vector<std::string> &masks = {"*"}, bool scan_subdirs = true );
        void exclude( const std::string &path, const std::vector<std::string> &masks = {"*"}, bool scan_subdirs = true );

        std::string str( const char *format1 = "\1\n" ) const;
    };

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
            }
            template<typename ostream>
            inline friend ostream &operator<<( ostream &os, const Segment &self ) {
                return os << self.segment, os;
            }
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
        static path join( const std::vector<Segment>& segments);

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
        template<typename ostream>
        inline friend ostream& operator<<( ostream& stream, const path &self ) {
            return stream << self.m_path, stream;
        }

    private:
        /* Our current path */
        std::string m_path;
    };
}
