#include "JackCompiler.h"
#define RESET   "\033[0m"
#define RED     "\033[31m" 

int main(int argc, char* argv[])
{
    JackCompiler jack;
    if (argc == 1)
        printf(RED "ERROR: " RESET "No file or path given as input.\n");
    else if (argc == 2)
        jack.startUp(argv[1]);
    else
        printf(RED "ERROR: " RESET "Too many arguments given, please provide the path of the file/directory you want to compile only.\n");
}