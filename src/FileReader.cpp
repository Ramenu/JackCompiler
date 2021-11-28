#include "FileReader.h"
#include "Line.h"
#define RESET   "\033[0m"
#define RED     "\033[31m" 

/* IMPLEMENTATION TIPS:

1) When the ifstream encounters a subroutine declaration, save that line as a string and put it into the map as key
2) To find out when the subroutine finishes, when it encounters another subroutine declaration then that means the subroutine has ended
3) Until the subroutine ends for each local variable found, increment and when the function finishes, insert it into a map as a key
4) Then, compilationengine can put the subroutine declaration line, put it into the map, and retrieve the value from it
*/

//Class constructor
FileReader::FileReader(const char* filePath): in {openFile(filePath)}, numFields {} {}


//Opens the file
std::ifstream FileReader::openFile(const char* filePath)
{
    std::ifstream stream {filePath};
    if (stream.fail()) //If it fails to open the file
        printf(RED "ERROR: " RESET "Failed to open file at \"%s\". File is corrupted or missing.\n", filePath);
    return stream;
}

/* Reads the entire file and puts the total number of local variables in each function into the hashmap,
   also provides the total number of fields in each file */
void FileReader::firstPass()
{
    size_t noLocals {};
    const char* subroutineDecKeywords[3] {"constructor", "function", "method"}; //Array with subroutine declaration keywords

    while (in.peek() != EOF) //While the end of file is not reached
    {
        LOOP:
        if (!isComment() && !currentLine.empty()) //If the line is not a comment and not empty..
        {
            if (!isMultiLineComment()) //If the line does not start a multi-line comment..
            {
                currentLine = removeWhiteSpace(currentLine);
                currentLine = removeLineComments(currentLine);
                if (currentLine.find("field") != std::string::npos)
                {
                    numFields++;
                    for (size_t i {}; i < currentLine.length(); i++)
                        if (currentLine[i] == ',')
                            numFields++;
                }
                else
                {
                    for (unsigned short i {}; i < 3; i++)
                    {
                        if (currentLine.find(subroutineDecKeywords[i]) != std::string::npos) //If a subroutine declaration is found
                        {
                            bool isConstructor {subroutineDecKeywords[i] == "constructor"};
                            std::string subroutineDecLine {currentLine};
                            while (in.peek() != EOF)
                            {
                                std::getline(in, currentLine);
                                if (currentLine.find("var") != std::string::npos)
                                {
                                    localTable[subroutineDecLine] = ++noLocals; //For each variable declaration found.. increment

                                    /* 
                                        Because variable declarations in Jack can be seperated by a comma, for each comma there
                                        is another variable (if written according to proper Jack syntax) so increment the number
                                        of local variables for each ',' found 
                                    */
                                    if (currentLine.find(",") != std::string::npos)
                                        for (size_t j {}; j < currentLine.length(); j++)
                                            if (currentLine[j] == ',')
                                                localTable[subroutineDecLine] = ++noLocals;
                                } 

                                // If it finds another sub routine declaration, then that means that the current subroutine has ended
                                else
                                {
                                    for (unsigned short j {}; j < 3; j++)
                                    {
                                        if (currentLine.find(subroutineDecKeywords[j]) != std::string::npos)
                                        {
                                            noLocals = 0; //Reset the number of locals back to 0
                                            goto LOOP;
                                        }
                                    }
                                }
                            }
                        }
                    } 
                }
            }
            else
                skipLines(); //Skips lines until the multi-line comment ends
        }
        std::getline(in, currentLine); //Get the next line of the file
    }
}

//Returns the number of local variables the subroutine has
size_t FileReader::getLocals(std::string line)
{
    return localTable[line];
}

//Checks if the beginning of the line is a comment
bool FileReader::isComment()
{
    if (currentLine[0] == '/' && currentLine[1] == '/')
        return true;
    return false;
}

//Checks if the current line starts a multi-line comment
bool FileReader::isMultiLineComment()
{
    if (currentLine[0] == '/' && currentLine[1] == '*')
        return true;
    return false;
}

//Skips lines until a multi-line comment ends (should be called iff isMultiLineComment returns true)
void FileReader::skipLines()
{
    // While the file is not empty..
    while (in.peek() != EOF)
    {
        if (currentLine.find("*/") != std::string::npos) // When the multi-line comment ends..
            break; //End loop
        std::getline(in, currentLine);
    }
    currentLine = currentLine.substr(currentLine.find("*/"), currentLine.back()); //Get the position of where the comment ends
    currentLine = removeWhiteSpace(currentLine);
}

//Checks how many instances there are of a symbol (only until an expression ends)
size_t FileReader::instancesOf(std::string line, char symbol)
{
    size_t instancesOfCount {};
    bool ends {};
    char endExpSymbols[8] {',', ';', '{', '}', '.', '=', ']'}; //Symbols that can end an expression (not including parenthesis) 

    for (auto&i: line)
    {
        if (i == symbol)
            instancesOfCount++; //Increment for each time a symbol is found
        for (unsigned short j {}; j < 8; j++) 
        {
            if (i == endExpSymbols[j]) //Check if the current character ends the expression
            {
                ends = true;
                break;
            }
        }
        if (ends)
            break;
    }
    return instancesOfCount;
}

//Checks if the next symbol starts an array (should be called only if the current token is an identifier!)
bool FileReader::startsArray(std::string line)
{
    //Includes all the symbols in the Jack language with the exception of '['
    char symbolArray[18] {'(', ')', '{', '}', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|', '<', '>', '=', '~'};
    for (auto&i: line)
    {
        if (i == '[') //If right after the identifier a '[' appears, then an array begins so return true
            return true;
        for (unsigned short j {}; j < 18; j++)
            if (i == symbolArray[j]) //If it is any one of the symbols other than '[' then an array does not appear
                return false;
    }
    return false;
}
