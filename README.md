Apathy <a href="https://travis-ci.org/r-lyeh/apathy"><img src="https://api.travis-ci.org/r-lyeh/apathy.svg?branch=master" align="right" /></a>
======

- Apathy is a stream/file/mmap/path/virtual-filesystem IO C++11 library.
- Apathy is stand-alone. Embedded libraries. No external dependencies.
- Apathy is tiny. Single source and header.
- Apathy is cross-platform.
- Apathy is zlib/libpng licensed.

## Motivation
- path manipulation api -> path
- file operation api -> file
- file globbing api -> glob
- data streaming api -> stream
- data decoding api -> pipe
- fs watching api -> watch
- fs mounting api -> mount

## Features
- Stream api: @todoc
- File api: @todoc
- Folder api: @todoc
- MMap api: @todoc

## Todo
- Path manipulation: pwd, split, .., absolute, relative, home, push_back, pop_front, sanitize

## Public API
- `stream::attach(ostream,callback)` @todoc
- `stream::detach(ostream)` @todoc
- `stream::detach()` @todoc
- `stream::make()` @todoc

## Sample
```c++
@todoc
```

## Possible output
```c++
@todoc
```

## Redistribution
- To rebuild from original sources follow instructions on [redist branch.](https://github.com/r-lyeh/apathy/tree/redist)

## Licenses
- [Apathy](https://github.com/r-lyeh/apathy), zlib/libpng licensed.
- [Apathy](https://github.com/dlecocq/apathy) by Dan Lecocq, MIT licensed.
- [Giant](https://github.com/r-lyeh/giant), zlib/libpng licensed.
- [Dessert](https://github.com/r-lyeh/dessert), zlib/libpng licensed.
- [Dirent](http://softagalleria.net/dirent.php) by Toni Ronkko, MIT licensed.
- [FMStream](http://sourceforge.net/projects/fmstream/) by Benichou Software, BSD-3 licensed.

## Alternatives
- https://github.com/yiptool/path-util
- https://github.com/dlecocq/apathy
