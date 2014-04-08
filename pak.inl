#pragma once

#include <sstream>
#include <string>

namespace sao
{
    class pakstring : public std::string
    {
        public:

        pakstring() : std::string()
        {}

        template< typename T >
        explicit
        pakstring( const T &t ) : std::string()
        {
            operator=(t);
        }

        template< typename T >
        pakstring &operator=( const T &t )
        {
            std::stringstream ss;
            ss.precision(20);
            if( ss << /* std::boolalpha << */ t )
                this->assign( ss.str() );
            return *this;
        }

        template< typename T >
        T as() const
        {
            T t;
            std::stringstream ss;
            ss << *this;
            return ss >> t ? t : T();
        }

        template< typename T0 >
        pakstring( const std::string &_fmt, const T0 &t0 ) : std::string()
        {
            std::string out, t[1] = { pakstring(t0) };

            for( const char *fmt = _fmt.c_str(); *fmt; ++fmt )
            {
                if( *fmt == '\1' )
                    out += t[0];
                else
                    out += *fmt;
            }

            this->assign( out );
        }

        template< typename T0, typename T1 >
        pakstring( const std::string &_fmt, const T0 &t0, const T1 &t1 ) : std::string()
        {
            std::string out, t[2] = { pakstring(t0), pakstring(t1) };

            for( const char *fmt = _fmt.c_str(); *fmt; ++fmt )
            {
                if( *fmt == '\1' )
                    out += t[0];
                else
                if( *fmt == '\2' )
                    out += t[1];
                else
                    out += *fmt;
            }

            this->assign( out );
        }
    };
}
