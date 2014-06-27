#include <cassert>
#include <iostream>

#include "apathy.hpp"

int main() {
    std::string tmpname = apathy::file().name();
    assert( tmpname.size() > 0 );

    std::cout << apathy::file("test.c").ext() << std::endl;
    std::cout << apathy::file("test.exe").ext() << std::endl;

    assert( apathy::file(tmpname).overwrite(tmpname) );
    assert( apathy::file(tmpname).exist() );
    assert( apathy::file(tmpname).append(tmpname) );
    assert( apathy::file(tmpname).size() > 0 );
    assert( apathy::file(tmpname).read() == tmpname + tmpname );

    time_t date = apathy::file(tmpname).date();
    assert( apathy::file(tmpname).date() > 0 );
    assert( apathy::file(tmpname).touch() );
    assert( apathy::file(tmpname).date() >= date );

    assert( apathy::file(tmpname).patch("hello world") );
    assert( apathy::file(tmpname).read() == "hello world" );

    std::string tmpname2 = apathy::file().name();
    assert( apathy::file(tmpname).rename(tmpname2) );
    assert( apathy::file(tmpname2).remove() );

    apathy::file file;
    file.meta("author") = "@rlyeh";

    std::cout << "All ok." << std::endl;
}
