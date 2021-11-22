#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <iostream>
#include <unordered_map>
#include "Identifier.h"

class SymbolTable
{
    public:
        void startSubroutine();
        void define(std::string name, std::string type, std::string kind);
        unsigned int varCount(std::string kind);
        std::string kindOf(std::string name);
        std::string typeOf(std::string name);
        unsigned int indexOf(std::string name);
        unsigned int noOfStaticVars;
        unsigned int noOfFieldVars;
    private:
        std::string getValFromTable(std::string name, std::string valueReturned);
        std::unordered_map<std::string, Identifier> classTable;
        std::unordered_map<std::string, Identifier> subroutineTable;
        unsigned int noOfArgVars;
        unsigned int noOfLclVars;

};


#endif