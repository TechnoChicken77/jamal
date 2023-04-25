#ifndef MESSAGE
#define MESSAGE

#include "color.cpp"

namespace message
{
    void error(const char* text)
    {
        std::string resutl;
        std::cout << "[";
        color("red", "ERROR");
        std::cout << "]: " << text << '\n';
    }
    void warning(const char* text)
    {
        std::string resutl;
        std::cout << "[";
        color("yellow", "WARNING");
        std::cout << "]: " << text << '\n';
    }
    void succes(const char* text)
    {
        std::string resutl;
        std::cout << "[";
        color("green", "SUCCESS!");
        std::cout << "]: " << text << '\n';
    }
}
#endif