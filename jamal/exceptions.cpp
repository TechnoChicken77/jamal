#ifndef JAMAL_EXCEPTIONS
#define JAMAL_EXCEPTIONS

#include <iostream>
#include <exception>

#include "string-functions.cpp"

namespace jamal_exceptions
{
    class jamal_exception : public std::exception
    {
    public:
        jamal_exception(std::string message){message_ = message;}
        virtual const char* what() const throw() {
            return message_.c_str();
        }
    private:
        std::string message_;
    };
    std::string append_exception_message(std::string base_message, std::string new_message)
    {
        return new_message + '\n' + base_message;
    }
}

#endif