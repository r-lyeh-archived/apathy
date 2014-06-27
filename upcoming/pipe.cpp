#include "pipe.hpp"
#include <algorithm>
#include <iostream>
#include <string>

int main() {
    pipe< std::string > in, out;

    // setup
     in << []( std::string h ) { return h + "1"; }
        << []( std::string h ) { return h + "2"; }
        << []( std::string h ) { return std::reverse( h.begin(), h.end() ), h; };

    out << []( std::string h ) { return std::reverse( h.begin(), h.end() ), h; }
        << []( std::string h ) { return h.pop_back(), h; }
        << []( std::string h ) { return h.pop_back(), h; };

    std::cout << in( "hello" ) << std::endl;
    std::cout << out( in("hello") ) << std::endl;
}
