#include <filesystem>
#include "JackCompiler.h"
#define RESET   "\033[0m"
#define RED     "\033[31m" 

namespace fs = std::filesystem;

//Is called from main, and checks to see if the path given is a directory or not
void JackCompiler::startUp(const char* path, bool doDebug)
{
    if (fs::is_directory(path))
        readDirectory(path, doDebug);
    else if (fs::is_regular_file(path))
    {
        if (fs::path(path).filename().extension().string() == ".jack") //Confirm that a jack file is being compiled
        {
            addClassDeclarationsToArr(path);
            CompilationEngine engine(path, fs::path(path).filename().replace_extension(".vm").string().c_str(), fs::path(path).filename().string().c_str());
        }
        else
            printf(RED"ERROR: " RESET "File \"%s\" is not a jack file\n", fs::path(path).filename().string().c_str());
    }
    else
        printf(RED"ERROR: " RESET "Failed to open file/directory at \"%s\". Path not found\n", path);
}

//Adds the class definitions to the classDec array, useful for handling tokens
void JackCompiler::addClassDeclarationsToArr(std::string pathName)
{
    //Standard OS libraries (must be put in before the other classes)
    JackTokenizer::classDeclarations[0] = "Math";
    JackTokenizer::classDeclarations[1] = "Screen";
    JackTokenizer::classDeclarations[2] = "Output";
    JackTokenizer::classDeclarations[3] = "Keyboard";
    JackTokenizer::classDeclarations[4] = "Memory";
    JackTokenizer::classDeclarations[5] = "String";
    JackTokenizer::classDeclarations[6] = "Array";
    JackTokenizer::classDeclarations[7] = "Sys";
    JackTokenizer::numberOfClasses = 8;

    if (fs::is_regular_file(pathName)) //If only one file is being compiled
    {
        JackTokenizer::classDeclarations[8] = fs::path(pathName).filename().replace_extension().string();
        JackTokenizer::numberOfClasses++;
        if (fs::path(pathName).filename().replace_extension().string() != "Main")
            printf(RED "ERROR: " RESET "File \"%s\" must be named Main!\n", pathName.c_str());
    }
    else if (fs::is_directory(pathName))
    {
        for (const auto& entry: fs::directory_iterator(pathName))
        {
            if (entry.path().filename().extension() == ".jack")
            {
                JackTokenizer::classDeclarations[JackTokenizer::numberOfClasses] = entry.path().filename().replace_extension().string();
                JackTokenizer::numberOfClasses++;
            }
        }
        //TODO: Add a feature that checks if one of them is at least Main.jack
    }
}

/* Method that iterates through the contents of a directory, finding the files with the .jack extension and sending them to the
   JackTokenizer to parse them */
void JackCompiler::readDirectory(const char* directoryPath, bool doDebug)
{
    bool hasAnyJackFiles {};
    addClassDeclarationsToArr(directoryPath);
    for (const auto& entry: fs::directory_iterator(directoryPath)) //Iterate through directory
    {
        if (entry.path().extension() == ".jack")
        {
            hasAnyJackFiles = true;
            std::string pathToInputFile {entry.path().string()};
            CompilationEngine engine(pathToInputFile.c_str(), entry.path().filename().replace_extension(".vm").string().c_str(), entry.path().filename().string().c_str());
        }
    }
    if (!hasAnyJackFiles) //If no jack files were found in the directory output an error
        printf(RED"ERROR: " RESET "No jack files found in directory \"%s\"\n", directoryPath);
}

