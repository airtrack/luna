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
        gen0_.max_count_ = kGen0InitMaxCount;
        gen1_.max_count_ = kGen1InitMaxCount;
    }

    void GC::SetRootTraveller(const RootTravelType &minor, const RootTravelType &major)
    {
        minor_traveller_ = minor;
        major_traveller_ = major;
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

    void GC::SetBarrier(GCObject *obj)
    {
        barriered_.push_back(obj);
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

    void GC::MinorGC()
    {
        minor_traveller_();

        for (auto obj : barriered_)
            obj;
    }

    void GC::MajorGC()
    {
        major_traveller_();
    }
} // namespace luna
