#include <algorithm>
#include "JackTokenizer.h"

std::string JackTokenizer::classDeclarations[10];

bool JackTokenizer::hasMoreTokens()
{
    if (lineLeftToParse.size() != 1) //If the line only has one token, then it does not have any more tokens remaining to parse
            return true;
    
    return false;
}

//Advances to the next line
void JackTokenizer::advance()
{
    if (in && in.peek() != EOF) //If the file still contains lines, continue to advance
    {
        std::getline(in, line);
        line = removeWhiteSpace(line); //Removes whitespace
        lineNum++;
        if (line[0] == '/' || line.empty()) //If the line is a comment or is empty, continue getting the next line
        {
            while (in && in.peek() != EOF)
            {
                lineNum++;
                if (lineNum == 53)
                {

                }
                std::getline(in, line);
                line = removeWhiteSpace(line); //Removes whitespace
                //If the line is not a comment, and not empty/contains spaces terminate the loop
                if (line[0] != '/' && !line.empty()) 
                    break;
                if (line.find("/*") != std::string::npos) //If it finds a multi-line comment
                {
                    while (in && in.peek() != EOF) //Keep getting the next line until the multi-line comment ends
                    {
                        if (line.find("*/") != std::string::npos)
                            break;
                        std::getline(in, line);
                        lineNum++;
                    }
                }
            }
        }
        count = 0; //Reset the substring counter to index 0
        line = removeLineComments(line); //Removes line comments
    }
    else
        endOfFileReached = true;
}


std::string JackTokenizer::getTokenType()
{
    std::string keyWordArray[21] //Array full of keywords in the Jack language
    {
     "class", "constructor", "function", "method", "field", "static", "var", "int", "char", "boolean",
     "this", "let", "if", "else", "while", "do", "return", "void", "true", "false", "null"
    };
    const char* keywordsCouldHaveClass[6] //Keywords that could have a CLASS name within them
    {
        "class", "constructor", "field", "var", "do", "return"
    };
    //Array full of symbols in the Jack language
    char symbolArray[19] {'(', ')', '{', '}', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|', '<', '>', '=', '~'};
    lineLeftToParse = line.substr(count, line.back());
    for (unsigned int i {}; i < 21; i++) //Check if the substring contains any keywords
    {
        if (lineLeftToParse.find(keyWordArray[i]) != std::string::npos) 
        {
            //Check if the number of chars in the keyword match up with the line (double authenication)
            if (mostSimilarStr(lineLeftToParse, keyWordArray[i]) == keyWordArray[i].length()) 
            {
                count = line.find(keyWordArray[i]) + keyWordArray[i].size();
                token = lineLeftToParse.substr(0, keyWordArray[i].size());
                return "KEYWORD";
            }
        }
    }

    for (unsigned int i {}; i < 19; i++) //Check if the current character is a symbol by looping through all the symbols
    {
       if (lineLeftToParse[0] == symbolArray[i])
       {
           count++; //Increment since the current character was passed
           token = lineLeftToParse[0];
           switch (symbolArray[i])
           {
               case '<': token = "&lt;"; break;
               case '>': token = "&gt;"; break;
               case '\"': token = "&quot;"; break;
               case '&': token = "&amp"; break;
           }
           return "SYMBOL";
       }
    }

    if (lineLeftToParse.find("\"") != std::string::npos)
    {
        count = lineLeftToParse.find("\"");
        return "STRING_CONST";
    }
    if (isdigit(lineLeftToParse[count])) //If the current character is a number, return INT_CONST
    {
        for (unsigned int i {count}; i < lineLeftToParse.size(); i++) //Loop through line
        {
            if (!isdigit(lineLeftToParse[i])) //When a character is not a digit, that character is where the INT_CONST ends
            {
                count = i--;
                token = lineLeftToParse.substr(0, i--);
                break;
            }
        }
        return "INT_CONST";
    }
    unsigned int mostCharsMatch {};
    bool anyClassNamesMatch {};
    if (isupper(lineLeftToParse[0])) //First character of class must be capital
    {
        for (unsigned int j {}; j < 6; j++) //Loop through keywords that can legally have class names within them
        {
            if (line.find(keywordsCouldHaveClass[j]) != std::string::npos) //Check if "Var" "field".. keywords that could have class names within them
            {
                for (unsigned int i {}; i < 3; i++) //Check if the identifier is a class/object
                {
                    if (lineLeftToParse.find(classDeclarations[i]) != std::string::npos) //Check if the line contains a class/obj token
                    {
                        anyClassNamesMatch = true;
                        if (checkConflictingNames()) //If there more than 1 similar class names
                        {
                            unsigned int charsMatched {mostSimilarStr(lineLeftToParse, classDeclarations[i])}; //Returns # of chars matched
                            if (charsMatched > mostCharsMatch) //If the characters matched is more than the total # of chars matched then it is that class/obj
                            {
                                count = classDeclarations[i].size() + line.find(classDeclarations[i]);
                                token = lineLeftToParse.substr(0, classDeclarations[i].size());
                            }
                            continue; //Continue the loop, as we do not want to return yet
                        }
                        else
                        {
                            count = classDeclarations[i].size() + line.find(classDeclarations[i]);
                            token = lineLeftToParse.substr(0, classDeclarations[i].size());
                        }
                        
                    }
                }
            }
        }
    }
    if (anyClassNamesMatch)
        return "IDENTIFIER";

    //If none of the above were returned, the token is considered as an identifier
    for (unsigned int i {count}; i < line.size(); i++) //Iterate through the string characters
    {
        for (unsigned int j {}; j < 19; j++) //Loop through all the symbols
        {
            if (line[i] == symbolArray[j]) //Check where the symbol is located, that is where the identifier ends
            {
                count = i;
                token = lineLeftToParse.substr(0, lineLeftToParse.find(line[i]));
                return "IDENTIFIER";
            }
        }
    }
    return "IDENTIFIER"; //If none of the above were returned, then it must be an identifier 
}

//Returns how many # of how many characters match with the exact string
unsigned int JackTokenizer::mostSimilarStr(std::string lineToCompare, std::string exactString)
{
    unsigned int charsMatching {}, j {};
    for (unsigned int i {}; i < exactString.length(); i++) //Loop through both strings
    {
        for (; j < lineToCompare.length(); j++)
        {
            if (lineToCompare[j] == exactString[i]) //If the characters match increment the charsMatch
            {
                charsMatching++;
            }
            break;
        }
        j++;
    }
    return charsMatching;
}

//Checks if any class names are similar, if there are returns true
bool JackTokenizer::checkConflictingNames()
{
    unsigned int matchingNames {};
    for (unsigned int i {}; i < 3; i++)
    {
        if (line.find(classDeclarations[i]) != std::string::npos) //If line finds any declared class names
            matchingNames++;
    }
    if (matchingNames > 1)
        return true;
    return false;
}


std::string JackTokenizer::removeLineComments(std::string str)
{
    return str.substr(0, str.find("/", 0));
}

std::string JackTokenizer::removeWhiteSpace(std::string str)
{
    str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
    return str;
}