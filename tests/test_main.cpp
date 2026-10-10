#include <StupidINI/StupidINI.hpp>

#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Begining test..." << '\n';
    if (argc != 2) {
        std::cout << "  No path provided for the test ini." << std::endl;
        return 0;
    }

    std::cout << "  1. Loading test INI:" << '\n';
    auto ini = StupidINI::load_ini(argv[1]);

    if (!ini) {
        std::cout << "    - Failed to load with error: " << ini.error() << '\n';
        return 0;
    }
    std::cout << "    - Passed." << std::endl;
    return 0;
}