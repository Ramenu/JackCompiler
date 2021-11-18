#include <filesystem>
#include "JackAnalyzer.h"

namespace fs = std::filesystem;



//Is called from main, and checks to see if the path given is a directory or not
void JackAnalyzer::startUp(const char* path, bool doDebug)
{
    if (fs::is_directory(path))
        readDirectory(path, doDebug);
    else
    {
        addClassDeclarationsToArr(path);
        CompilationEngine engine(path, fs::path(path).filename().replace_extension(".xml").string().c_str(), fs::path(path).filename().string().c_str());
    }
}

//Adds the class definitions to the classDec array, useful for handling tokens
void JackAnalyzer::addClassDeclarationsToArr(std::string pathName)
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

    if (fs::is_regular_file(pathName)) //If only one file is being compiled
        pathName = fs::path(pathName).parent_path().string(); //Make sure to store the other file names in case it calls other classes
    JackTokenizer::numberOfClasses = 8;
    for (const auto& entry: fs::directory_iterator(pathName))
        if (entry.path().filename().extension() == ".jack")
        {
            JackTokenizer::classDeclarations[JackTokenizer::numberOfClasses] = entry.path().filename().replace_extension().string();
            JackTokenizer::numberOfClasses++;
        }
}

/* Method that iterates through the contents of a directory, finding the files with the .jack extension and sending them to the
   JackTokenizer to parse them */
void JackAnalyzer::readDirectory(const char* directoryPath, bool doDebug)
{
    addClassDeclarationsToArr(directoryPath);
    for (const auto& entry: fs::directory_iterator(directoryPath)) //Iterate through directory
    {
        if (entry.path().extension() == ".jack")
        {
            std::string pathToInputFile {entry.path().string()};
            CompilationEngine engine(pathToInputFile.c_str(), entry.path().filename().replace_extension(".xml").string().c_str(), entry.path().filename().string().c_str());
        }
    }
}

