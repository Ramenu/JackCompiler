#include "FileReader.h"

std::ifstream openFile(const char* filePath)
{
    std::ifstream fileOpener {filePath};

    if (fileOpener.fail())
        printf("ERROR: Failed to open file at %s. Path or file is not found.\n", filePath);
    return fileOpener;
}