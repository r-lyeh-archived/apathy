/*
 * Simple file handling classes

 * Copyright (c) 2010-2011 Mario 'rlyeh' Rodriguez
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
 *
 * add stream() << >> chunk()
 * or
 * add subread( from, to ), subwrite( from, to ), chunk(), next(), tell(),

 * - rlyeh ~~ listening to Alice in chains / Nutshell
 */

#pragma once

#include <ctime>

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace sao
{
    class path : public std::string
    {
    public:

        const bool subdirs;

        path();
        path( const std::string &os_path, bool include_subdirs );
        path( const std::string &host_extended_path );

        std::string to_os() const;
        std::string to_host() const;

    protected:

        std::string os_to_host( const std::string &dir ) const;
        std::pair<std::string,bool> host_to_host( const std::string &dir ) const;
        std::pair<std::string,bool> host_to_os( const std::string &dir ) const;
    };

    class file
    {
    public:

        enum sorting_type { by_name, by_size, by_date, by_extension, by_type } sorting;

        file( const std::string &_pathfile = std::string(), sorting_type defaults = by_name );

        ~file();

        std::string name() const;
        std::string ext() const;

        sao::path path() const;

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

              std::string &meta( const std::string &property );
        const std::string &meta( const std::string &property ) const;

              std::map<std::string,std::string> &metas();
        const std::map<std::string,std::string> &metas() const;

        // sorting

        void sort_by( sorting_type sorter );

        int compare_with( const file &other, sorting_type sorter );

        bool operator==( const file &other ) const;
        bool operator<( const file &other ) const;

        // patching

        bool patch( const std::string &patch_data, bool delete_tempfile = true ) const;

    private:

        bool is_temp_name;
        std::string pathfile;
        mutable std::map< std::string /*property*/, std::string /*value*/ > metadata;
    };

    class files : public std::set< file >
    {
    public:

        files();

        void include( const std::string &path, const std::vector<std::string> &masks, bool scan_subdirs );
        void exclude( const std::string &path, const std::vector<std::string> &masks, bool scan_subdirs );

        files sort( file::sorting_type sorting ) const;

        std::string str( const char *format1 = "\1\n" ) const;
    };
}


#if 0

// filesystem = ~xml

#include <sao/io/xml.hpp>

namespace sao
{
    class filesystem
    {
        sao::xml xml;

    public:

        filesystem()
        {}

        ~filesystem()
        {}

        virtual files search()
        {}

        //useful when having ::files ?
        //virtual void mount( id, files, ro/rw ) // unmount
        //{}

        //cwd,pwd,pushd,popd
        //root,defaults
        //read,overwrite,append,compress,uncompress
        //stream

        //import_file
        //import_folder

        //remove (from fs)

        //bool update()
        //-> get_additions
        //-> get_removals

        virtual void unmount( id )
        {}

        virtual void patch( id, files, ro/rw )
        {}
    };
}

#endif

