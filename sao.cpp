/*
 * Sao is a lightweight IO C++11 library with no dependencies.
 * Copyright (c) 2011,2012,2013 Mario 'rlyeh' Rodriguez

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

 * To do:
 * - thread safe?

 * - rlyeh
 */

#include <deque>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "sao.hpp"

// api

namespace sao
{
    namespace detail
    {
        std::deque<std::string> split( const std::string &str, char sep )
        {
            std::deque<std::string> tokens;

            tokens.push_back( std::string() );

            for( std::string::const_iterator it = str.begin(), end = str.end(); it != end; ++it )
            {
                if( *it == sep )
                {
                    tokens.push_back( std::string() + sep );
                    tokens.push_back( std::string() );
                }
                else
                {
                    tokens.back() += *it;
                }
            }

            return tokens;
        }

        class sbb : public std::streambuf
        {
            public:

            typedef void (*proc)( bool open, bool feed, bool close, const std::string &text );
            typedef std::set< proc > set;
            set cb;

            sbb()
            {}

            sbb( void (*cbb)( bool, bool, bool, const std::string & ) )
            {
                insert( cbb );
            }

            ~sbb()
            {
                clear();
            }

            void log( const std::string &line )
            {
                if( !line.size() )
                    return;

                std::deque<std::string> lines = split( line, '\n' );

                for( set::iterator jt = cb.begin(), jend = cb.end(); jt != jend; ++jt )
                    for( std::deque<std::string>::iterator it = lines.begin(), end = lines.end(); it != end; ++it )
                    {
                        if( *it != "\n" )
                            (**jt)( false, false, false, *it );
                        else
                            (**jt)( false, true, false, std::string() );
                    }
            }

            virtual int_type overflow( int_type c = traits_type::eof() )
            {
                log( std::string() + (char)(c) );

                return 1;
            }

            virtual std::streamsize xsputn( const char *c_str, std::streamsize n )
            {
                log( std::string( c_str, n ) );

                return n;
            }

            void clear()
            {
                for( set::iterator jt = cb.begin(), jend = cb.end(); jt != jend; ++jt )
                    (**jt)( false, false, true, std::string() );
                cb.clear();
            }

            void insert( proc p )
            {
                if( !p )
                    return;

                // make a dummy call to ensure any static object of this callback are deleted after ~sbb() call (RAII)
                p( 0, 0, 0, std::string() );
                p( true, false, false, std::string() );

                // insert into map
                cb.insert( p );
            }

            void erase( proc p )
            {
                p( false, false, true, std::string() );
                cb.erase( p );
            }
        };
    }
}

namespace
{
    struct captured_ostream
    {
        std::streambuf *copy;
        sao::detail::sbb sb;

        captured_ostream() : copy(0)
        {}
    };

    std::map< std::ostream *, captured_ostream > loggers;
}

namespace sao
{
namespace stream
{
    void attach( std::ostream &_os, void (*custom_stream_callback)( bool open, bool feed, bool close, const std::string &line ) )
    {
        std::ostream *os = &_os;

        ( loggers[ os ] = loggers[ os ] );

        if( !loggers[ os ].copy )
        {
            // capture ostream
            loggers[ os ].copy = os->rdbuf( &loggers[ os ].sb );
        }

        loggers[ os ].sb.insert( custom_stream_callback );
    }

    void detach( std::ostream &_os, void (*custom_stream_callback)( bool open, bool feed, bool close, const std::string &line ) )
    {
        std::ostream *os = &_os;

        attach( _os, custom_stream_callback );

        loggers[ os ].sb.erase( custom_stream_callback );

        if( !loggers[ os ].sb.cb.size() )
        {
            // release original stream
            os->rdbuf( loggers[ os ].copy );
        }
    }

    void detach( std::ostream &_os )
    {
        std::ostream *os = &_os;

        ( loggers[ os ] = loggers[ os ] ).sb.clear();

        // release original stream
        os->rdbuf( loggers[ os ].copy );
    }

    std::ostream &make( void (*proc)( bool open, bool feed, bool close, const std::string &line ) )
    {
        static struct container
        {
            std::map< void (*)( bool open, bool feed, bool close, const std::string &text ), sao::detail::sbb > map;
            std::vector< std::ostream * > list;

            container()
            {}

            ~container()
            {
                for( auto it = list.begin(), end = list.end(); it != end; ++it )
                    delete *it;
            }

            std::ostream &insert( void (*proc)( bool open, bool feed, bool close, const std::string &text ) )
            {
                ( map[ proc ] = map[ proc ] ) = sao::detail::sbb(proc);

                list.push_back( new std::ostream( &map[proc] ) );
                return *list.back();
            }
        } _;

        return _.insert( proc );
    }
} // stream::
} // sao::

