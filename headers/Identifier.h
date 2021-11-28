#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <string>

struct Identifier
{
    std::string name;
    std::string type;
    std::string kind;
    unsigned int index;
};

#endif