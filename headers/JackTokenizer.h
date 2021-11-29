#ifndef JACKTOKENIZER_H
#define JACKTOKENIZER_H

#include <fstream>

class JackTokenizer
{
    public:
        JackTokenizer(const char* inputFile);
        std::string token;
        std::string prevToken;
        static std::string classDeclarations[15];
        void advance();
        std::string getTokenType();
        bool hasMoreTokens();
        std::ifstream in;
        unsigned int count;
        unsigned int lineNum;
        static unsigned int numberOfClasses; 
        std::string line;
        std::string lineNoSpace;
        std::string tokenString;
        std::string lineLeftToParse;
        void toNextLine();
        std::ifstream openFile(const char* filePath);
        bool parsedDecStatement;
    private:
        bool authenticateToken(std::string arr[], unsigned short size);
        bool checkConflictingNames();
        unsigned int mostSimilarStr(std::string lineToCompare, std::string exactString);
};

#endif