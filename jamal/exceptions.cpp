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
        std::vector<std::string> base_message_lines = string_functions::split(base_message, '\n');
        for (auto &&message_line : base_message_lines)
        {
            message_line = "    " + message_line;
        }
        std::string message;
        for (auto &&message_line : base_message_lines)
        {
            message.append("\n");
            message.append(message_line);
        }
        std::string result = new_message + message;
        return result;
    }
}

#endif