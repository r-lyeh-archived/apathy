#pragma once
#include <functional>
#include <vector>

template<typename T>
struct pipe {
    using filter = std::function<T(T)>;
    std::vector< filter > filters;

    pipe &chain( const filter &t ) {
        return filters.push_back( t ), *this;
    }

    T process( const T &t ) const {
        T f = t;
        for( auto fn : filters ) {
            f = fn( f );
        }
        return f;
    }

    pipe &operator <<( const filter &t ) {
        return chain( t );
    }

    T operator()( const T &t ) const {
        return process( t );
    }
};

#if 0
struct node {
    std::map< std::string, std::string > data;
    pipe< std::string > in, out;

    node()
    {}
};
#endif
