#include <iostream>
#include <string>

int main()
{
    std::string input;
    while(true){
        std::cout << "user> " << std::flush;
        if(!std::getline(std::cin, input))
            break;
        std::cout << input << std::endl;
    }
    return 0;
}
