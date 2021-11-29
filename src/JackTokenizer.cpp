#include <algorithm>
#include "JackTokenizer.h"
#include "Line.h"

std::string JackTokenizer::classDeclarations[15];
unsigned int JackTokenizer::numberOfClasses;

JackTokenizer::JackTokenizer(const char* inputFile): in {openFile(inputFile)}, parsedDecStatement {} {}; //Constructor 

bool JackTokenizer::hasMoreTokens()
{
    if (lineLeftToParse.length() > 1) //If the line has one or more tokens
            return true;
    
    return false;
}

//Opens the file to prepare for reading
std::ifstream JackTokenizer::openFile(const char* filePath)
{
    std::ifstream fileOpener {filePath};

    if (fileOpener.fail())
        printf("ERROR: Failed to open file at %s. Path or file is not found.\n", filePath);
    return fileOpener;
}

//Gets the next line of the file
void JackTokenizer::toNextLine()
{
    std::getline(in, line);
}

//Advances to the next line
void JackTokenizer::advance()
{
    if (in && in.peek() != EOF) //If the file still contains lines, continue to advance
    { 
        std::getline(in, line);
        lineNoSpace = line;
        line = removeWhiteSpace(line); //Removes whitespace
        lineNum++;
        if (line[0] == '/' || line.empty()) //If the line is a comment or is empty, continue getting the next line
        {
            while (in && in.peek() != EOF)
            {
                lineNum++;
                std::getline(in, line);
                lineNoSpace = line;
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
                        lineNoSpace = line;
                        lineNum++;
                    }
                }
            }
        }
        count = 0; //Reset the substring counter to index 0
        line = removeLineComments(line); //Removes line comments
        lineNoSpace = removeLineComments(lineNoSpace);
    }
    else 
        in.close();
}

//Checks if the line substring is a keyword (must be called only if checking for a keyword)
bool JackTokenizer::authenticateToken(std::string arr[], unsigned short size)
{
    for (unsigned int i {}; i < size; i++) 
    {
        if (lineLeftToParse.find(arr[i]) != std::string::npos)
        { 
            //Check if the number of chars in the keyword match up with the line (double authenication)
            if (mostSimilarStr(lineLeftToParse, arr[i]) == arr[i].length()) 
            {
                count = line.find(arr[i], count) + arr[i].size();
                token = lineLeftToParse.substr(0, arr[i].size());
                return true;
            }
        }
    }
    return false;
}


std::string JackTokenizer::getTokenType()
{
    prevToken = token;
    std::string keyWordArray[8] //Array with keywords that do NOT start a declaration or statement
    {
        "int", "char", "boolean", "this", "void", "true", "false", "null"
    };
    std::string decOrStatementKeywords[13] //Array with keywords that start a declaration or a statement
    {
        "class", "constructor", "function", "method", "field", "static", "var", "let", "if", "else", "while", "do", "return"
    };
    const char* declarationKeywords[6] //Keywords that could have a CLASS name within them
    {
        "var", "field", "static", "class", "constructor", "function"
    };
    //Array full of symbols in the Jack language
    char symbolArray[19] {'(', ')', '{', '}', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|', '<', '>', '=', '~'};

    if (token == ";" || token == "{") //If the statement or declaration ends, the keyword can be parsed again
        parsedDecStatement = false;
    
    lineLeftToParse = line.substr(count, line.back());
    if (!parsedDecStatement) //If the declaration/statement has not been parsed yet
    {
        if (authenticateToken(decOrStatementKeywords, 13)) //Checks to see if the line starts a statement or declaration
        {
            parsedDecStatement = true; 
            return "KEYWORD";
        }
    }
    else
    {
        if (authenticateToken(keyWordArray, 8)) //Checks to see if the line is a keyword (not including keywords that start statements or declarations)
            return "KEYWORD";
    }
    for (unsigned int i {}; i < 19; i++) //Check if the current character is a symbol by looping through all the symbols
    {
       if (lineLeftToParse[0] == symbolArray[i])
       { 
           count++; //Increment since the current character was passed
           token = lineLeftToParse[0];
           return "SYMBOL";
       }
    }

    if (lineLeftToParse[0] == '\"') //Check if its a string
    {
        size_t index {lineNoSpace.find('\"', count)};
        size_t afterIndex {lineNoSpace.length() - index};
        count = line.find('\"', count + 1) + 1;
        token = lineLeftToParse.substr(1, lineLeftToParse.find('\"', 1) - 1);
        
        //Same as the token except includes spaces which is necessary
        tokenString = lineNoSpace.substr(index + 1, lineNoSpace.find('\"', index + 1) - index - 1); 
        return "STRING_CONST";
    }
    if (isdigit(lineLeftToParse[0])) //If the current character is a number, return INT_CONST
    {
        unsigned int x {};
        for (size_t i {count}; i < line.length(); i++) //Loop through line
        {
            if (!isdigit(line[i])) //When a character is not a digit, that character is where the INT_CONST ends
            {
                count = i--;
                token = lineLeftToParse.substr(0, x);
                break;
            }
            else
                x++;
        }
        return "INT_CONST";
    }
    unsigned int mostCharsMatch {};
    bool anyClassNamesMatch {};
    if (isupper(lineLeftToParse[0])) //First character of class must be capital
    {
        for (size_t j {}; j < 6; j++) //Loop through keywords that can legally have class names within them
        {
            if (line.find(declarationKeywords[j]) != std::string::npos) //Check if "Var" "field".. keywords that could have class names within them
            {
                for (size_t i {}; i < numberOfClasses; i++) //Check if the identifier is a class/object
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
        for (unsigned short j {}; j < 19; j++) //Loop through all the symbols
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
    for (size_t i {}; i < exactString.length(); i++) //Loop through both strings
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
    for (unsigned int i {}; i < numberOfClasses; i++)
    {
        if (line.find(classDeclarations[i]) != std::string::npos) //If line finds any declared class names
            matchingNames++;
    }
    if (matchingNames > 1)
        return true;
    return false;
}


