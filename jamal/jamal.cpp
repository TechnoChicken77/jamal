#ifndef JAMAL
#define JAMAL

#include "jamal.hpp"
#include <iostream>

namespace jamal
{
    //sections
    void section::insert_code(std::vector<std::string> c)
    {
        code = c;
    }
    void section::insert_args(std::vector<std::string> a)
    {
        args = a;
    }
    std::vector<std::string> section::get_code()
    {
        return this->code;
    }
    long define_stack_in_vector(std::vector<stack>& vec)
    {
        stack newstack;
        int stacks = vec.size();
        int empty = -1;
        for (int i = 0; i < stacks; i++)
        {
            stack stack_in_vector = vec[i];
            if(stack_in_vector.size() == 0)
            {
                empty = i;
                break;
            }
        }
        if(empty == -1)
        {
            vec.push_back(newstack);
            return stacks;
        }
        vec[empty] = newstack;
        return empty;
    }
    void empty_stack_in_vector(std::vector<stack>& vec, long stack_id)
    {
        if(vec.size() <= stack_id) return;
        vec[stack_id].clear();
    }
}

#endif