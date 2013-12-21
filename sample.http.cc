#include <cassert>

#include <string>
#include <iostream>

#include "http.hpp"

int main( int argc, char **argv )
{
	std::string content = sao::download("http://www.google.com");
	assert( content.size() > 0 );

	std::cout << content << std::endl;

	std::cout << "All ok." << std::endl;
	return 0;
}
