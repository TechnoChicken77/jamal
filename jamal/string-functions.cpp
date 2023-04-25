#ifndef STRING_FUNCTIONS
#define STRING_FUNCTIONS

#include <iostream>
#include <vector>

namespace string_functions
{
    std::vector<std::string> split(std::string s, char c)
    {
        std::vector<std::string> result;
        int len = s.size();
        int last_split = 0;
        int i = 0;
        for(;i < len; i++)
        {
            char current_c = s[i];
            if(current_c == c)
            {
                int split_len = i - last_split;
                if(split_len != 0) result.push_back(s.substr(last_split, split_len));
                last_split = i+1;
            }
        }
        int split_len = i - last_split;
        if(split_len != 0) result.push_back(s.substr(last_split, split_len));
        last_split = i + 1;
        return result;
    }
    std::string parse_import_path(std::string path)
    {
        std::string result;
        int len = path.size();
        for(int i = 0; i < len; i++)
        {
            char c = path[i];
            if(c == '\\')
            {
                if(path[i+1] == 's') {result += ' ';}
                else {result += path[i+1];}
                i++;
            }
            else {result += c;}
        }
        return result;
    }
    std::string remove_front_spaces(std::string s)
    {
        std::string result;
        int len = s.size();
        int i;
        for(i = 0; i < len; i++)
        {
            if(s[i] != ' ') break;
        }
        result = s.substr(i);
        return result;
    }
    std::string remove_back_spaces(std::string s)
    {
        std::string result;
        int len = s.size() - 1;
        int i;
        for(i = len; i >= 0; i--)
        {
            if(s[i] != ' ') break;
        }
        result = s.substr(0, i+1);
        return result;
    }
    std::string stripe_spaces(std::string s)
    {
        int len = s.size();
        int i;
        for(i = 0; i < len; i++)
        {
            if(s[i] != ' ') break;
        }
        s = s.substr(i);
        std::string result;
        len = s.size() - 1;
        for(i = len; i >= 0; i--)
        {
            if(s[i] != ' ') break;
        }
        result = s.substr(0, i+1);
        return result;
    }
}
#endif