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

#include <algorithm>
#include <cassert>
#include <iostream>

#include "apathy.hpp"

//#line 1 "dessert.hpp"
/* Public API */
#define dessert(...) ( bool(__VA_ARGS__) ? \
		( dessert::suite(#__VA_ARGS__,__FILE__,__LINE__,1) < __VA_ARGS__ ) : \
		( dessert::suite(#__VA_ARGS__,__FILE__,__LINE__,0) < __VA_ARGS__ ) )
#define desserts(...) \
		static void dessert$line(dessert)(); \
		static const bool dessert$line(dsstSuite_) = dessert::suite::queue( [&](){ \
			dessert(1)<< "start of suite: " __VA_ARGS__; \
			dessert$line(dessert)(); \
			dessert(1)<< "end of suite: " __VA_ARGS__; \
			}, "" #__VA_ARGS__ ); \
		void dessert$line(dessert)()
#define throws(...) ( [&](){ try { __VA_ARGS__; } catch( ... ) { return true; } return false; }() )

/* Private API */
#pragma once
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <deque>
#include <functional>
#include <string>
#include <sstream>
namespace dessert {
	using namespace std;
	using timer = chrono::high_resolution_clock;
	template<typename T> inline string to_str( const T &t ) { stringstream ss; return (ss << t) ? ss.str() : "??"; }
	template<          > inline string to_str( const timer::time_point &start ) {
		return to_str( double((timer::now() - start).count()) * timer::period::num / timer::period::den );
	}
	class suite {
		timer::time_point start = timer::now();
		deque< string > xpr;
		int ok = false, has_bp = false;
		enum { BREAKPOINT, BREAKPOINTS, PASSED, FAILED, TESTNO };
		static unsigned &get(int i) { static unsigned var[TESTNO+1] = {}; return var[i]; }
	public:
		static bool queue( const function<void()> &fn, const string &text ) {
			static auto start = timer::now();
			static struct install : public deque<function<void()>> {
				install() : deque<function<void()>>() {
					get(BREAKPOINT) = stoul( getenv("BREAKON") ? getenv("BREAKON") : "0" );
				}
				~install() {
					for( auto &fn : *this ) fn();
					string ss, run = to_str( get(PASSED)+get(FAILED) ), res = get(FAILED) ? "[FAIL]  " : "[ OK ]  ";
					if( get(FAILED) ) ss += res + "Failure! " + to_str(get(FAILED)) + '/'+ run + " tests failed :(\n";
					else              ss += res + "Success: " + run + " tests passed :)\n";
					ss += "        Breakpoints: " + to_str( get(BREAKPOINTS) ) + " (*)\n";
					ss += "        Total time: " + to_str(start) + " seconds.\n";
					fprintf( stderr, "\n%s", ss.c_str() );
				}
			} queue;
			return text.find("before main()") == string::npos ? ( queue.push_back( fn ), 0 ) : ( fn(), 1 );
		}
		suite( const char *const text, const char *const file, int line, bool result )
		:   xpr( {string(file) + ':' + to_str(line), " - ", text, "" } ), ok(result) {
			xpr[0] = "Test " + to_str(++get(TESTNO)) + " at " + xpr[0];
			if( 0 != ( has_bp = ( get(TESTNO) == get(BREAKPOINT) )) ) {
				get(BREAKPOINTS)++;
				fprintf( stderr, "<dessert/dessert.hpp> says: breaking on test #%d\n\t", get(TESTNO) );
					assert(! "<dessert/dessert.hpp> says: breakpoint requested" );
				fprintf( stderr, "%s", "\n<dessert/dessert.hpp> says: breakpoint failed!\n" );
			};
		}
		~suite() {
			if( xpr.empty() ) return;
			operator bool(), queue( [&](){ get(ok ? PASSED : FAILED)++; }, "before main()" );
			string res[] = { "[FAIL]", "[ OK ]" }, bp[] = { "  ", " *" }, tab[] = { "        ", "" };
			xpr[0] = res[ok] + bp[has_bp] + xpr[0] + " (" + to_str(start) + " s)" + (xpr[1].size() > 3 ? xpr[1] : tab[1]);
			xpr.erase( xpr.begin() + 1 );
			if( ok ) xpr = { xpr[0] }; else {
				xpr[2] = xpr[2].substr( xpr[2][2] == ' ' ? 3 : 4 );
				xpr[1].resize( (xpr[1] != xpr[2]) * xpr[1].size() );
				xpr.push_back( "(unexpected)" );
			}
			for( unsigned it = 0; it < xpr.size(); ++it ) {
			 if(xpr[it].size()) fprintf( stderr, "%s%s\n", tab[ !it ].c_str(), xpr[it].c_str() );
		} }
#       define dessert$join(str, num) str##num
#       define dessert$glue(str, num) dessert$join(str, num)
#       define dessert$line(str)      dessert$glue(str, __LINE__)
#       define dessert$impl(OP) \
		template<typename T> suite &operator OP( const T &rhs         ) { return xpr[3] += " "#OP" " + to_str(rhs), *this; } \
		template<unsigned N> suite &operator OP( const char (&rhs)[N] ) { return xpr[3] += " "#OP" " + to_str(rhs), *this; }
		template<typename T> suite &operator <<( const T &t           ) { return xpr[1] += to_str(t),               *this; }
		template<unsigned N> suite &operator <<( const char (&str)[N] ) { return xpr[1] += to_str(str),             *this; }
		operator bool() {
			return xpr.size() >= 3 && xpr[3].size() >= 6 && [&]() -> bool {
				char sign = xpr[3].at(xpr[3].size()/2+1);
				bool equal = xpr[3].substr( 4 + xpr[3].size()/2 ) == xpr[3].substr( 3, xpr[3].size()/2 - 3 );
				return ok = ( sign == '=' ? equal : ( sign == '!' ? !equal : ok ) );
			}(), ok;
		}
		dessert$impl( <); dessert$impl(<=); dessert$impl( >); dessert$impl(>=);
		dessert$impl(!=); dessert$impl(==); dessert$impl(&&); dessert$impl(||);
	};
}


using namespace apathy;


//#line 1 "file.cxx"
// -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

desserts("tmpname()", "Make sure temp name works") {
	apathy::file f;
	dessert( !f.name().empty() );
}

desserts("good()", "Make sure data works") {
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

desserts("read()", "Make sure preload works") {
	std::string read = apathy::file( __FILE__ ).read();
	dessert( !read.empty() );
}

desserts("chunk()", "Mmap") {
	apathy::file open( __FILE__ );
	apathy::stream st1 = open.chunk( 0, 16 );
	dessert( st1.good() );
	int ch;
	ch = st1.getc();
	dessert( ch == int('/') );
	ch = st1.getc();
	dessert( ch == int('*') );
}


//#line 1 "filesystem.cxx"
namespace game {
	struct texture {
		int id;

		texture()
		{}

		texture( const void *ptr, size_t len ) : id(0) {
			if( ptr ) {
				// do whatever
				// glBindTexture
			} else {
				id = 0; // placeholder
			}
		}
	};
};

desserts("file globbing") {
	auto list = apathy::lsfr("*.cxx;");
	dessert( list.size() > 0 );
}

desserts("absolute file globbing") {
	uris files;
	uris dirs;

#ifdef _WIN32
	std::string tmpdir = getenv("TEMP") ? getenv("TEMP") : "";
#else
	std::string tmpdir = getenv("TMPDIR") ? getenv("TMPDIR") : "";
#endif

	if( !tmpdir.empty() ) {
		files.clear();
		dirs.clear();
		dessert( apathy::mkdir(tmpdir + "/apathy") );
		dessert( apathy::exists(tmpdir + "/apathy") );
		dessert( apathy::rmdir( tmpdir + "/apathy" ) );

		dessert( apathy::mkdir(tmpdir + "/apathy") );
		dessert( apathy::globr(tmpdir, files, dirs) );
		assert( files.find(tmpdir + "/apathy") == files.end() );
	  //assert( dirs.find(tmpdir + "/apathy") != dirs.end() );
	}

#ifdef _WIN32
	files.clear();
	dirs.clear();
	dessert( apathy::glob("C:", files, dirs) );
	for(auto &file : files ) std::cout << file << std::endl;
	assert( files.find("c:apathy.cpp") != files.end() );
	assert( dirs.find("c:redist") != dirs.end()  );

	files.clear();
	dirs.clear();
	dessert( apathy::glob("C:/", files, dirs) );
	for(auto &file : files ) std::cout << file << std::endl;
	assert( files.find("c:/apathy.cpp") == files.end() );
	assert( dirs.find("c:/redist") == dirs.end() );
#endif
}

desserts("folder monitoring") {
#if 0
	std::cout << "save " << __FILE__ << " to exit..." << std::endl;
	apathy::watch( "./", [&]( apathy::watcher &wt ) {
		std::cout << "\tin:" << std::endl;
		apathy::print( wt.in );

		std::cout << "\tout:" << std::endl;
		apathy::print( wt.out );

		std::cout << "\tmodif:" << std::endl;
		apathy::print( wt.modif );

		wt.sleep( 1.0 );
		wt.reload();
	} );

	for(;;) {
		apathy::sleep(1.0);
	}
#endif
}

desserts("filesystems") {
	apathy::vfilesystem vfs;

	// register std_io_filesystem to handle "file://" and empty protocol urls
	vfs.add_filesystem<apathy::std_io_filesystem>( {"file://", ""} );

	// test
	game::texture tex;
	tex = vfs.make<game::texture>( "test.tga" );
	vfs.make( tex, "test.tga" );

	vfs.mount("tests/*", "my_tests/");
	vfs.mount("music.zip/*", "music/");
	vfs.mount("textures/*", "icons/");
	vfs.mount("*.c*", "code/");
	vfs.mount("**.cpp", "code-r/");

	// flat paths
	vfs.mount("dlc-watchman/*.txt", "/" );

	dessert( vfs.exists("code/tests.cxx") );
	dessert( vfs.exists("file://code/tests.cxx") );

	std::cout << vfs.toc() << std::endl;

	dessert( vfs.read("file://code/tests.cxx") == apathy::file(__FILE__).read() );
}


//#line 1 "path.cxx"
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


//#line 1 "pipe.cxx"
desserts() {
	apathy::pipe< std::string > in, out;

	// setup
	 in << []( std::string &h, std::string &flow ) { return h += "1", true; }
		<< []( std::string &h, std::string &flow ) { return h += "2", true; }
		<< []( std::string &h, std::string &flow ) { return std::reverse( h.begin(), h.end() ), true; };

	out << []( std::string &h, std::string &flow ) { return std::reverse( h.begin(), h.end() ), true; }
		<< []( std::string &h, std::string &flow ) { return h.pop_back(), true; }
		<< []( std::string &h, std::string &flow ) { return h.pop_back(), true; };

	std::string h = "hello";
	std::cout << in( h ) << h << std::endl;
	std::cout << out( h ) << h << std::endl;
}


//#line 1 "stream.cxx"
desserts() {
	apathy::stream s;
	assert( !s.open() );
	assert( !s.close() );
	assert( s.size() == 0 );
	assert( s.tell() == 0 );
	assert( s.left() == 0 );
	assert( s.eof() == true );
	assert( s.rewind() == false );
	assert( s.offset(0) == false );
	assert( s.seek(0) == false );
	assert( s.offset(~0) == false );
	assert( s.seek(~0) == false );
	int i;
	assert( s.read(i) == false );
	assert( s.write(i) == false );

	{
		const char *hello_world = "hello world";
		s = apathy::stream( hello_world, strlen(hello_world) );
		assert( s.open() );
		assert(!s.write32('1234') );
		assert( std::string(hello_world) == "hello world" );
	}
	{
		char hello_world[] = "hello world";
		s = apathy::stream( hello_world, strlen(hello_world) );
		assert( s.open() );
		int32_t val = '1234';
		assert( s.write32(val) ); // host to le
		assert( std::string(hello_world) == "4321o world" || std::string(hello_world) == "1234o world" );
		assert( s.rewind() );
		assert( s.read(val) );
		assert( val == '1234' );
	}
	{
		std::string h( "son of a " );
		for (int i = 0; i < 2; ++i) h += h;
		s = apathy::stream( &h[0], h.size() );
		std::cout << s << std::endl;
	}
}

int main() {
	/*
	//desserts("preload()", "threaded") {
	apathy::file f( __FILE__ );
	bool ok = false, done = false;
	auto thread = f.preload( [&]{ done = true; ok = true; }, [&]{ done = true; ok = false; } );
	dessert( done == false );
	thread.join();
	dessert( done == true );
	dessert( ok == true );
	*/
}

