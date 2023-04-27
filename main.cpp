#include <iostream>
using namespace std;

#include "jamal/jamal.cpp"
#include "jamal/file-interactions.cpp"
#include "jamal/message.cpp"
#include "jamal/code-parsing.cpp"
#include "jamal/runner.cpp"

int main(int argc, char** argv)
{
    if(argc==1)
    {
        cout << "Usage: " << argv[0] << " <code file path>\n";
        return -1;
    }
    else
    {
        string path = argv[1];
        string code = file::read_text(path);
        if(code == "")
        {
            message::error("nothing to do");
        }
        else
        {
            jamal_runner::run_code(code);
        }
    }
    
}