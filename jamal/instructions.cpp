#include "jamal.hpp"
#include "stacks.cpp"
#include "message.cpp"
#include "exceptions.cpp"
#include <iostream>
#include <cstring>

namespace instructions
{
    jamal::stack run_section(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        std::string section_name = stacks::to_string(args[0]);
        std::vector<jamal::stack> section_args(args.begin()+1, args.end());
        jamal::variable_map section_variables;
        jamal::stack result = jamal::run_section(section_name, section_args, data, section_variables, true);
        return result;
    }
    jamal::stack get_address_of(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        std::string var_name = stacks::to_string(args[0]);
        jamal::stack result = stacks::long_to_stack(variables.at(var_name).stack_address);
        return result;
    }
    jamal::stack remove_stack(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        long stack_id = stacks::to_long(args[0]);
        jamal::empty_stack_in_vector(data.stacks, stack_id);
        return jamal::stack();
    }
    jamal::stack int_to_string(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        u_char int_bytes[args[0].size()];
        std::memcpy(int_bytes, args[0].data(), args[0].size());
        long int_value;
        std::memcpy(&int_value, (u_char*) int_bytes, sizeof(long));
        //std::cout << "Got value " << int_value << "\n";
        std::string result_string = std::to_string(int_value);
        //std::cout << "Got string " << result_string << "\n";
        jamal::stack result(result_string.begin(), result_string.end());
        return result;
    }
    jamal::stack out(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        std::string msg;
        for (auto &&c : args[0])
        {
            msg.push_back(c);
        }
        std::cout << msg;
        jamal::stack result;
        return result;
    }
    jamal::stack add_variable(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        jamal::stack result;
        std::string type = stacks::to_string(args[0]);
        std::string name = stacks::to_string(args[1]);
        jamal::stack value = args[2];
        bool existed_before = false;

        long stack_id;
        if(variables.find(name) == variables.end()) {stack_id = jamal::define_stack_in_vector(data.stacks);}
        else{stack_id = variables.at(name).stack_address; existed_before = true;}
        if(!existed_before)
        {
            data.stacks[stack_id] = value;
            variables.emplace(jamal::variable_map::value_type(name, jamal::variable_data{stack_id, type}));
        }
        else
        {
            if(type != variables.at(name).type)
            {
                message::error(std::string("incorrect type for variable \"" + name + "\" (must be \"" + variables.at(name).type + "\" but is \"" + type + "\")").c_str());
            }
            else
            {
                data.stacks[stack_id] = value;
            }
        }
        return stacks::long_to_stack(stack_id);
    }
    jamal::stack int_operators(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        long val1 = stacks::to_long(args[0]);
        long val2 = stacks::to_long(args[1]);
        std::string op = stacks::to_string(args[2]);
        jamal::stack result;
        //math operators
        if(op == "+"){result = stacks::long_to_stack(val1 + val2);}
        else if(op == "-") {result = stacks::long_to_stack(val1 - val2);}
        else if(op == "*") {result = stacks::long_to_stack(val1 * val2);}
        else if(op == "/") {result = stacks::long_to_stack(val1 / val2);}
        else if(op == "%") {result = stacks::long_to_stack(val1 % val2);}
        else if(op == "^" || op == "**") {result = stacks::long_to_stack(val1 ^ val2);}
        else //comparison operators
        {
            int n_count = 0;
            int len = op.size();
            for(int i = 0; i < len; i++)
            {
                char c = op[i];
                if(c == '!') n_count++;
                else break;
            }
            bool invert = true;
            if(n_count%2 == 0) invert = false;
            op = op.substr(n_count);

            bool raw_result;

            if(op == "==") raw_result = val1 == val2;
            else if(op == ">") raw_result = raw_result = val1 > val2;
            else if(op == "<") raw_result = raw_result = val1 < val2;
            else if(op == "<=") raw_result = raw_result = val1 <= val2;
            else if(op == ">=") raw_result = raw_result = val1 >= val2;
            else
            {
                result = stacks::long_to_stack(0);
                message::error(std::string("invalid operator: " + op).c_str());
                return result;
            }
            if(invert) raw_result = !raw_result;
            result = stacks::long_to_stack(raw_result);
        }
        return result;
    }
    jamal::stack string_operators(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        std::string val1 = stacks::to_string(args[0]);
        std::string val2 = stacks::to_string(args[1]);
        std::string op = stacks::to_string(args[2]);
        jamal::stack result;
        //math operators
        if(op == "+"){result = stacks::string_to_stack(val1 + val2);}
        else //comparison operators
        {
            int n_count = 0;
            int len = op.size();
            for(int i = 0; i < len; i++)
            {
                char c = op[i];
                if(c == '!') n_count++;
                else break;
            }
            bool invert = true;
            if(n_count%2 == 0) invert = false;
            op = op.substr(n_count);

            bool raw_result;

            if(op == "==") raw_result = val1 == val2;
            else
            {
                result = stacks::long_to_stack(0);
                message::error(std::string("invalid operator: " + op).c_str());
                return result;
            }
            if(invert) raw_result = !raw_result;
            result = stacks::long_to_stack(raw_result);
        }
        return result;
    }
    jamal::stack take(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        long variable_address = stacks::to_long(args[0]);
        long stack_adress = stacks::to_long(args[1]);
        long len = stacks::to_long(args[2]);

        jamal::stack stack = data.stacks[variable_address];
        jamal::stack element = jamal::stack(stack.begin() + stack_adress, stack.begin() + stack_adress + len);
        return element;
    }
    jamal::stack push(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        long variable_address = stacks::to_long(args[0]);
        long stack_adress = stacks::to_long(args[1]);
        jamal::stack value = args[2];

        jamal::stack stack = data.stacks[variable_address];

        long len = value.size();
        for(int i = 0; i < len; i++)
        {
            stack[stack_adress+i] = value[i];
        }
        data.stacks[variable_address] = stack;
        return stack;
    }
    jamal::stack unbind(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        std::string varname = stacks::to_string(args[0]);
        variables.erase(varname);
        return stacks::long_to_stack(0);
    }
    jamal::stack bind(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        long ptr = stacks::to_long(args[0]);
        std::string type = stacks::to_string(args[1]);
        std::string name = stacks::to_string(args[2]);

        jamal::variable_data new_data = {ptr, type};
        variables.emplace(jamal::variable_map::value_type(name, new_data));
        return stacks::long_to_stack(0);
    }
    jamal::stack append_stack(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        long stack_id = stacks::to_long(args[0]);
        jamal::stack value = args[1];

        for (auto &&c : value)
        {
            data.stacks[stack_id].push_back(c);
        }

        return stacks::long_to_stack(0);
    }
    jamal::stack allocate_stack(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        jamal::stack result;
        long size = stacks::to_long(args[0]);
        for(int i = 0; i < size; i++)
        {
            result.push_back(0);
        }
        return result;
    }
    jamal::stack get_stack_size(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        long stack_id = stacks::to_long(args[0]);
        return stacks::long_to_stack(data.stacks[stack_id].size());
    }
    jamal::stack stripe(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        long ptr = stacks::to_long(args[0]);
        long begin = stacks::to_long(args[1]);
        long end = stacks::to_long(args[2]);

        data.stacks[ptr] = jamal::stack(data.stacks[ptr].begin()+begin, data.stacks[ptr].begin() + end);
        return stacks::long_to_stack(0);
    }
    jamal::stack input(std::vector<jamal::stack> args, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        std::string s;
        std::getline(std::cin, s);
        return stacks::string_to_stack(s);
    }
    void load_basic(jamal::instruction_map& i_map)
    {
        i_map.emplace(jamal::instruction_map::value_type("OUT", out));
        i_map.emplace(jamal::instruction_map::value_type("I_TS", int_to_string));
        i_map.emplace(jamal::instruction_map::value_type("VAR", add_variable));
        i_map.emplace(jamal::instruction_map::value_type("RMSTK", remove_stack));
        i_map.emplace(jamal::instruction_map::value_type("ADDOF", get_address_of));
        i_map.emplace(jamal::instruction_map::value_type("CALL", run_section));
        i_map.emplace(jamal::instruction_map::value_type("__INTOP", int_operators));
        i_map.emplace(jamal::instruction_map::value_type("__STROP", string_operators));
        i_map.emplace(jamal::instruction_map::value_type("PUSH", push));
        i_map.emplace(jamal::instruction_map::value_type("TAKE", take));
        i_map.emplace(jamal::instruction_map::value_type("UBIND", unbind));
        i_map.emplace(jamal::instruction_map::value_type("BIND", bind));
        i_map.emplace(jamal::instruction_map::value_type("APPND", append_stack));
        i_map.emplace(jamal::instruction_map::value_type("ALLOC", allocate_stack));
        i_map.emplace(jamal::instruction_map::value_type("SIZEOF", get_stack_size));
        i_map.emplace(jamal::instruction_map::value_type("STRIPE", stripe));
        i_map.emplace(jamal::instruction_map::value_type("IN", input));
    }
    jamal::stack run_instruction(std::string name, jamal::jamal_data& data, std::vector<jamal::stack> args, jamal::variable_map& variables)
    {
        jamal::stack result;
        try
        {
            jamal::instruction_t instruction = data.instructions.at(name);
            result = instruction(args, data, variables);
        }
        catch(std::exception& e)
        {
            std::string exception_message = jamal_exceptions::append_exception_message(e.what(), "While trying to run instruction \"" + name + "\":");
            throw jamal_exceptions::jamal_exception(exception_message);
        }   
        return result;
    }
}