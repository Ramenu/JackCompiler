#ifndef JACKCOMPILER_H
#define JACKCOMPILER_H

#include "CompilationEngine.h"

class JackCompiler
{
    public:
        void startUp(const char* path, bool doDebug);
    private:
        void readDirectory(const char* directoryPath, bool doDebug);
        void writeToFile(const char* filePath);
        void addClassDeclarationsToArr(std::string pathName);
};

#endif