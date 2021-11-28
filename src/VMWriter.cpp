#include "VMWriter.h"

//VMWriter constructor, with the name of the output file as an argument
VMWriter::VMWriter(std::string outputFileName): outVM {openOutputFile(outputFileName.c_str())}, isNeg {} {}

//Writes a VM push command
void VMWriter::writePush(std::string segment, unsigned int index)
{
    outVM << "push " << segment << " " << index << "\n";
    outVM.flush();
}

//Writes a VM pop command
void VMWriter::writePop(std::string segment, unsigned int index)
{
    outVM << "pop " << segment << " " << index << "\n";
    outVM.flush();
}

//Writes a VM arithmetic command (ADD, SUB, NEG, EQ, GT, LT, AND, OR, NOT)
void VMWriter::writeArithmetic(char command)
{
    std::string outputStr {};
    switch (command)
    {
        case '+': outputStr = "add"; break;
        case '-': outputStr = "sub"; break;
        case '*': outputStr = "call Math.multiply 2"; break;
        case '/': outputStr = "call Math.divide 2"; break;
        case '>': outputStr = "gt"; break;
        case '<': outputStr = "lt"; break;
        case '=': outputStr = "eq"; break;
        case '&': outputStr = "and"; break;
        case '|': outputStr = "or"; break;
        case '~': outputStr = "not"; break;
    }
    if (isNeg)
        outputStr = "neg";
    outVM << outputStr << "\n";
    outVM.flush();
}

//Writes a VM label command
void VMWriter::writeLabel(std::string label)
{
    outVM << "label " << label << "\n";
    outVM.flush();
}

//Writes a VM goto command
void VMWriter::writeGoto(std::string label)
{
    outVM << "goto " << label << "\n";
    outVM.flush();
}

//Writes a VM if-goto command
void VMWriter::writeIf(std::string label)
{
    outVM << "if-goto " << label << "\n";
    outVM.flush();
}

//Writes a VM call command
void VMWriter::writeCall(std::string name, unsigned int nArgs)
{
    outVM << "call " << name << " " << nArgs << "\n";
    outVM.flush();
}

//Writes a VM function command
void VMWriter::writeFunction(std::string name, unsigned int nLocals)
{
    outVM << "function " << name << " " << nLocals << "\n";
    outVM.flush();
}

//Writes a VM return command
void VMWriter::writeReturn()
{
    outVM << "return\n";
    outVM.flush();
}

//Closes the output file
void VMWriter::close()
{
    outVM.close();
}

//Opens the output file
std::ofstream VMWriter::openOutputFile(const char* filePath)
{
    std::ofstream of;
    of.open(filePath, std::ofstream::out | std::ofstream::trunc);
    if (of.fail())
        printf("ERROR: Failed to create/write to output file at %s.\n", filePath);
    return of;
}