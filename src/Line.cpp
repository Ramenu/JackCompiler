#include <algorithm>
#include "Line.h"

std::string removeLineComments(std::string str)
{
    return str.substr(0, str.find("//", 0));
}

std::string removeWhiteSpace(std::string str)
{
    str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
    return str;
}