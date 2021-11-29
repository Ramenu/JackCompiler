#include "JackCompiler.h"

int main(int argc, char* argv[])
{
    JackCompiler jack;
    if (argc == 1)
        printf("ERROR: No file or path given as input.\n");
    else if (argc == 2)
        jack.startUp(argv[1]);
    else
        printf("ERROR: Too many arguments given, please provide the path of the file/directory you want to compile only.\n");
}