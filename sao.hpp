#pragma once
#include <iostream>
#include <string>

namespace sao
{
    namespace stream
    {
        // stream api
        void attach( std::ostream &os, void (*custom_stream_callback)( bool open, bool feed, bool close, const std::string &line ) );
        void detach( std::ostream &os, void (*custom_stream_callback)( bool open, bool feed, bool close, const std::string &line ) );
        void detach( std::ostream &os );

        // stream api (extra)
        std::ostream &make( void (*custom_stream_callback)( bool open, bool feed, bool close, const std::string &line ) );
    }
}
