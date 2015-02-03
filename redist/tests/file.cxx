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

// -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

desserts("tmpname() - Make sure temp name works") {
    apathy::file f;
    dessert( !f.name().empty() );
}

desserts("good() - Make sure data works") {
    {
        apathy::file f( __FILE__ );
        dessert( f.exists() );
        dessert( f.has_data() );
        dessert( f.size() > 0 );
    }
    {
        apathy::file f;
        dessert( !f.exists() );
        dessert( !f.has_data() );
        dessert( 0 == f.size() );
    }
}

desserts("read() - Make sure preload works") {
    std::string read = apathy::file( __FILE__ ).read();
    dessert( !read.empty() );
}

desserts("chunk() - Mmap") {
    apathy::file open( __FILE__ );
    apathy::stream st1 = open.chunk( 0, 16 );
    dessert( st1.good() );
    int ch;
    ch = st1.getc();
    dessert( ch == int('/') );
    ch = st1.getc();
    dessert( ch == int('*') );
}

