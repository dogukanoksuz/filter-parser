/**
 * @file main.cpp   
 * @brief Main file for the FilterHandler example
 * @author Doğukan Öksüz
*/

#include "handlers/filter.hpp"

using namespace Pistache;

int main(int argc, char* argv[])
{
    Port port(9080);

    int thr = 2;

    if (argc >= 2)
    {
        port = static_cast<uint16_t>(std::stol(argv[1]));

        if (argc == 3)
            thr = std::stoi(argv[2]);
    }

    Address addr(Ipv4::any(), port);

    std::cout << "Cores = " << hardware_concurrency() << std::endl;
    std::cout << "Using " << thr << " threads" << std::endl;

    FilterHandler handler(addr);

    handler.init(thr);
    handler.start();
}