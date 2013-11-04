#include "GC.h"
#include "Table.h"
#include "Function.h"
#include <assert.h>

namespace luna
{
    GCObject::GCObject()
        : next_(nullptr), generation_(GCGen0), gc_(0)
    {
    }

    GCObject::~GCObject()
    {
    }

    GC::GC()
    {
    }

    Table * GC::NewTable(GCGeneration gen)
    {
        auto t = new Table;
        SetObjectGen(t, gen);
        return t;
    }

    Function * GC::NewFunction(GCGeneration gen)
    {
        auto f = new Function;
        SetObjectGen(f, gen);
        return f;
    }

    Closure * GC::NewClosure(GCGeneration gen)
    {
        auto c = new Closure;
        SetObjectGen(c, gen);
        return c;
    }

    void GC::SetObjectGen(GCObject *obj, GCGeneration gen)
    {
        GenInfo *gen_info = nullptr;

        switch (gen)
        {
            case GCGen0:
                gen_info = &gen0_;
                break;
            case GCGen1:
                gen_info = &gen1_;
                break;
            case GCGen2:
                gen_info = &gen2_;
                break;
        }

        assert(gen_info);

        obj->generation_ = gen;
        obj->next_ = gen_info->gen_;
        gen_info->gen_ = obj;
        gen_info->count_++;
    }
} // namespace luna
