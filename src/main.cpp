#include "JackCompiler.h"

int main(int argc, char* argv[])
{
    JackCompiler jack;
    if (argc == 1)
        printf("ERROR: No file or path given as input.\n");
    else if (argc >= 2)
    {
        if (argc == 3)
        {
            std::string thirdArg = argv[2];
            if (thirdArg == "-d")
                jack.startUp(argv[1], true);
            else
                jack.startUp(argv[1], false);
        }
        else
            jack.startUp(argv[1], false);
    }
}