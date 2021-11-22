#ifndef VMWRITER_H
#define VMWRITER_H

#include <fstream>
#include <ostream>

class VMWriter
{
    public:
        VMWriter(std::string outputFileName);
        void writePush(const char* segment, unsigned int index);;
        void writePop(const char* segment, unsigned int index);
        void writeArithmetic(const char* command);
        void writeLabel(const char* label);
        void writeGoto(const char* label);
        void writeIf(const char* label);
        void writeCall(const char* name, unsigned int nArgs);
        void writeFunction(const char* name, unsigned int nLocals);
        void writeReturn();
        void close();
    private:
        std::ofstream outVM;
};

#endif