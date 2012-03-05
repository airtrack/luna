#include "FunctionWriterAdaptor.h"
#include "types/Function.h"

namespace lua
{
    FunctionWriterAdaptor::FunctionWriterAdaptor(Function *f)
        : fun_(f)
    {
    }

    Instruction * FunctionWriterAdaptor::NewInstruction()
    {
        return fun_->NewInstruction();
    }
} // namespace lua
