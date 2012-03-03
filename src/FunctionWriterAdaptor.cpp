#include "FunctionWriterAdaptor.h"

namespace lua
{
    FunctionWriterAdaptor::FunctionWriterAdaptor(Function *f)
        : fun_(f)
    {
    }

    Instruction * FunctionWriterAdaptor::NewInstruction()
    {
        return 0;
    }
} // namespace lua
