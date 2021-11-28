#ifndef COMPILATIONENGINE_H
#define COMPILATIONDEFINE_H

#include "JackTokenizer.h"
#include "SymbolTable.h"
#include "VMWriter.h"
#include "FileReader.h"

class CompilationEngine
{
    public:
        CompilationEngine(const char* inputFile,  const char* outputFile, const char* inputFileName);
    private: 
        bool isStandardClass(std::string token);
        std::string className;
        std::string subroutineDec;
        std::string subroutineName;
        std::string subroutineType;
        std::string arrayIdentifier;
        const char* inputFileName;
        bool errorHappened;
        bool equalToExpression;
        unsigned int errorsFound;
        bool inLet;
        bool inTerm;
        size_t noifsCompiled;
        size_t nowhilesCompiled;
        bool inSubroutine;
        std::string tokenType; 
        JackTokenizer tokenizer;
        SymbolTable table;
        VMWriter vm;
        FileReader reader;
        bool tokenIsStatement;
        bool inExpressionList;
        bool inDo;
        unsigned int noStatementMatches();
        bool subroutineEnds();
        bool subroutineStarts();
        void compileFile();
        void selectCompilationTask();
        void compileClass();
        void compileClassVarDec();
        void compileSubroutineDec();
        void compileParameterList();
        void compileSubroutineBody();
        void compileVarDec();
        void compileStatements();
        void compileLet();
        void compileIf();
        void compileWhile();
        void compileDo();
        void compileReturn();
        void compileExpression();
        void compileString();
        unsigned int expressionCallCount;
        std::string callTo;
        size_t nArgs;
        size_t noOfTerms;
        bool compileAnotherExpressionOrTerm(bool isSecondCall);
        void compileTerm();
        void compileExpressionList();
        void updateToken();
        void advanceIfNoTokens();
        bool syntaxAnalyzer(const char* expectedOutput);
        void reportError(const char* tokenOrMessage, bool isDefault);
        void parseUntilSymbol(char symbol, bool append);
};

#endif