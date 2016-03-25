Apathy :floppy_disk: <a href="https://travis-ci.org/r-lyeh/apathy"><img src="https://api.travis-ci.org/r-lyeh/apathy.svg?branch=master" align="right" /></a>
====

Apathy is a lightweight IO library (C++03).

### Features
- [x] Append, overwrite, create, copy, move, delete, delete recursively, etc.
- [x] Binary file patching (including locked binaries).
- [x] Fast file/dir disk globbing.
- [x] Functional API.
- [x] Memory-map support.
- [x] Semantic URI class.
- [x] Compatible in-memory std::istreams/ostreams.
- [x] Tmpdir and tmpnames.
- [x] Touch and date functions.
- [x] Wildcards support.
- [x] Tiny, portable, cross-platform and header-only.
- [x] ZLIB/LibPNG licensed.

### Rationale design
- Functional API, because OOP is not really needed at all.
- In order to simplify both library and API, Apathy assumes that:
  - Paths are arguments that end with slash '/'.
  - Files are arguments that do not end with slash '/'.
  - Pathfiles are arguments that can be both paths or files.
  - Path+path and path+file concatenations are valid.
- On errors, check `apathy::why()` to retrieve last known error string.

### Public API
```c++
namespace apathy {

    // Usage:
    // { imstream membuf(ptr, len); std::istream is(&membuf); /*...*/ }
    // { omstream membuf(ptr, len); std::ostream os(&membuf); /*...*/ }
    class imstream { imstream( ptr, size ); }
    class omstream { omstream( ptr, size ); }

    // Any of following functions may fail due to permissions, non-existing path, etc
    // Check apathy::why() to retrieve reason for last known error string.

    // Read/write API

    void  *map( file uri, size_t size, size_t offset=0 );
    void unmap( void *ptr, size_t size );

    bool read( file uri, string &buffer );
    bool read( file uri, void *data, size_t &size );

    bool append( file uri, const string &data );
    bool append( file uri, const void *data, size_t size );

    bool overwrite( file uri, const string &data );
    bool overwrite( file uri, const void *data, size_t size );

    // Info API (RO)

    bool   exists( pathfile uri );

    bool  is_path( pathfile uri );
    bool  is_file( pathfile uri );
    bool  is_link( pathfile uri );

    size_t   size( pathfile uri );
    bool    empty( pathfile uri );

    int       gid( pathfile uri );
    int       uid( pathfile uri );

    path     stem( pathfile uri );
    file     name( pathfile uri );
    file     base(     file uri );
    pathfile  ext(     file uri );

    // Info API (RW)

    bool    chown( file uri, int uid, int gid );
    bool    chmod( file uri, int mode );

    // Folder API

    bool pushd( path uri );
    bool  popd();
    path   cwd();
    bool    cd( path uri );
    bool    rd( path uri );
    bool    md( path uri, size_t mode );

    // Disk operations API

    bool   mv( pathfile uri, pathfile uri_dst );
    bool   cp( pathfile uri, pathfile uri_dst );
    bool   rm( pathfile uri );
    bool rmrf( pathfile uri );
    bool   ls( vector<string> &list, path uri="", string masks="*" );
    bool  lsr( vector<string> &list, path uri="", string masks="*" );

    // File patching API (will patch locked binaries too)

    bool patch( file uri, const file &patchdata );

    // Date & modif API

    bool touch( pathfile uri, time_t );
    bool touched( pathfile uri );

    time_t adate( pathfile uri );
    time_t cdate( pathfile uri );
    time_t mdate( pathfile uri );
    string stamp( time_t date, format="%Y-%m-%d %H:%M:%S" );

    // Temp API

    path tmpdir();
    file tmpname();

    // Handy aliases (for convenience)

    string read( file uri );
    vector<string> ls( path uri="", string masks="*" );
    vector<string> lsr( path uri="", string masks="*" );

    // Error retrieval API

    string why();
}
```

### Alternatives
- https://github.com/yiptool/path-util
- https://github.com/dlecocq/apathy

### Licenses
- [Apathy](https://github.com/r-lyeh/apathy), ZLIB/libPNG licensed.
- [Dirent](http://softagalleria.net/dirent.php) by Toni Ronkko, MIT licensed.

### Changelog
- v1.0.3 (2016/03/25): Fix MingW compilation issues
- v1.0.2 (2016/02/02): Fix ext() with dotless files
- v1.0.2 (2016/02/02): Fix m/c/adate() on invalid pathfiles
- v1.0.2 (2016/02/02): Handle proper Win32 stat() case
- v1.0.1 (2015/12/02): Add resize() function
- v1.0.0 (2015/11/20): Simplified API, moved vfs/ostream to libraries apart
- v0.0.0 (2013/04/16): Initial commit
