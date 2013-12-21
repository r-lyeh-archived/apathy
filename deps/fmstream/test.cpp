#include "fmstream.h"
#include <iostream>
#include <iomanip>
int main() {

	filemappingbuf fp;
	fp.open("gritengine-code-2706-trunk.zip", std::ios_base::in );
	unsigned char *buf = (unsigned char *)fp.data();

    std::cout << std::hex << (unsigned)buf[0x0] << std::endl;
    std::cout << std::hex << (unsigned)buf[0x1] << std::endl;
    std::cout << std::hex << (unsigned)buf[0x12] << std::endl;
    std::cout << std::hex << (unsigned)buf[0x123] << std::endl;
    std::cout << std::hex << (unsigned)buf[0x1234] << std::endl;
    std::cout << std::hex << (unsigned)buf[0x12345] << std::endl;
    std::cout << std::hex << (unsigned)buf[0x123456] << std::endl;
    std::cout << std::hex << (unsigned)buf[0x1234567] << std::endl;
}
