#ifndef COMPILATIONENGINE_H
#define COMPILATIONDEFINE_H

#include <fstream>
#include <ostream>
#include "JackTokenizer.h"
#include "SymbolTable.h"

class CompilationEngine
{
    public:
        CompilationEngine(const char* inputFile,  const char* outputFile, const char* inputFileName);
    private: 
        const char* inputFileName;
        bool errorHappened;
        unsigned int errorsFound;
        bool inSubroutine;
        std::string tokenType; 
        JackTokenizer tokenizer;
        SymbolTable table;
        std::ofstream out {};
        bool tokenIsStatement;
        bool inExpressionList;
        bool inDo;
        std::ofstream openOutputFile(const char* filePath);
        unsigned int noStatementMatches();
        bool subroutineEnds();
        bool subroutineStarts();
        void compileFile();
        void getTokenAndOutput();
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
        unsigned int expressionCallCount;
        bool compileAnotherExpressionOrTerm(bool isSecondCall);
        void compileTerm();
        void compileExpressionList();
        void parseLineAndOutput();
        void updateToken();
        void advanceIfNoTokens();
        bool syntaxAnalyzer(const char* expectedOutput);
        void reportError(const char* tokenOrMessage, bool isDefault);
        void parseUntilSymbol(char symbol);
        void output(std::string tokenType);
};

#endif