#ifndef FUNCTION_WRITER_ADAPTOR_H
#define FUNCTION_WRITER_ADAPTOR_H

#include "CodeWriter.h"

namespace lua
{
    class Function;

    class FunctionWriterAdaptor : public CodeWriter
    {
    public:
        explicit FunctionWriterAdaptor(Function *f);

    private:
        Function *fun_;
    };
} // namespace lua

#endif // FUNCTION_WRITER_ADAPTOR_H
