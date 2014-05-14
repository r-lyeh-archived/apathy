#include <cassert>
#include <iostream>

#include "sao.hpp"

int main() {
    std::string tmpname = sao::file().name();
    assert( tmpname.size() > 0 );

    std::cout << sao::file("test.c").ext() << std::endl;
    std::cout << sao::file("test.exe").ext() << std::endl;

    assert( sao::file(tmpname).overwrite(tmpname) );
    assert( sao::file(tmpname).exist() );
    assert( sao::file(tmpname).append(tmpname) );
    assert( sao::file(tmpname).size() > 0 );
    assert( sao::file(tmpname).read() == tmpname + tmpname );

    time_t date = sao::file(tmpname).date();
    assert( sao::file(tmpname).date() > 0 );
    assert( sao::file(tmpname).touch() );
    assert( sao::file(tmpname).date() >= date );

    assert( sao::file(tmpname).patch("hello world") );
    assert( sao::file(tmpname).read() == "hello world" );

    std::string tmpname2 = sao::file().name();
    assert( sao::file(tmpname).rename(tmpname2) );
    assert( sao::file(tmpname2).remove() );

    sao::file file;
    file.meta("author") = "@rlyeh";

    std::cout << "All ok." << std::endl;
}
