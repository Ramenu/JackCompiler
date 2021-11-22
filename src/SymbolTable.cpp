#include "SymbolTable.h"

//Clears keys and values of the table, is called whenever a new subroutine begins
void SymbolTable::startSubroutine()
{
    //Reinitialize the number of argument and local variables back to 0 since a new subroutine has begun
    noOfArgVars = 0;
    noOfLclVars = 0;
    subroutineTable.clear();
}

//Defines a new identifier, STATIC/FIELD identifiers have class scope while ARG/VAR identifiers have subroutine scope
void SymbolTable::define(std::string name, std::string type, std::string kind)
{
    Identifier identifier;
    identifier.name = name;
    identifier.type = type;
    identifier.kind = kind;
    identifier.index = varCount(kind);

    if (kind == "field" || kind == "static")
        classTable[name] = identifier;
    else
        subroutineTable[name] = identifier;

    switch (kind[0]) //Increments the num of variables found with the same kind in the scope
    {
        case 's': noOfStaticVars++; break; //STATIC
        case 'f': noOfFieldVars++; break; //FIELD
        case 'a': noOfArgVars++; break; //ARGUMENTS
        case 'l': noOfLclVars++; break; //LOCAL
    }
}

//Returns the number of variables of the given kind defined in the current scope
unsigned int SymbolTable::varCount(std::string kind)
{
    if (kind == "static")
        return noOfStaticVars;
    else if (kind == "field")
        return noOfFieldVars;
    else if (kind == "argument")
        return noOfArgVars;
    else if (kind == "local")
        return noOfLclVars;
    else
        return 0; //It should not reach this condition, but this is just to make sure the method returns something 
}

std::string SymbolTable::getValFromTable(std::string name, std::string valueReturned)
{
    if (subroutineTable.find(name) != subroutineTable.end()) //Check if the identifier exists in the table before iterating through it
    {
        for (auto&i: subroutineTable)
        {
            if (i.first == name) 
            {
                //Check which attribute is being retrieved
                if (valueReturned == "KIND")
                    return i.second.kind.c_str();
                else if (valueReturned == "TYPE")
                    return i.second.type.c_str();
                else
                    return std::to_string(i.second.index);
            }
        }
    }
    else //If not found in the subroutine scope, check the class scope
    {
        if (classTable.find(name) != classTable.end()) //Repeat...
        {
            for (auto&i: classTable)
            {
                if (i.first == name)
                {
                    if (valueReturned == "KIND")
                        return i.second.kind.c_str();
                    else if (valueReturned == "TYPE")
                        return i.second.type.c_str();
                    else
                        return std::to_string(i.second.index);
                }
            }
        }
        else //If no identifier is located in either table..
            if (valueReturned != "INDEX")
                return "NONE";
            else
                return "0"; //Index must have a number returned
    }
    if (valueReturned != "INDEX")   
        return "NONE";
    else
        return "0";
}

//Returns the kind of the identifier in the current scope
std::string SymbolTable::kindOf(std::string name)
{
    return getValFromTable(name, "KIND");
}

//Returns the type of the identifier in the current scope
std::string SymbolTable::typeOf(std::string name)
{
    return getValFromTable(name, "TYPE");
}

//Returns the index of the identifier
unsigned int SymbolTable::indexOf(std::string name)
{
    return std::stoi(getValFromTable(name, "INDEX"));
}

