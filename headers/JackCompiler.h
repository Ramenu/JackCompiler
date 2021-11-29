#ifndef JACKCOMPILER_H
#define JACKCOMPILER_H

#include "CompilationEngine.h"

class JackCompiler
{
    public:
        void startUp(const char* path);
    private:
        void readDirectory(const char* directoryPath);
        void writeToFile(const char* filePath);
        void addClassDeclarationsToArr(std::string pathName);
        bool isDefaultClass(const char* fileName);
};

#endif