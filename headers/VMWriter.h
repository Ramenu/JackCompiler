#ifndef VMWRITER_H
#define VMWRITER_H

#include <fstream>
#include <ostream>

class VMWriter
{
    public:
        VMWriter(std::string outputFileName);
        void writePush(std::string segment, unsigned int index);;
        void writePop(std::string segment, unsigned int index);
        void writeArithmetic(char command);
        void writeLabel(std::string label);
        void writeGoto(std::string label);
        void writeIf(std::string label);
        void writeCall(std::string name, unsigned int nArgs);
        void writeFunction(std::string name, unsigned int nLocals);
        void writeReturn();
        void close();
        bool isNeg;
    private:
        std::ofstream outVM;
        std::ofstream openOutputFile(const char* filePath);
};

#endif