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

desserts("cwd()", "And equivalent vs ==") {
    path cwd(path::cwd());
    path empty("");
    dessert(cwd != empty);
    dessert(cwd.equivalent(empty));
    dessert(empty.equivalent(cwd));
    dessert(cwd.is_absolute());
    dessert(!empty.is_absolute());
    dessert(empty.absolute() == cwd);
    dessert(path() == "");
}

desserts("operator=()", "Make sure assignment works as expected") {
    path cwd(path::cwd());
    path empty("");
    dessert(cwd != empty);
    empty = cwd;
    dessert(cwd == empty);
}

desserts("operator+=()", "Make sure operator<< works correctly") {
    path root("/");
    root << "hello" << "how" << "are" << "you";
    dessert(root.string() == "/hello/how/are/you");

    /* It also needs to be able to accept things like floats, ints, etc. */
    root = path("/");
    root << "hello" << 5 << "how" << 3.14 << "are";
    dessert(root.string() == "/hello/5/how/3.14/are");
}

desserts("operator+()", "Make sure operator+ works correctly") {
    path root("foo/bar");
    dessert((root + "baz").string() == "foo/bar/baz");
}

desserts("trim()", "Make sure trim actually strips off separators") {
    path root("/hello/how/are/you////");
    dessert(root.trim().string() == "/hello/how/are/you");
    root = path("/hello/how/are/you");
    dessert(root.trim().string() == "/hello/how/are/you");
    root = path("/hello/how/are/you/");
    dessert(root.trim().string() == "/hello/how/are/you");
}

desserts("directory()", "Make sure we can make paths into directories") {
    path root("/hello/how/are/you");
    dessert(root.directory().string() == "/hello/how/are/you/");
    root = path("/hello/how/are/you/");
    dessert(root.directory().string() == "/hello/how/are/you/");
    root = path("/hello/how/are/you//");
    dessert(root.directory().string() == "/hello/how/are/you/");
}

desserts("relative()", "Evaluates relative urls correctly") {
    path a("/hello/how/are/you");
    path b("foo");
    dessert(a.relative(b).string() == "/hello/how/are/you/foo");
    a = path("/hello/how/are/you/");
    dessert(a.relative(b).string() == "/hello/how/are/you/foo");
    b = path("/fine/thank/you");
    dessert(a.relative(b).string() == "/fine/thank/you");
}

desserts("parent()", "Make sure we can find the parent directory") {
    path a("/hello/how/are/you");
    dessert(a.parent().string() == "/hello/how/are/");
    a = path("/hello/how/are/you");
    dessert(a.parent().parent().string() == "/hello/how/");

    /* / is its own parent, at least according to bash:
     *
     *    cd / && cd ..
     */
    a = path("/");
    dessert(a.parent().string() == "/");

    a = path("");
    dessert(a.parent() != path::cwd().parent());
    dessert(a.parent().equivalent(path::cwd().parent()));

    a = path("foo/bar");
    dessert(a.parent().parent() == "");
    a = path("foo/../bar/baz/a/../");
    dessert(a.parent() == "bar/");
}

desserts("md()", "Make sure we recursively make directories") {
    path p("foo");
    dessert(!p.exists());
    p << "bar" << "baz" << "whiz";
    path::md(p);
    dessert(p.exists());
    dessert(p.is_directory());

    /* Now, we should remove the directories, make sure it's gone. */
    dessert(path::rmrf("foo"));
    dessert(!path("foo").exists());
}

desserts("ls()", "Make sure we can list directories") {
    path p("foo");
    p << "bar" << "baz" << "whiz";
    path::md(p);
    dessert(p.exists());

    /* Now touch some files in this area */
    path::touch(path(p).append("a"));
    path::touch(path(p).append("b"));
    path::touch(path(p).append("c"));

    /* Now list that directory */
    std::vector<path> files = path::ls(p);
    dessert(files.size() == 3);

    /* ls() doesn't enforce any ordering */
    dessert((std::find(files.begin(), files.end(),
        path(p).append("a").string()) != files.end()));
    dessert((std::find(files.begin(), files.end(),
        path(p).append("b").string()) != files.end()));
    dessert((std::find(files.begin(), files.end(),
        path(p).append("c").string()) != files.end()));

    dessert(path::rmrf("foo"));
    dessert(!path("foo").exists());
}

desserts("rm()", "Make sure we can remove files we create") {
    dessert(!path("foo").exists());
    path::touch("foo");
    dessert( path("foo").exists());
    path::rm("foo");
    dessert(!path("foo").exists());
}

desserts("mv()", "Make sure we can move files / directories") {
    /* We should be able to move it in the most basic case */
    path source("foo");
    path dest("bar");
    dessert(!source.exists());
    dessert(!  dest.exists());
    path::touch(source);

    dessert(path::mv(source, dest));
    dessert(!source.exists());
    dessert(   dest.exists());

    dessert(path::rm(dest));
    dessert(!source.exists());
    dessert(!  dest.exists());

    /* And now, when the directory doesn't exist */
    dest = "bar/baz";
    dessert(!dest.parent().exists());
    path::touch(source);

    dessert(!path::mv(source, dest));
    dessert( path::mv(source, dest, true));
    dessert(!source.exists());
    dessert(   dest.exists());
    path::rmrf("bar");
    dessert(!path("bar").exists());
}

desserts("normalize()", "Make sure we can normalize a path") {
    path p("foo///bar/a/b/../c");
    dessert(p.normalize() == "foo/bar/a/c");

    p = "../foo///bar/a/b/../c";
    dessert(p.normalize() == "../foo/bar/a/c");

    p = "../../a/b////c";
    dessert(p.normalize() == "../../a/b/c");

    p = "/../../a/b////c";
    dessert(p.normalize() == "/a/b/c");

    p = "/./././a/./b/../../c";
    dessert(p.normalize() == "/c");

    p = "././a/b/c/";
    dessert(p.normalize() == "a/b/c/");
}

desserts("equivalent()", "Make sure equivalent paths work") {
    path a("foo////a/b/../c/");
    path b("foo/a/c/");
    dessert(a.equivalent(b));

    a = "../foo/bar/";
    b = path::cwd().parent().append("foo").append("bar").directory();
    dessert(a.equivalent(b));
}

desserts("split()", "Make sure we can get segments out") {
    path a("foo/bar/baz");
    std::vector<path::Segment> segments(a.split());
    dessert(segments.size() == 3);
    dessert(segments[0].segment == "foo");
    dessert(segments[1].segment == "bar");
    dessert(segments[2].segment == "baz");

    a = path("foo/bar/baz/");
    dessert(a.split().size() == 4);

    a = path("/foo/bar/baz/");
    dessert(a.split().size() == 5);
}

desserts("extension()", "Make sure we can accurately get th file extension") {
    /* Works in a basic way */
    dessert(path("foo/bar/baz.out").extension() == "out");
    /* Gets the outermost extension */
    dessert(path("foo/bar.baz.out").extension() == "out");
    /* Doesn't take extensions from directories */
    dessert(path("foo/bar.baz/out").extension() == "");
}

desserts("stem()", "Make sure we can get the path stem") {
    /* Works in a basic way */
    dessert(path("foo/bar/baz.out").stem() == path("foo/bar/baz"));
    /* Gets the outermost extension */
    dessert(path("foo/bar.baz.out").stem() == path("foo/bar.baz"));
    /* Doesn't take extensions from directories */
    dessert(path("foo/bar.baz/out").stem() == path("foo/bar.baz/out"));

    /* Can be used to successively pop off the extension */
    path a("foo.bar.baz.out");
    a = a.stem(); dessert(a == path("foo.bar.baz"));
    a = a.stem(); dessert(a == path("foo.bar"));
    a = a.stem(); dessert(a == path("foo"));
    a = a.stem(); dessert(a == path("foo"));
}

desserts("glob()", "Make sure glob works") {
    /* We'll touch a bunch of files to work with */
    path::md("foo");
    path::touch("foo/bar");
    path::touch("foo/bar2");
    path::touch("foo/bar3");
    path::touch("foo/baz");
    path::touch("foo/bazzy");
    path::touch("foo/foo");

    /* Make sure we can get it to work in a few basic ways */
    dessert(path::glob("foo/*"   ).size() == 6);
    dessert(path::glob("foo/b*"  ).size() == 5);
    dessert(path::glob("foo/baz*").size() == 2);
    dessert(path::glob("foo/ba?" ).size() == 2);

    /* Now, we should remove the directories, make sure it's gone. */
    dessert(path::rmrf("foo"));
    dessert(!path("foo").exists());
}
