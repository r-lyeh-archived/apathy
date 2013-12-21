#pragma once
#include <string>

struct asset {
    const void * const data = 0;
    const unsigned size = 0;

    explicit
    asset( const std::string &uri );

    operator bool() const {
        return data && size;
    }
};
