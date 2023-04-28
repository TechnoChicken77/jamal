#ifndef JAMAL_H
#define JAMAL_H

#include <string>
#include <vector>
#include <map>

namespace jamal
{
    enum member_type
    {
        VARIABLE, SUBSECTION
    };
    enum operator_action_type
    {
        SECTION, INSTRUCTION
    };
    struct operator_data
    {
        operator_action_type action_type;
        std::string action_name;
    };
    
    typedef std::vector<u_char> stack;
    typedef std::map<int, std::string> member_map;

    class section
    {
    private:
        std::vector<std::string> code;
    public:
        std::vector<std::string> args;
        void insert_code(std::vector<std::string> c);
        void insert_args(std::vector<std::string> a);
        std::vector<std::string> get_code();
        stack run(stack* args);
        section(){}
        ~section(){}
    };
    typedef std::map<std::string, section> section_map;

    struct jamal_data;

    struct variable_data
    {
        long stack_address;
        std::string type;
    };
    struct type_member_data
    {
        long start_pos;
        long length;
    };
    typedef std::map<std::string, type_member_data> type_member_map;
    typedef std::map<std::string, std::string> subsection_map;
    typedef std::map<std::string, member_type> member_type_map;
    class type
    {
    public:
        type_member_map members;
        subsection_map subsections;
        operator_data type_operator;
        std::string type_destroyer = "none";
        std::string type_cloner = "none";
        member_type_map member_types;

        type();
    };
    type::type()
    {

    }
    const type empty_type;
    typedef std::map<std::string, type> type_map;
    typedef std::map<std::string, variable_data> variable_map;

    typedef stack (*instruction_t)(std::vector<stack> args, jamal_data& data, variable_map& variables);
    typedef std::map<std::string, instruction_t> instruction_map;

    struct jamal_data
    {
        std::vector<std::string> libraries;
        std::vector<stack> stacks;
        section_map sections;
        std::vector<std::string> entries;
        instruction_map instructions;
        type_map types;
    };

    long define_stack_in_vector(std::vector<stack>& vec);
    void empty_stack_in_vector(std::vector<stack>& vec, long stack_id);

    stack run_section(std::string name, std::vector<stack> args, jamal_data& data, jamal::variable_map& variables, bool destroy_self);
}

#endif