#pragma once

#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "pak.inl"

namespace sao
{
    struct pakfile : public std::map< std::string, pakstring >
    {
        bool has_field( const std::string &property ) const;

        bool has_operator( const std::string &plugin, const std::string &opcode ) const;

        template <typename T>
        T get_value( const std::string &property ) const
        {
            return (*this->find( property )).second.as<T>();
        }

        std::string debug( const std::string &format12 = "\t.\1=\2\n" ) const;
    };

    struct paktype
    {
        enum enumeration { ZIP };
    };

    class pak : public std::vector< pakfile >
    {
        public:

        const paktype::enumeration type;

        explicit
        pak( const paktype::enumeration &etype = paktype::ZIP ) : type(etype)
        {}

        // binary serialization

        bool bin( const std::string &bin_import ); //const
        std::string bin(); //const

        // debug

        std::string debug( const std::string &format1 = "\1\n" ) const;
    };
}

namespace pack
{
    // note: <sao/io/bistring.hpp> defines also a compatible 'bistring(s)' type
    typedef std::pair<std::string,std::string> bistring;
    typedef std::vector< bistring > bistrings;
    std::string zip( const bistrings &bs );
}

namespace unpack
{
    // note: <sao/io/bistring.hpp> defines also a compatible 'bistring(s)' type
    typedef std::pair<std::string,std::string> bistring;
    typedef std::vector< bistring > bistrings;
    bistrings zip( const std::string &s );
}
