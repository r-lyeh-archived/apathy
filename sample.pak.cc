#include <cassert>

#include <iostream>
#include <string>

#include "pak.hpp"

int main( int argc, char **argv )
{
    std::string binary;

    if( const bool saving_test = true )
    {
        sao::pak pak;

        pak.resize(2);

        pak[0]["filename"] = "test.txt";
        pak[0]["content"] = "hello world";

        pak[1]["filename"] = "test2.txt";
        pak[1]["content"] = 1337;

        std::cout << "zipping files..." << std::endl;

        // save zip archive to memory string (then optionally to disk)
        binary = pak.bin();

        std::cout << "saving test:\n" << pak.debug() << std::endl;
    }

    if( const bool loading_test = true )
    {
        std::cout << "unzipping files..." << std::endl;

        sao::pak pak;
        pak.bin( binary );

        std::cout << "loading test:\n" << pak.debug() << std::endl;

        assert( pak[0]["filename"] == "test.txt" );
        assert( pak[0]["content"] == "hello world" );

        assert( pak[1]["filename"] == "test2.txt" );
        assert( pak[1]["content"] == "1337" );
    }

    std::cout << "All ok." << std::endl;

    return 0;
}
