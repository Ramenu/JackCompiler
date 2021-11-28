#ifndef FILEREADER_H
#define FILEREADER_H

#include <fstream>
#include <unordered_map>

class FileReader
{
    public:
        FileReader(const char* filePath);
        void firstPass();
        size_t getLocals(std::string line);
        size_t numFields;
        size_t instancesOf(std::string line, char symbol);
        bool startsArray(std::string line);
    private:
        std::ifstream in;
        std::string currentLine;
        std::ifstream openFile(const char* filePath);
        std::unordered_map<std::string, size_t> localTable; 
        bool isMultiLineComment();
        bool isComment();
        void skipLines();
};


#endif