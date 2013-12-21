#include "asset.hpp"
#include <iostream>

int main() {

{
    asset logo("http://www.google.com/images/errors/logo_sm.gif");
    std::cout << logo.data << std::endl;
    std::cout << logo.size << std::endl;

    asset gb4("raf1.mkv");
    std::cout << gb4.data << std::endl;
    std::cout << gb4.size << std::endl;

    asset empty("forced.fail");
    std::cout << empty.data << std::endl;
    std::cout << empty.size << std::endl;
}

{
    asset logo("http://www.google.com/images/errors/logo_sm.gif");
    std::cout << logo.data << std::endl;
    std::cout << logo.size << std::endl;

    asset gb4("raf1.mkv");
    std::cout << gb4.data << std::endl;
    std::cout << gb4.size << std::endl;

    asset empty("forced.fail");
    std::cout << empty.data << std::endl;
    std::cout << empty.size << std::endl;
}

    system("pause");

    return 0;
}
