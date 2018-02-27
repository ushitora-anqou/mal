#include <iostream>
#include <string>
#include "reader.hpp"

int main()
{
    std::string input;
    while (true) {
        std::cout << "user> " << std::flush;
        if (!std::getline(std::cin, input)) break;
        // std::cout << input << std::endl;
        auto reader = Reader(input);
        try {
            // while (true) {
            //    auto token = reader.pop();
            //    std::cout << "." << token << ". ";
            //}
            std::cout << reader.read_form()->pr_str() << std::endl;
        }
        catch (std::runtime_error& ex) {
            std::cerr << "RUNTIME_ERROR: " << ex.what() << std::endl;
        }
    }
    return 0;
}
