#include "FunctionWriterAdaptor.h"

namespace lua
{
    FunctionWriterAdaptor::FunctionWriterAdaptor(Function *f)
        : fun_(f)
    {
    }
} // namespace lua
