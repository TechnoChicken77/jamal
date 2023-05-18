#ifndef CODE_PARSING
#define CODE_PARSING

#include <iostream>
#include <vector>
#include <algorithm>

#include "exceptions.cpp"
#include "file-interactions.cpp"
#include "string-functions.cpp"
#include "jamal.cpp"
#include "message.cpp"

namespace code_parsing
{
    bool is_int(char c);
    bool is_op(char c);
    std::vector<std::string> split_variable(std::string* initializer);
    namespace types
    {
        enum expression_type
        {
            INT, STRING, FLOAT, CUSTOM, OPERATOR, NULLTYPE, VARIABLE
        };
        jamal::type parse_type(std::vector<std::string> code)
        {
            jamal::type result;
            int len = code.size();
            for(int i = 0; i < len; i++)
            {
                std::string line = code[i];
                if(line[0] == '#')
                {
                    std::vector<std::string> parsed_line = string_functions::split(&line, ' ');
                    if(parsed_line[0] == "#member")
                    {
                        std::string member_name = parsed_line[1];
                        long start_pos = std::stol(parsed_line[2]);
                        long member_len = std::stol(parsed_line[3]);

                        result.members.emplace(jamal::type_member_map::value_type(member_name, jamal::type_member_data {start_pos, member_len}));
                        result.member_types.emplace(jamal::member_type_map::value_type(member_name, jamal::member_type::VARIABLE));
                    }
                    else if(parsed_line[0] == "#subsection")
                    {
                        std::string subsection_name = parsed_line[1];
                        std::string coresponding_section = parsed_line[2];
                        result.subsections.emplace(jamal::subsection_map::value_type(subsection_name, coresponding_section));
                        result.member_types.emplace(jamal::member_type_map::value_type(subsection_name, jamal::member_type::SUBSECTION));
                    }
                    else if(parsed_line[0] == "#operator")
                    {
                        std::string operator_type = parsed_line[1];
                        jamal::operator_action_type operator_action;
                        std::string operator_name = parsed_line[2];
                        
                        if(operator_type == "section")
                        {
                            operator_action = jamal::operator_action_type::SECTION;
                        }
                        else if(operator_type == "instruction")
                        {
                            operator_action = jamal::operator_action_type::INSTRUCTION;
                        }
                        else
                        {
                            throw jamal_exceptions::jamal_exception("unknown operator type \"" + operator_type + "\"");
                        }
                        result.type_operator = {operator_action, operator_name};
                    }
                    else if(parsed_line[0] == "#destroyer")
                    {
                        result.type_destroyer = parsed_line[1];
                    }
                    else if(parsed_line[0] == "#clone")
                    {
                        result.type_cloner = parsed_line[1];
                    }
                    else
                    {
                        //seems like sb donesn't know how jamal types work
                        message::error(std::string("invalid type initializer argument: " + parsed_line[0]).c_str());
                        return jamal::empty_type;
                    }
                }
            }
            return result;
        }
        expression_type get_type(std::string* expression)
        {
            char identifier = (*expression)[0];
            if(is_int(identifier)) //either int or float
            {
                if(string_functions::split(expression, ' ').size() == 1) return expression_type::INT;
                else return expression_type::FLOAT;
            }
            else if(is_op(identifier)) return expression_type::OPERATOR; //is an operator
            else if(identifier == '"') return expression_type::STRING; //is a string
            else if(identifier == '~') return expression_type::CUSTOM; //a custom type
            else if(identifier == '$') return expression_type::VARIABLE; //a variable type, ore type or its element
            //bruh those comments above this one are as useless as this one
            else return expression_type::NULLTYPE;
        }
        std::string get_variable_type(std::string* variable, jamal::variable_map* variables)
        {
            std::vector<std::string> parsed_variable = split_variable(variable);
            std::string name = parsed_variable[0];
            jamal::variable_data parent_data = (*variables).at(name);
            if(parsed_variable[1] == "self")
            {
                return parent_data.type;
            }
            else
            {
                return "none";
            }
        }
    };
    std::vector<std::string> get_instruction_name_and_args(std::string* line);
    std::vector<std::string> split_arguments(std::string arg_line);
    bool is_int(char c)
    {
        const char ints[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
        for (auto &&i : ints)
        {
            if(i == c) return true;
        }
        return false;
    }
    bool is_op(char c)
    {
        const char operators[10] = {'=', '<', '>', '!', '+', '-', '*', '/', '%', '^'};
        for (auto &&i : operators)
        {
            if(i == c) return true;
        }
        return false;
    }
    std::vector<std::string> parse_imports(std::string code, std::string path, std::vector<std::string>& libraries)
    {
        std::vector<std::string> result;
        std::vector<std::string> code_lines = string_functions::split(&code, '\n');
        int line_count = code_lines.size();
        for(int i = 0; i < line_count; i++)
        {
            std::string line = code_lines[i];
            std::vector<std::string> parsed_line = string_functions::split(&line, ' ');
            if(parsed_line[0][0] == '/')
            {
                //do nothing, it's a fucking comment
            }
            else if(parsed_line[0] == "#import")
            {
                std::string import_path = string_functions::parse_import_path(parsed_line[1]);
                if(import_path[0] != '/')
                {
                    import_path = path + import_path;
                }
                std::string imported_code = file::read_text(import_path);
                std::string path_of_import = file::path_of(import_path);
                std::vector<std::string> parsed_import_code = parse_imports(imported_code, path_of_import, libraries);
                for (auto &&l : parsed_import_code) {result.push_back(l);}
            }
            else if(parsed_line[0] == "@library")
            {
                std::string lib_name = parsed_line[1];
                if(std::find(libraries.begin(), libraries.end(), lib_name) == libraries.end())
                {
                    libraries.push_back(lib_name);
                }
                else
                {
                    break;
                }
            }
            else{result.push_back(string_functions::stripe_spaces(line));}
        }
        return result;
    }
    jamal::section_map parse_sections(std::vector<std::string> code)
    {
        jamal::section_map result;
        int code_len = code.size();
        for (int i = 0; i < code_len; i++)
        {
            std::string line = code[i];
            std::vector<std::string> parsed_line = string_functions::split(&line, ' ');
            if(parsed_line[0] == "#section")
            {
                i++;
                std::vector<std::string> section_code;
                std::string section_name = parsed_line[1];
                for(;i<code_len;i++)
                {
                    std::vector<std::string> section_line = string_functions::split(&code[i], ' ');
                    if(section_line[0] == "#endsection") break;
                    section_code.push_back(string_functions::remove_front_spaces(code[i]));
                }
                jamal::section section;
                section.insert_code(section_code);
                std::string args = get_instruction_name_and_args(&line)[1];
                std::vector<std::string> parsed_args = split_arguments(args);
                for (auto &&a : parsed_args)
                {
                    a = string_functions::remove_back_spaces(a);
                }
                
                section.insert_args(parsed_args);
                result.insert(jamal::section_map::value_type(section_name, section));
            }
        }
        return result;
    }
    std::vector<std::string> parse_entries(std::vector<std::string> code)
    {
        std::vector<std::string> result;
        int len = code.size();
        for(int i = 0; i < len; i++)
        {
            std::string parsed_line = string_functions::remove_front_spaces(code[i]);
            if(parsed_line[0] == '@') if (parsed_line == "@entry")
            {
                std::string entry = string_functions::split(&code[i+1], ' ')[1];
                //std::cout << "Found entry: " << entry << '\n';
                result.push_back(entry);
            }
        }
        return result;
    }
    jamal::type_index_map parse_types(std::vector<std::string> code, std::vector<jamal::type>& typev)
    {
        jamal::type_index_map result;
        int len = code.size();
        int type_id = -1;
        for(int i = 0; i<len; i++)
        {
            std::string line = code[i];
            if(line[0] == '#')
            {
                std::vector<std::string> parsed_line = string_functions::split(&line, ' ');
                if(parsed_line[0] == "#type")
                {
                    std::vector<std::string> type_code_lines;
                    for(i++;i<len;i++)
                    {
                        std::string type_code_line = string_functions::remove_front_spaces(code[i]);
                        if(type_code_line[0] == '#')
                        {
                            if(type_code_line.substr(0,9) == "#endtype") {break;}
                        }
                        type_code_lines.push_back(type_code_line);
                    }
                    //all of type code was separated at this point
                    type_id++;
                    jamal::type type = types::parse_type(type_code_lines);
                    typev.push_back(type);
                    result.emplace(jamal::type_index_map::value_type(parsed_line[1], type_id));
                }
            }
        }
        return result;
    }
    std::vector<std::string> get_instruction_name_and_args(std::string* line)
    {
        std::vector<std::string> result = {"", ""};
        int len = (*line).size();
        bool has_name = false;
        bool has_args = false;

        bool in_str = false;
        int p_count = 0;

        int arg_start = 0;
        int arg_len = len;

        for(int i = 0; i<len; i++)
        {
            char c = (*line)[i];

            switch (c)
            {
            case '(':
                if(!in_str)
                {
                    if(!has_args)
                    {
                        has_args = true;
                        arg_start = i+1;
                    }
                    p_count++;
                }
                break;
            case ')':
                if(!in_str)
                {
                    p_count--;
                    if(has_args)
                    {
                        if(p_count == 0)
                        {
                            arg_len = i - arg_start;
                        }
                    }
                }
                break;
            case '"':
                if((*line)[i-1] != '\\') in_str = !in_str;
                break;
            }
        }
        if(has_args)
        {
            result[0] = string_functions::remove_front_spaces(string_functions::remove_back_spaces((*line).substr(0, arg_start-1)));
            result[1] = string_functions::remove_front_spaces(string_functions::remove_back_spaces((*line).substr(arg_start, arg_len)));
        }
        else {result[0] = string_functions::remove_front_spaces(string_functions::remove_back_spaces((*line)));}
        return result;
    }
    std::vector<std::string> split_arguments(std::string arg_line)
    {
        std::vector<std::string> result;
        int len = arg_line.size();
        int p_count = 0;
        int b_count = 0;
        bool in_string = false;

        int arg_begin = 0;
        int arg_len = 0;

        for(int i=0; i<len; i++)
        {
            char c = arg_line[i];
            switch (c)
            {
            case '(':
                if(!in_string)p_count++;
                break;
            case ')':
                if(!in_string) p_count--;
                break;
            case '"':
                if(arg_line[i-1] != '\\') in_string = !in_string;
                break;
            case '[':
                if(!in_string) b_count++;
                break;
            case ']':
                if(!in_string) b_count--;
                break;
            }

            if(c == ',')
            {
                if(p_count==0&&b_count==0&&!in_string)
                {
                    if(arg_len > 0)result.push_back(string_functions::stripe_spaces(arg_line.substr(arg_begin, arg_len)));
                    arg_begin = i + 1;
                    arg_len = 0;
                }
                else arg_len++;
            }
            arg_len++;
        }
        if(arg_len > 0)result.push_back(string_functions::stripe_spaces(arg_line.substr(arg_begin, arg_len+1)));
        
        return result;
    }
    std::vector<std::string> parse_math_expression(std::string* expression)
    {
        std::vector<std::string> result;

        long p_count = 0;
        bool in_str = false;
        long len = (*expression).size();
        bool in_operator = false;

        int element_begin = 0;
        int element_size = 0;

        for(int i = 0; i < len; i++)
        {
            char c = (*expression)[i];
            if(c == '(') p_count++;
            else if(c == ')') p_count--;
            else if(c == '"' && (*expression)[i-1] != '\\') in_str = !in_str;

            if(in_operator)
            {
                if(is_op(c)){element_size++;}
                else
                {
                    in_operator = false;
                    i--;
                    if(element_size > 0)result.push_back(string_functions::stripe_spaces((*expression).substr(element_begin, element_size)));
                    element_begin = i + 1;
                    element_size = 0;
                }
            }
            else
            {
                if(is_op(c) && p_count == 0 && !in_str)
                {
                    in_operator = true;
                    i--;
                    if(element_size > 0)result.push_back(string_functions::stripe_spaces((*expression).substr(element_begin, element_size)));
                    element_begin = i + 1;
                    element_size = 0;
                }
                else{element_size++;}
            }
        }
        if(element_size > 0)result.push_back(string_functions::stripe_spaces((*expression).substr(element_begin, element_size)));

        return result;
    }
    std::vector<std::string> split_variable(std::string* initializer)
    {
        std::vector<std::string> result;
        std::vector<std::string> parsed_init = get_instruction_name_and_args(initializer);
        std::vector<std::string> init_members = string_functions::split(&parsed_init[0], '.');
        int init_member_count = init_members.size();
        if(init_member_count == 1)
        {
            result.push_back(init_members[0]);
            result.push_back("self");
            result.push_back("none");
        }
        else if(init_member_count == 2)
        {
            result.push_back(init_members[0]);
            result.push_back(init_members[1]);
            result.push_back(parsed_init[1]);
        }
        else {message::error(std::string("bad variable init: " + *initializer).c_str());}

        return result;
    }
}
#endif