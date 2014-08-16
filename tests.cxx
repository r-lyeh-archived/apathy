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


//#line 1 "apathy.hpp"
#pragma once

#include <cassert>
#include <ctime>
#include <cctype>
#include <stdint.h>
#include <string.h>

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
		const char *begin = 0, *end = 0, *cursor = 0, *rw = 0;

		stream();
		stream( const void *from, const void *to );
		stream( const void *ptr, unsigned len );
		stream( void *ptr, unsigned len );

		bool good( unsigned off = 0 );

		bool open();
		bool close();

		bool rewind();
		bool eof();
		unsigned tell();
		unsigned size();
		unsigned left();
		bool seek( unsigned offset );
		bool offset( int offset );

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
	template<typename DATA,typename META = std::string>
	struct pipe {
		enum state { FAIL, KEEP, STOP };

		using filter = std::function<bool(DATA&,META&)>;
		std::vector< filter > filters;

		// basics
		pipe &chain( const filter &filt ) {
			return filters.push_back( filt ), *this;
		}
		bool process(DATA &data, META &meta = META()) const {
			auto st = KEEP;
			for( auto fn : filters ) {
				if( KEEP != ( st = (state)fn( data, meta ) ) )
					break;
			}
			return FAIL != st;
		}

		// aliases
		pipe &operator <<( const filter &filt ) {
			return chain( filt );
		}
		bool operator()(DATA &data, META &meta = META()) const {
			return process( data, meta );
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
		stream chunk( size_t offset, size_t len );

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

		void include( const std::string &path, const std::vector<std::string> &masks = {"*"}, bool recurse_subdirs = true );
		void exclude( const std::string &path, const std::vector<std::string> &masks = {"*"}, bool recurse_subdirs = true );

		std::string str( const char *format1 = "\1\n" ) const;
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
		template<typename ostream>
		inline friend ostream& operator<<( ostream& stream, const path &self ) {
			return stream << self.m_path, stream;
		}

	private:
		/* Our current path */
		std::string m_path;
	};

	// extras

	using uri = std::string;
	using uris = std::set<std::string>;
	using hash = std::tuple< size_t /*filestamp*/, size_t /*filesize*/, std::string /*filename*/ >;
	class watcher;
	using watcher_callback = std::function<void(watcher &)>;

	std::ostream &print( const uris &uris_, std::ostream &out = std::cout );
	std::string replace( const std::string &str_, const std::string &from, const std::string &to );
	std::vector<std::string> split( const std::string &s, char sep );
	std::vector<std::string> wildcards( const std::string &mask );

	bool is_dir( const std::string &uri );
	bool exists( const std::string &uri );
	bool has_changed( const std::string &uri );
	bool matches( const std::string &text, const std::string &pattern );

	std::string normalize( const std::string &uri_ );
	std::string notrails( const std::string &uri );

	void chdir( const std::string &path );
	void pushd();
	void popd();
	std::string cwd();

	void sleep( double t );

	// glob files and directories

	bool glob( const std::string &uri, uris &fs, uris &dirs );
	bool globr( const std::string &uri_, uris &fs, uris &dirs, unsigned recurse = ~0 );

	uris &filter( uris &set, const std::string &includes_ = "*", const std::string &excludes_ = "" );

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

	using mapping = std::map< std::string, std::string >;
	using table_of_contents = std::map< std::string /* phys */, entry >;

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
			return { "file://", "" };
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
		vfilesystem( const vfilesystem & ) = delete;
		vfilesystem &operator=( const vfilesystem & ) = delete;

		filesystem *locate( const std::string &virt ) const;

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
	class suite {
		using timer = chrono::high_resolution_clock;
		timer::time_point start = timer::now();
		deque< string > xpr;
		int ok = false, has_bp = false;
		enum { BREAKPOINT, BREAKPOINTS, PASSED, FAILED, TESTNO };
		static unsigned &get(int i) { static unsigned var[TESTNO+1] = {}; return var[i]; }
		template<typename T> static string to_str( const T &t ) { stringstream ss; return ss << t ? ss.str() : "??"; }
		template<          > static string to_str( const timer::time_point &start ) {
			return to_str( double((timer::now() - start).count()) * timer::period::num / timer::period::den );
		}
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
				fprintf( stderr, xpr[it].size() ? "%s%s\n" : "", tab[ !it ].c_str(), xpr[it].c_str() );
			}
		}
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
	vfs.mount("*.cpp", "code/");
	vfs.mount("**.cpp", "code-r/");

	// flat paths
	vfs.mount("dlc-watchman/*.txt", "/" );

	dessert( vfs.exists("code/extra.cpp") );
	dessert( vfs.exists("file://code/extra.cpp") );

	std::cout << vfs.toc() << std::endl;

	dessert( vfs.read("file://code/extra.cpp") == apathy::file(__FILE__).read() );
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

