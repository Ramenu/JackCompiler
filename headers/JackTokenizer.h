#ifndef JACKTOKENIZER_H
#define JACKTOKENIZER_H

#include <fstream>

class JackTokenizer
{
    public:
        bool debug;
        std::string token;
        static std::string classDeclarations[15];
        void advance();
        std::string getTokenType();
        bool hasMoreTokens();
        std::ifstream in;
        unsigned int count;
        unsigned int lineNum;
        static unsigned int numberOfClasses;
        bool endOfFileReached;
        std::string line;
        std::string lineLeftToParse;
    private:
        std::string removeWhiteSpace(std::string str);
        std::string removeLineComments(std::string str);
        bool checkConflictingNames();
        unsigned int mostSimilarStr(std::string lineToCompare, std::string exactString);
};

#endif