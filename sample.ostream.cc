#include <iostream>
#include <string>

#ifdef _WIN32
#   include <Windows.h>
#   pragma comment(lib,"user32.lib")
#endif

#include "sao.hpp"
using namespace sao;

namespace
{
    void default_error_callback( bool open, bool feed, bool close, const std::string &line )
    {
        static std::string cache;

        if( open )
        {
        }
        else
        if( close )
        {
        }
        else
        if( feed )
        {
#ifdef _WIN32
            MessageBoxA( 0, cache.c_str(), 0, MB_ICONERROR );
#endif
            std::cerr << cache << std::endl;
            cache = std::string();
        }
        else
        {
            cache += line;
        }
    }
}

std::ostream &error = stream::make(default_error_callback);

int main( int argc, char **argv )
{
    error << "this is an error sample" << std::endl;
    error << "this is another error sample; errcode(" << (-1) << ")" << std::endl;

    return 0;
}
