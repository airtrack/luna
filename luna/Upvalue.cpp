#include "Upvalue.h"

namespace luna
{
    void Upvalue::Accept(GCObjectVisitor *v)
    {
        if (v->Visit(this))
        {
            value_.Accept(v);
        }
    }
} // namespace luna
