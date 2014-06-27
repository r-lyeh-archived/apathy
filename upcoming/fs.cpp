
#include <iostream>
#include <fstream>
#include "fs.hpp"

namespace {
    filesystem fs;
}

namespace sao {
    bool cmd( const std::string &action, const std::string &rc ) {
        if( fs.call(action, rc) ) {
            std::cout << "[ ok ] >> " << action << ": " << rc << std::endl;
            return true;
        } else {
            std::cout << "[fail] >> " << action << ": " << rc << std::endl;
            return false;
        }
    }
    bool cmd( const std::string &action, const std::string &rc, std::string &out ) {
        if( fs.call(action, rc, out ) ) {
            std::cout << "[ ok ] >> " << action << ": " << rc << std::endl;
            return true;
        } else {
            std::cout << "[fail] >> " << action << ": " << rc << std::endl;
            return false;
        }
    }
    bool open( const std::string &rc ) {
        return cmd( "open", rc );
    }
}

#include <stdarg.h>

struct sao2 {
    bool ok = true;
    std::string f;
    std::vector<std::string> cmd;
    sao2( const std::string &rc ) : f(rc) {
        //ok = sao::cmd( act, rc );
    }
    sao2( const char *fmt, ... ) {
        char buf[128];
        va_list args;
        va_start(args, fmt);
        vsprintf(buf, fmt, args );
        va_end(args);
        f = buf;
    }
    bool good() const {
        return ok;
    }
    operator bool() const {
        return good();
    }
    template<typename FN>
    sao2 &operator>>( const FN &fn ) {
        if( ok ) fn(*this);
        return *this;
    }
    template<size_t N>
    sao2 &operator>>( const char (&str)[N] ) {
        if( ok ) ok = sao::cmd( (cmd.push_back(str), str), f );
        return *this;
    }
    template<typename FN>
    sao2 &operator>>( FN &fn ) {
        if( ok ) ok = sao::cmd( cmd.back(), f, fn );
        return *this;
    }
};

    bool hello( sao2 &f ) {
        std::cout << "hello" << std::endl;
        return true;
    }

struct disk : public plugin {

    std::map< std::string, bool > ok;
    std::map< std::string, std::ifstream > map;
    std::vector< std::vector<char> > queue;

    disk() {
        name = "test";
        description = "test plugin";
        version = "0.0.1";

        if0["open"] = [&]( const std::string &t ) -> bool {
            map[ t ] = std::ifstream( t.c_str(), std::ios::binary );
            ok[ t ] = map[ t ].good();
            return ok[ t ];
        };
        if0["read"] = [&]( const std::string &t ) -> bool {
            using iit = std::istreambuf_iterator<char>;
            auto &ifs = map[ t ];
            std::vector<char> buffer( (iit(ifs)), (iit()) );
            queue.push_back( buffer );
            return ok[ t ];
        };
        if1["read"] = [&]( const std::string &t, std::string &out ) -> bool {
            out = std::string( queue.back().begin(), queue.back().end() );
            return ok[ t ];
        };
        if0["close"] = [&]( const std::string &t ) -> bool {
            auto &ifs = map[ t ];
            ifs.close();
            return ok[ t ];
        };
        if0["mount"] = [&]( const std::string &t ) -> bool {
            ok[ t ] &= true;
            return ok[ t ];
        };
    }
};




int main( int argc, const char **argv ) {

    disk pl;

    //fs.decode("png", {decoder1,decoder2,fallback});
    //fs.encode("png", {decoder1,decoder2,fallback});
    //fs.link("folder1","folder2");

    fs.mount( "/", pl );
    fs.mount( "/pics/", pl );
    fs.mount( "/pics/img/", pl );
    fs.mount( "/pics/textures/", pl );
    fs.mount( "//tmp/", pl );

    std::cout << fs.showcase() << std::endl;

    sao::open("1@root");
    sao::open("/2@root");
    sao::open("/pics/3@pics");
    sao::open("/pics/img/../4@pics");
    sao::open("/pics/img/../img/.//5@img");
    sao::open("////////////////////////6@root");
    sao::open("../../../../../../../../../7@root");
    sao::open("/invalid/path/8@nowhere");

    if( argc > 1 ) {
        std::string data = "data";
        bool ok = sao2("%s",argv[1])
            >> [](sao2&) { std::cout << 1 << std::endl; }
            >> [](sao2&) { std::cout << 2 << std::endl; }
            >> [](sao2&) { std::cout << 3 << std::endl; }
            >> "open" >> "read" >> data >> "close" >> hello;

        if( ok ) {
            std::cout << "perfect! (read " << data.size() << " bytes)" << std::endl;
        } else {
            std::cout << "could not process file: " << argv[1] << std::endl;
        }
    }


    fs.unmount( pl );

}
