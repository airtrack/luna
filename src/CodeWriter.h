#ifndef CODE_WRITER_H
#define CODE_WRITER_H

#include "Instruction.h"

namespace lua
{
    class State;

    class CodeWriter
    {
    public:
        virtual ~CodeWriter() {}
        virtual Instruction * NewInstruction() = 0;
    };
} // namespace lua

#endif // CODE_WRITER_H
