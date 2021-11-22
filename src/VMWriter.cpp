#include "VMWriter.h"

//VMWriter constructor, with the name of the output file as an argument
VMWriter::VMWriter(std::string outputFileName)
{
    outVM.open(outputFileName.append(".vm")); //Prepares an output file for writing
}

//Writes a VM push command
void VMWriter::writePush(const char* segment, unsigned int index)
{
    outVM << "push " << segment << " " << index << "\n";
}

//Writes a VM pop command
void VMWriter::writePop(const char* segment, unsigned int index)
{
    outVM << "pop " << segment << " " << index << "\n";
}

//Writes a VM arithmetic command (ADD, SUB, NEG, EQ, GT, LT, AND, OR, NOT)
void VMWriter::writeArithmetic(const char* command)
{
    outVM << command << "\n";
}

//Writes a VM label command
void VMWriter::writeLabel(const char* label)
{
    outVM << "label " << label << "\n";
}

//Writes a VM goto command
void VMWriter::writeGoto(const char* label)
{
    outVM << "goto " << label << "\n";
}

//Writes a VM if-goto command
void VMWriter::writeIf(const char* label)
{
    outVM << "if-goto " << label << "\n";
}

//Writes a VM call command
void VMWriter::writeCall(const char* name, unsigned int nArgs)
{
    outVM << "call " << name << " " << nArgs << "\n";
}

//Writes a VM function command
void VMWriter::writeFunction(const char* name, unsigned int nLocals)
{
    outVM << "function " << name << " " << nLocals << "\n";
}

//Writes a VM return command
void VMWriter::writeReturn()
{
    outVM << "return\n";
}

//Closes the output file
void VMWriter::close()
{
    outVM.close();
}
