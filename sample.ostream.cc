#include <iostream>
#include <string>
#include "sao.hpp"

namespace {
    void default_error_callback( bool open, bool feed, bool close, const std::string &line ) {
        static std::string cache;

        if( open ) {
        }
        else
        if( close ) {
        }
        else
        if( feed ) {
            std::cerr << "hello from my custom ostream: " << cache << std::endl;
            cache = std::string();
        }
        else {
            cache += line;
        }
    }
}

std::ostream &error = sao::stream::make(default_error_callback);

int main() {
    error << "this is an error sample" << std::endl;
    error << "this is another error sample; errcode(" << (-1) << ")" << std::endl;
}
