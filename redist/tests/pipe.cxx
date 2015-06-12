
desserts() {
    apathy::pipe< std::string > in, out;

    // setup
     in << []( std::string &h, std::string &flow ) { return h += "1", true; }
        << []( std::string &h, std::string &flow ) { return h += "2", true; }
        << []( std::string &h, std::string &flow ) { return std::reverse( h.begin(), h.end() ), true; };

    out << []( std::string &h, std::string &flow ) { return std::reverse( h.begin(), h.end() ), true; }
        << []( std::string &h, std::string &flow ) { return h.pop_back(), true; }
        << []( std::string &h, std::string &flow ) { return h.pop_back(), true; };

    std::string h = "hello";
    std::cout << in( h ) << h << std::endl;
    std::cout << out( h ) << h << std::endl;
}
