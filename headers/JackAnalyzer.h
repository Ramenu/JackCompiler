#ifndef JACKANALYZER_H
#define JACKANALYZER_H

#include <fstream>
#include <ostream>
#include "CompilationEngine.h"

class JackAnalyzer
{
    public:
        void startUp(const char* path, bool doDebug);
    private:
        void readDirectory(const char* directoryPath, bool doDebug);
        void writeToFile(const char* filePath);
        void addClassDeclarationsToArr(std::string pathName);
};

#endif