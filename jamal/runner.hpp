#ifndef RUNNER_H
#define RUNNER_H

#include "jamal.hpp"

namespace jamal_runner
{
    jamal::stack run_expression(std::string* line, jamal::jamal_data* data, jamal::variable_map* variables);
    jamal::stack run_code_cluster(std::vector<std::string>& code, jamal::jamal_data& data, jamal::variable_map& variables, bool& return_point);

    enum expression_type
    {
        INSTRUCTION, STRING, INT, MATH,
    };
}

#endif