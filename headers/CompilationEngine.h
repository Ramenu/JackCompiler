#ifndef COMPILATIONENGINE_H
#define COMPILATIONDEFINE_H

#include <fstream>
#include <ostream>
#include "JackTokenizer.h"

class CompilationEngine
{
    public:
        CompilationEngine(const char* inputFile,  const char* outputFile);
    private: 
        std::string tokenType;
        JackTokenizer tokenizer;
        std::ofstream out {};
        bool tokenIsStatement;
        std::ofstream openOutputFile(const char* filePath);
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
        void compileTerm();
        void compileExpressionList();
        void compileNext();
        void parseLineAndOutput();
        void updateTokenType();
        void output(std::string tokenType);
};

#endif