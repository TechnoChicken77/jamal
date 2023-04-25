#ifndef STACKS
#define STACKS

#include <iostream>
#include <cstring>
#include "jamal.hpp"

namespace stacks
{
    std::string to_string(jamal::stack stack)
    {
        std::string result(stack.begin(), stack.end());
        return result;
    }
    long to_long(jamal::stack stack)
    {
        long result;
        u_char bytes[stack.size()];
        std::memcpy(bytes, stack.data(), stack.size());
        std::memcpy(&result, bytes, sizeof(long));
        return result;
    }
    jamal::stack long_to_stack(long l)
    {
        u_char bytes[sizeof(long)];
        std::memcpy(bytes, &l, sizeof(long));
        jamal::stack result(bytes, bytes + sizeof(bytes)/sizeof(unsigned char));
        return result;
    }
    jamal::stack string_to_stack(std::string s)
    {
        jamal::stack result (s.begin(), s.end());
        return result;
    }
    void print_stack(jamal::stack stack)
    {
        std::cout << "[ ";
        for (auto &&i : stack)
        {
            std::cout << (int) i << ' ';
        }
        std::cout << "]\n";
    }
}

#endif