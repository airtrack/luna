#include "GC.h"

namespace luna
{
    GCObject::~GCObject()
    {
    }

    GC::GC()
        : gen0_(nullptr), gen1_(nullptr), gen2_(nullptr)
    {
    }
} // namespace luna
