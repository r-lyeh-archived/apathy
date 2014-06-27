//
// - rlyeh // listening to boerd ~ materialize

#pragma once

#include <map>
#include <functional>
#include <string>
#include <set>
#include <sstream>
#include <vector>

using unary_call = std::function< bool( const std::string & ) >;
using binary_call = std::function< bool( const std::string &, std::string & ) >;
using interface0 = std::map< std::string, unary_call >;
using interface1 = std::map< std::string, binary_call >;

class plugin {
public:
    std::string name = "dummy";
    std::string description = "this plugin does nothing";
    std::string mask = "*";
    std::string version = "0.0.0";
    std::vector<void *> data;
    interface0 if0;
    interface1 if1;

    bool operator<( const plugin &other ) const {
        return name < other.name;
    }
    bool operator==( const plugin &other ) const {
        return name == other.name;
    }
    template<typename ostream>
    inline friend ostream& operator<<( ostream &out, const plugin &self ) {
        out << self.name << " v" << self.version << " (" << self.description << ")";
        return out;
    }
};

#include "oak/oak.hpp"
#include <algorithm>

// @todo
// c:\prj -> /c/prj
// http://kk -> /http/kk
// ftp://ko -> /ftp/ko
/*
split path
join path

c:\prj\kaka.png
/c/prj/kaka.png

nameext -> kaka.png
name -> kaka
ext -> png

parent = /c/
parent = /
parent = /
path = /c/prj/
abs = /c/prj/
relative = ../prj */

struct pathfile {
    std::vector<std::string> path;
    std::string file;

    pathfile( const std::string &uri_ ) {
        auto uri = uri_;
        std::replace( uri.begin(), uri.end(), '\\', '/' );
        if( uri[0] != '/' ) uri = std::string() + "./" + uri;
        std::istringstream str(uri);
        for( std::string each; std::getline(str, each, '/'); path.push_back(each) );
        if( uri.back() != '/' ) {
            file = path.back();
            path.pop_back();
        }
    }
};

class filesystem
{
    std::map< std::string, plugin > providers;
    std::map< std::string, plugin > paths;

    oak::tree< std::string, plugin > tree;

    plugin &traverse( const std::vector< std::string > &path, bool make = false ) {
        auto *p = &tree.root();
        for( auto &in : path ) {
			/**/ if( in.empty() ) continue;
			else if( in == "." )  continue;
			else if( in == ".." ) { p = &p->up(); continue; }

            if( make ) {
                p = &(*p)[in];
            } else {
                if( !p->has(in) ) {
                    static plugin null;
                    null.description = "reason: invalid path";
                    std::cout << "nononono" << std::endl;
                    return null;
                }
                p = &(*p)[in];
            }
        }

        return p->get();
    }

public:

    filesystem()
    {}

    bool call( const std::string &function, const std::string &arg0 ) {
        if( function.empty() )
            return false;
        if( arg0.empty() )
            return false;
        bool ok = true;
        pathfile pf( arg0 );
        plugin &p = traverse( pf.path, false );
        auto &method = p.if0[function];
        if( method ) {
            ok &= method( pf.file );
            return ok;
        } else {
            std::cout << "[fail] method '" << function << "' does not exist in plugin <" << p.name << ">" << std::endl;
            return false;
        }
    }
    bool call( const std::string &function, const std::string &arg0, std::string &out ) {
        if( function.empty() )
            return false;
        if( arg0.empty() )
            return false;
        bool ok = true;
        pathfile pf( arg0 );
        plugin &p = traverse( pf.path, false );
        auto &method = p.if1[function];
        if( method ) {
            ok &= method( pf.file, out );
            return ok;
        } else {
            std::cout << "[fail] method '" << function << "' does not exist in plugin <" << p.name << ">" << std::endl;
            return false;
        }
    }

    void mount( const std::string &path, const plugin &p ) {
        traverse( pathfile(path).path, true) = p;

        providers[ p.name ] = p;
        paths[ path ] = p;
    }
    void unmount( const std::string &path ) {
        paths[ path ] = paths[ path ];
        paths.erase( path );
    }
    void unmount( const plugin &p ) {
        std::vector< std::string > targets;
        for( auto &path : paths ) {
            if( path.second == p ) {
                targets.push_back( path.first );
            }
        }
        for( auto &target : targets ) {
            unmount( target );
        }
        providers[ p.name ] = providers[ p.name ];
        providers.erase( p.name );
    }

    std::string plugins() const {
        std::stringstream out;
        for( const auto &kv : providers ) {
            out << kv.second << std::endl;
        }
        return out.str();
    }
    std::string showcase() const {
        std::stringstream out;
		out << tree;
        return out.str();
    }

    // fs( ro )
    // fs( rw )

    // locale( "en;es;*" )
    // profile( "ios;iphone;*")
    // revision( 12399 )

    // mount( /, plugin )
    // unmount( / )
    // link( /, / )
    // unlink( / )
    // watch( /, []( item, events ) {} ) // events { removals,additions,touch }
    // unwatch( / )

    // stream { void *begin, *end, *cursor }
    // open, close
    // seek, rewind, eof, offset, size
    // push
    // read1,read2,read4,read8,
    // write1, write2, write4, write8
    // pop
    // read, overwrite, append
    // compress, uncompress
    // mmap, unmmap

    // tmpfile

    // cd,up,pwd,root,home,pushd,popd
    // ls/json, ls-r, find, find-r
    // cp, cp-r, mv, mv-r
    // rm, rm-r
    // concat
    // z,z-r,unz,unz-r
    // crypt,crypt-r,decrypt,decrypt-r
    // touch,date,time,author,revision,extension,mime
    // cp archive.zip/file/1 file -> (revision)
    // cp archive.zip/file/es file -> (localization)
    // cp archive.zip/file@author file -> attrib
    // cp archive.zip/file@date file -> attrib
    // echo `archive.zip/file@es` -> 1

    // localization

    // patch, patch-r
    // diff, diff-r



};

