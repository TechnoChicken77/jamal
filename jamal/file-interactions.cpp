#ifndef FILE_INTERRACTIONS
#define FILE_INTERRACTIONS

#include <iostream>
#include <fstream>
#include <vector>

#include "string-functions.cpp"

namespace file
{
    std::string read_text(std::string path)
    {
        std::ifstream file(path);
        std::string result((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return result;
    }
    std::string path_of(std::string path)
    {
        std::string result;
        std::vector<std::string> splitted_path = string_functions::split(path, '/');
        int folder_count = splitted_path.size()-1;
        if(path[0] == '/') result += '/';
        else{if(folder_count == 0) return("");}
        for(int i = 0; i < folder_count; i++)
        {
            result += splitted_path[i] + '/';
        }
        return result;
    }
}

#endif