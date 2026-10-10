#include <StupidINI/StupidINI.hpp>

#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Begining test..." << '\n';
    if (argc != 2) {
        std::cout << "  No path provided for the test ini." << std::endl;
        return 0;
    }

    std::cout << "  1. Loading test INI:" << '\n';
    auto parsed = StupidINI::load_ini(argv[1]);

    if (!parsed) {
        std::cout << "    - Failed to load with error: " << parsed.error() << '\n';
        return 0;
    }
    std::cout << "    - Passed." << std::endl;

    std::cout << "  2. Arbitrary value:" << '\n';
    const auto ini = parsed.value();

    auto fetched = ini.GetValue<int>("Setting", "Section");
    if (!fetched) {
        std::cout << "    - Failed to fetch with error: " << fetched.error() << '\n';
        return 0;
    }
    std::cout << "    - Passed: " << fetched.value() << std::endl;
    return 0;
}