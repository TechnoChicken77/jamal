#include <cstring>

#include "jamal.cpp"
#include "code-parsing.cpp"
#include "message.cpp"
#include "instructions.cpp"
#include "runner.hpp"
#include "exceptions.cpp"

namespace jamal
{
    stack run_section(std::string name, std::vector<stack> args, jamal_data& data, jamal::variable_map& variables, bool destroy_self)
    {
        try
        {
            jamal::section this_section = data.sections.at(name);
            std::vector<std::string> code = this_section.get_code();
            int argc = args.size();

            for(int i = 0; i < argc; i++)
            {
                std::vector<std::string> parsed_arg = string_functions::split(this_section.args[i], ' ');
                long new_var_stack = jamal::define_stack_in_vector(data.stacks);
                data.stacks[new_var_stack] = args[i];
                jamal::variable_data new_var_data {new_var_stack, parsed_arg[0]};
                variables.emplace(jamal::variable_map::value_type(parsed_arg[1], new_var_data));
            }

            bool return_point = false;
            stack result = jamal_runner::run_code_cluster(code, data, variables, return_point);
            if(destroy_self)
            {
                for (auto &&var : variables)
                {
                    if(var.second.type != "none" && var.first != "this")
                    {
                        jamal::type type = data.types.at(var.second.type);
                        std::string destroyer_section_name = type.type_destroyer;
                        if(destroyer_section_name != "none")
                        {
                            jamal::variable_map section_variables;
                            jamal::variable_data this_var_data = var.second;
                            section_variables.emplace(jamal::variable_map::value_type("this", this_var_data));
                            run_section(type.type_destroyer, std::vector<jamal::stack>(), data, section_variables, true);
                        }
                    }
                }
            }
            return result;
        }
        catch(std::exception &e)
        {
            std::string exception_message = jamal_exceptions::append_exception_message(e.what(), "While trying to run section \"" + name + "\":");
            throw jamal_exceptions::jamal_exception(exception_message.c_str());
        }
    }
}

namespace jamal_runner
{
    jamal::stack run_expression(std::string line, jamal::jamal_data& data, jamal::variable_map& variables)
    {
        jamal::stack result;
        try
        {
            using code_parsing::types::expression_type;
            std::vector<std::string> expression_elements = code_parsing::parse_math_expression(line);
            long element_count = expression_elements.size();
            if(element_count >= 3 && line[0] != '>')
            {
                jamal::stack value_buffer;
                std::string element_type_name;

                for(int i = 0; i < element_count; i++)
                {
                    std::string element = expression_elements[i];
                    expression_type raw_element_type = code_parsing::types::get_type(element);
                    jamal::stack element_value;

                    switch (raw_element_type)
                    {
                    //no type specified
                    case expression_type::NULLTYPE:
                        message::error(std::string("could not recognize type of \"" + element + "\" in " + line).c_str());
                        break;
                    //correct types
                    case expression_type::INT:
                        element_type_name = "int";
                        element_value = run_expression(element, data, variables);
                        break;
                    case expression_type::STRING:
                        element_value = run_expression(element, data, variables);
                        element_type_name = "string";
                        break;
                    case expression_type::FLOAT:
                        element_value = run_expression(element, data, variables);
                        element_type_name = "float";
                        break;
                    case expression_type::VARIABLE:
                        element_value = run_expression(element, data, variables);
                        element_type_name = code_parsing::types::get_variable_type(element.substr(1), data, variables);
                        break;
                    case expression_type::CUSTOM:
                        std::vector<std::string> type_and_expression = code_parsing::split_arguments(code_parsing::get_instruction_name_and_args(element)[1]);
                        element_type_name = type_and_expression[0];
                        element_value = run_expression(type_and_expression[1], data, variables);
                        break;
                    }
                    if(raw_element_type != expression_type::OPERATOR) value_buffer = element_value;
                    else
                    {
                        i++;
                        std::string next_expression = expression_elements[i];
                        jamal::stack next_expression_value = run_expression(next_expression, data, variables);

                        std::vector<jamal::stack> argv = {value_buffer, next_expression_value, stacks::string_to_stack(element)};

                        jamal::variable_map op_section_variables;

                        jamal::type element_type = data.types.at(element_type_name);
                        std::string op_section_name = element_type.type_operator;

                        value_buffer = jamal::run_section(op_section_name, argv, data, op_section_variables, true);
                    }
                }
                result = value_buffer;
            }
            else if(line[0] == '(')
            {
                std::string inner_line;
                long p_count = 1;
                long len = line.size();
                for(int i = 1; i<len; i++)
                {
                    char c = line[i];
                    if(c == '(') p_count++;
                    else if (c == ')') (p_count--);

                    if(p_count == 0) break;
                    else inner_line.push_back(c);
                }
                result = run_expression(inner_line, data, variables);
            }
            else
            {
                if(line[0] == '"') //is a string
                {
                    int len = line.size();
                    for(int i = 1; i < len; i++)
                    {
                        char c = line[i];
                        if(c == '"') break;
                        else if(c == '\\')
                        {
                            i++;
                            char c1 = line[i];
                            switch (c1)
                            {
                            case 'n':
                                result.push_back('\n');
                                break;
                            case '"':
                                result.push_back('"');
                                break;                    
                            default:
                                char arr[1];
                                arr[0] = c1;
                                message::error(std::string("unknown special symbol '" + std::string(arr) + '\'').c_str());
                                break;
                            }
                        }
                        else {result.push_back(c);}
                    }
                    return result;
                }
                else if(line[0] == '$') //is a reference to a variable
                {
                    try
                    {
                        std::string var_init(line.begin()+1, line.end());
                        if(var_init[0] == '(') //id mode
                        {
                            var_init = std::string(var_init.begin()+1, var_init.end()-1);
                            long address = stacks::to_long(run_expression(var_init, data, variables));
                            if(address >= data.stacks.size()){throw jamal_exceptions::jamal_exception(std::string("Adress " + std::to_string(address) + " is outside of existing data").c_str());}
                            else result = data.stacks[address];
                        }
                        else //init mode
                        {
                            std::vector<std::string> parsed_init = code_parsing::split_variable(var_init, data);

                            if(parsed_init[1] == "self")
                            {
                                try {result = data.stacks[variables.at(parsed_init[0]).stack_address];}
                                catch(const std::exception& e) {message::error(std::string("failed to load data from variable \"" + var_init + "\"").c_str());}
                            }
                            else if(parsed_init[1] == "destroy")
                            {
                                std::string var_name = parsed_init[0];
                                jamal::type type = data.types.at(variables.at(var_name).type);

                                if(type.type_destroyer != "none")
                                {
                                    jamal::variable_map destroyer_variables;
                                    jamal::variable_data this_var_data = variables.at(var_name);
                                    variables.erase(var_name);
                                    destroyer_variables.emplace(jamal::variable_map::value_type("this", this_var_data));

                                    run_section(type.type_destroyer, std::vector<jamal::stack>(), data, destroyer_variables, true);
                                }
                            }
                            else if(parsed_init[1] == "clone")
                            {
                                jamal::variable_data var_data = variables.at(parsed_init[0]);
                                std::string type_name = var_data.type;
                                if(type_name == "none")
                                {
                                    //just copy already existing data
                                    result = data.stacks[var_data.stack_address];
                                }
                                else
                                {
                                    jamal::type type = data.types.at(type_name);
                                    std::string cloner_name = type.type_cloner;
                                    if(cloner_name == "none")
                                    {
                                        result = data.stacks[var_data.stack_address];
                                    }
                                    else
                                    {
                                        jamal::variable_map cloner_variables;
                                        cloner_variables.emplace(jamal::variable_map::value_type("this", var_data));
                                        result = jamal::run_section(cloner_name, {}, data, cloner_variables, true);
                                    }
                                }
                            }
                            else
                            {
                                jamal::variable_data var_data = variables.at(parsed_init[0]);
                                jamal::stack raw_data = data.stacks[var_data.stack_address];
                                std::string type = var_data.type;
                                std::string referenced_member = parsed_init[1];
                                jamal::member_type referenced_member_type = data.types.at(type).member_types.at(referenced_member);
                                if (referenced_member_type == jamal::member_type::VARIABLE)
                                {
                                    jamal::type_member_data tmd {data.types.at(type).members.at(referenced_member)};
                                    result = std::vector<u_char>(raw_data.begin()+tmd.start_pos, raw_data.begin()+tmd.length-1);
                                }
                                else if(referenced_member_type == jamal::member_type::SUBSECTION)
                                {
                                    std::string subsection_name = data.types.at(type).subsections.at(referenced_member);
                                    std::vector<jamal::stack> args;
                                    std::vector<std::string> raw_args = code_parsing::split_arguments(parsed_init[2]);
                                    for (auto &&raw_arg : raw_args)
                                    {
                                        args.push_back(run_expression(raw_arg, data, variables));
                                    }
                                    jamal::variable_map subsection_variables;
                                    subsection_variables.emplace(jamal::variable_map::value_type("this", var_data));
                                    result = jamal::run_section(subsection_name, args, data, subsection_variables, true);;
                                }           
                            }
                        }
                    }
                    catch(const std::exception& e)
                    {
                        std::string exception_message = jamal_exceptions::append_exception_message(e.what(), "While trying to load data from \"" + line + "\":");
                        throw jamal_exceptions::jamal_exception(exception_message.c_str());
                    }   
                }
                else if(line[0] == '~') //is a type assumption for mathematical expressions
                {
                    std::vector<std::string> parsed_line = code_parsing::get_instruction_name_and_args(line);
                    std::string expression = code_parsing::split_arguments(parsed_line[1])[1];
                    result = run_expression(expression, data, variables);
                }
                else if(code_parsing::is_int(line[0])) //is a numeric expression
                {
                    std::vector<std::string> float_parts = string_functions::split(line, '.');
                    if(float_parts.size() == 1)
                    {
                        //is an int
                        long value = std::stol(line);
                        unsigned char bytes[sizeof(long)];
                        std::memcpy(bytes, &value, sizeof(long));
                        result = (jamal::stack) std::vector<u_char>(bytes, bytes + sizeof(bytes)/sizeof(u_char));
                    }
                    else if(float_parts.size() == 2)
                    {
                        //is a float
                        float value = std::stof(line);
                        unsigned char bytes[sizeof(float)];
                        std::memcpy(bytes, &value, sizeof(float));
                        result = (jamal::stack) std::vector<u_char>(bytes, bytes + sizeof(bytes)/sizeof(u_char));
                    }
                    else
                    {
                        //wtf is wrong with you man?
                        //why would you do that?
                        //you are weird
                        //just
                        //message::error("kill yourself");
                        message::error(std::string("while trying to parse a numeric expression: " + line).c_str());
                    }
                }
                else if(line[0] == '>') //fast writing to an EXISTING variable
                {
                    line = line.substr(1);
                    std::vector<std::string> parsed_line = code_parsing::get_instruction_name_and_args(line);
                    std::string varname = parsed_line[0];
                    jamal::stack new_val = run_expression(parsed_line[1], data, variables);
                    jamal::variable_data var_data = variables.at(varname);
                    long var_id = var_data.stack_address;
                    data.stacks[var_id] = new_val;
                }
                else //is an intsruction
                {
                    std::vector<std::string> parsed_line = code_parsing::get_instruction_name_and_args(line);
                    std::string i_name = parsed_line[0];
                    std::vector<std::string> argv = code_parsing::split_arguments(parsed_line[1]);
                    std::vector<jamal::stack> arguments;
                    for (auto &&arg : argv) {arguments.push_back(run_expression(arg, data, variables));}
                    result = instructions::run_instruction(i_name, data, arguments, variables);
                }
                return result;
            }
        }
        catch(const std::exception& e)
        {
            std::string exception_message = jamal_exceptions::append_exception_message(e.what(), "While trying to run expression \"" + line + "\":");
            throw jamal_exceptions::jamal_exception(exception_message.c_str());
        }
        return result;
    }
    jamal::stack run_code_cluster(std::vector<std::string> code, jamal::jamal_data& data, jamal::variable_map& variables, bool& return_point)
    {
        jamal::stack result;
        int line_count = code.size();
        for(int i = 0; i < line_count; i++)
        {
            std::string line = string_functions::remove_front_spaces(code[i]);
            if(line[0] == '#') //The code line is detected to be a special instruction
            {
                std::vector<std::string> parsed_line = code_parsing::get_instruction_name_and_args(line);
                if(parsed_line[0] == "#return")
                {
                    std::vector<std::string> args = code_parsing::split_arguments(parsed_line[1]);
                    jamal::stack return_value = run_expression(args[0], data, variables);
                    if(args.size() > 1)
                    {
                        for(int i1 = 1; i1 < args.size(); i1++)
                        {
                            run_expression(args[i1], data, variables);
                        }
                    }
                    return_point = true;
                    return return_value;
                }
                else if(parsed_line[0] == "#if")
                {
                    std::string expression = code_parsing::split_arguments(code_parsing::get_instruction_name_and_args(line)[1])[0];
                    long expression_result = stacks::to_long(run_expression(expression, data, variables));
                    bool run_cluster = expression_result > 0;
                    long end_count = 1;
                    i++;
                    std::vector<std::string> internal_code;
                    for(;i < line_count; i++)
                    {
                        std::string internal_line = string_functions::remove_front_spaces(code[i]);
                        if(internal_line[0] == '#')
                        {
                            std::vector<std::string> parsed_internal_line = code_parsing::get_instruction_name_and_args(internal_line);
                            if(parsed_internal_line[0] == "#if" || parsed_internal_line[0] == "#while" || parsed_internal_line[0] == "#else")
                            {
                                end_count++;
                                if(run_cluster) internal_code.push_back(internal_line);
                            }
                            else if(parsed_internal_line[0] == "#end")
                            {
                                end_count--;
                                if(end_count == 0){i++; break;}
                                else if(run_cluster) internal_code.push_back(internal_line);
                            }
                            else if(run_cluster) internal_code.push_back(internal_line);
                        }
                        else if(run_cluster) internal_code.push_back(internal_line);
                    }
                    if(run_cluster)
                    {
                        //actually run the code
                        jamal::stack potential_result = run_code_cluster(internal_code, data, variables, return_point);
                        if(return_point == true)
                        {
                            i--;
                            return potential_result;
                        }
                    }
                    //check if is followed by an else statement
                    if(i+1 < code.size())if(code_parsing::get_instruction_name_and_args(code[i])[0] == "#else")
                    {
                        i++;
                        bool run_else;
                        if(run_cluster) run_else = false;
                        else run_else = true;
                        internal_code.clear();
                        end_count = 1;

                        for(;i < line_count; i++)
                        {
                            std::string internal_line = string_functions::remove_front_spaces(code[i]);
                            if(internal_line[0] == '#')
                            {
                                std::vector<std::string> parsed_internal_line = code_parsing::get_instruction_name_and_args(internal_line);
                                if(parsed_internal_line[0] == "#if" || parsed_internal_line[0] == "#while" || parsed_internal_line[0] == "#else")
                                {
                                    end_count++;
                                    internal_code.push_back(internal_line);
                                }
                                else if(parsed_internal_line[0] == "#end")
                                {
                                    end_count--;
                                    if(end_count == 0){i++; break;}
                                    else internal_code.push_back(internal_line);
                                }
                                else internal_code.push_back(internal_line);
                            }
                            else internal_code.push_back(internal_line);
                        }
                        if(run_else)
                        {
                            //actually run the code
                            jamal::stack potential_result = run_code_cluster(internal_code, data, variables, return_point);
                            if(return_point == true)
                            {
                                i--;
                                return potential_result;
                            }
                        }
                    }
                    i--;
                }
                else if(parsed_line[0] == "#while")
                {
                    std::string expression = parsed_line[1];
                    i++;
                    std::vector<std::string> internal_code;
                    long end_count = 1;

                    for(;i < line_count; i++)
                    {
                        std::string internal_line = string_functions::remove_front_spaces(code[i]);
                        if(internal_line[0] == '#')
                        {
                            std::vector<std::string> parsed_internal_line = code_parsing::get_instruction_name_and_args(internal_line);
                            if(parsed_internal_line[0] == "#if" || parsed_internal_line[0] == "#while" || parsed_internal_line[0] == "#else")
                            {
                                end_count++;
                                internal_code.push_back(internal_line);
                            }
                            else if(parsed_internal_line[0] == "#end")
                            {
                                end_count--;
                                if(end_count == 0)
                                {
                                    i++;
                                    break;
                                }
                                else internal_code.push_back(internal_line);
                            }
                            else internal_code.push_back(internal_line);
                        }
                        else internal_code.push_back(internal_line);
                    }
                    
                    jamal::stack potential_result;
                    while (stacks::to_long(run_expression(expression, data, variables)) > 0 && return_point != true)
                    {
                        potential_result = run_code_cluster(internal_code, data, variables, return_point);
                    }
                    if(return_point == true)
                    {
                        i--;
                        return potential_result;
                    }
                    i--;
                }
                else if(parsed_line[0] == "") {/*do nothing*/} //yep, this can happen, and eventally will braeak if statements
                else {message::error(std::string("unknown member \"" + parsed_line[0] + "\": " + std::to_string(i)).c_str());}
            }
            else //The code line is a normal instruction
            {
                run_expression(line, data, variables);
            }
        }
        return result;
    }
    void run_code(std::string code)
    {
        std::vector<std::string> parsed_code;
        jamal::section_map sections;
        std::vector<std::string> entries;
        jamal::jamal_data data;
        jamal::type_map types;
        try {parsed_code = code_parsing::parse_imports(code);}
        catch(const std::exception& e)
        {
            std::string error = e.what();
            error += " while parsing imports";
            message::error(error.c_str());
            return;
        }
        try {sections = code_parsing::parse_sections(parsed_code);}
        catch(const std::exception& e)
        {
            std::string error = e.what();
            error += " while parsing sections";
            message::error(error.c_str());
            return;
        }
        try {entries = code_parsing::parse_entries(parsed_code);}
        catch(const std::exception& e)
        {
            std::string error = e.what();
            error += " while parsing entries";
            message::error(error.c_str());
            return;
        }
        try {types = code_parsing::parse_types(parsed_code);}
        catch(const std::exception& e)
        {
            std::string error = e.what();
            error += " while parsing types";
            message::error(error.c_str());
            return;
        }

        jamal::instruction_map instructions;
        
        instructions::load_basic(instructions);

        data.entries = entries;
        data.sections = sections;
        data.instructions = instructions;
        data.types = types;
        data.stacks = std::vector<jamal::stack>();

        std::string last_entry;
        try
        {
            for (auto &&entry : entries)
            {
                last_entry = entry;
                jamal::variable_map variables;
                //std::cout << "Calling entry: " << entry << '\n';
                try
                {   
                    run_section(entry, std::vector<jamal::stack>(), data, variables, true);
                }
                catch(jamal_exceptions::jamal_exception &e)
                {
                    std::string message = jamal_exceptions::append_exception_message(e.what(), "While trying to run entry \"" + last_entry + "\": ");
                    throw jamal_exceptions::jamal_exception(message.c_str());
                }
            }
        }
        catch(std::exception &e)
        {
            message::error(std::string("An error occured while trying to run the program: \n" + std::string(e.what())).c_str());
        }
        if(getenv("DEBUG") != nullptr && ((std::string)getenv("DEBUG") == "true"))
        {
            message::succes("program finished!");
            std::cout << "Debug info:\n";
            std::cout << "  Memmory data after finnishing:\n";

            int i = 0;
            for (auto &&stk : data.stacks)
            {
                std::cout << "      " << "At " << i << ": [ ";
                for (auto &&e : stk)
                {
                    std::cout << (int) e << ' ';
                };
                std::cout << "]\n";
                i++;
            }
            
        }
    }
}