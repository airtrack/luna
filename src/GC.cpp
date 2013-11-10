#include "GC.h"
#include "Table.h"
#include "Function.h"
#include "String.h"
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

    class MinorMarkVisitor : public GCObjectVisitor
    {
    public:
        virtual bool Visit(Table *t) { return VisitObj(t); }
        virtual bool Visit(Function *f) { return VisitObj(f); }
        virtual bool Visit(Closure *c) { return VisitObj(c); }
        virtual bool Visit(String *s) { return VisitObj(s); }

    private:
        bool VisitObj(GCObject *obj)
        {
            if (obj->generation_ == GCGen0 && obj->gc_ == GCFlag_White)
            {
                obj->gc_ = GCFlag_Black;
                return true;
            }
            return false;
        }
    };

    class BarrieredMarkVisitor : public GCObjectVisitor
    {
    public:
        virtual bool Visit(Table *t) { return VisitObj(t); }
        virtual bool Visit(Function *f) { return VisitObj(f); }
        virtual bool Visit(Closure *c) { return VisitObj(c); }
        virtual bool Visit(String *s) { return VisitObj(s); }

    private:
        bool VisitObj(GCObject *obj)
        {
            // Visit member GC objects of obj when it is barriered object
            if (obj->generation_ != GCGen0 && obj->gc_ == GCFlag_Black)
            {
                obj->gc_ = GCFlag_White;
                return true;
            }

            // Visit GCGen0 generation object
            if (obj->generation_ == GCGen0 && obj->gc_ == GCFlag_White)
            {
                obj->gc_ = GCFlag_Black;
                return true;
            }
            return false;
        }
    };

    class MajorMarkVisitor : public GCObjectVisitor
    {
    public:
        virtual bool Visit(Table *t) { return VisitObj(t); }
        virtual bool Visit(Function *f) { return VisitObj(f); }
        virtual bool Visit(Closure *c) { return VisitObj(c); }
        virtual bool Visit(String *s) { return VisitObj(s); }

    private:
        bool VisitObj(GCObject *obj)
        {
            if (obj->gc_ == GCFlag_White)
            {
                obj->gc_ = GCFlag_Black;
                return true;
            }
            else
            {
                return false;
            }
        }
    };

    GC::GC()
    {
        gen0_.threshold_count_ = kGen0InitThresholdCount;
        gen1_.threshold_count_ = kGen1InitThresholdCount;
    }

    GC::~GC()
    {
        DestroyGeneration(gen0_);
        DestroyGeneration(gen1_);
        DestroyGeneration(gen2_);
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

    String * GC::NewString(GCGeneration gen)
    {
        auto s = new String;
        SetObjectGen(s, gen);
        return s;
    }

    void GC::SetBarrier(GCObject *obj)
    {
        assert(obj->generation_ != GCGen0);
        barriered_.push_back(obj);
    }

    void GC::CheckGC()
    {
        if (gen0_.count_ >= gen0_.threshold_count_)
        {
            if (gen1_.count_ >= gen1_.threshold_count_)
                MajorGC();
            else
                MinorGC();
        }
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
        unsigned int old_gen1_count = gen1_.count_;

        MinorGCMark();
        MinorGCSweep();

        barriered_.clear();

        // Caculate objects count from gen0_ to gen1_, which is how
        // many alived objects in gen0_ after mark-sweep, and adjust
        // gen0_'s threshold count by the alived_gen0_count
        unsigned int alived_gen0_count = gen1_.count_ - old_gen1_count;
        AdjustThreshold(alived_gen0_count, gen0_, kGen0InitThresholdCount);
    }

    void GC::MajorGC()
    {
        MajorGCMark();
        MajorGCSweep();

        barriered_.clear();

        // Adjust GCGen1 threshold count
        AdjustThreshold(gen1_.count_, gen1_, kGen1InitThresholdCount);
    }

    void GC::MinorGCMark()
    {
        assert(minor_traveller_);

        // Visit all minor GC root objects
        MinorMarkVisitor marker;
        minor_traveller_(&marker);

        // Visit all barriered GC objects
        BarrieredMarkVisitor barriered_maker;
        for (auto obj : barriered_)
        {
            // All barriered objects must be GCGen1 or GCGen2.
            assert(obj->generation_ != GCGen0);

            // Mark barriered objects, and visitor can visit
            // member GC objects of barriered objects.
            obj->gc_ = GCFlag_Black;
            obj->Accept(&barriered_maker);
        }
    }

    void GC::MinorGCSweep()
    {
        // Sweep GCGen0
        while (gen0_.gen_)
        {
            GCObject *obj = gen0_.gen_;
            gen0_.gen_ = gen0_.gen_->next_;

            // Move object to GCGen1 generation when object is black
            if (obj->gc_ == GCFlag_Black)
            {
                obj->gc_ = GCFlag_White;
                obj->generation_ = GCGen1;
                obj->next_ = gen1_.gen_;
                gen1_.gen_ = obj;
                gen1_.count_++;
            }
            else
            {
                delete obj;
            }
        }

        gen0_.count_ = 0;
    }

    void GC::MajorGCMark()
    {
        assert(major_traveller_);

        // Visit all major GC root objects
        MajorMarkVisitor marker;
        major_traveller_(&marker);
    }

    void GC::MajorGCSweep()
    {
        // Sweep all generations
        SweepGeneration(gen2_);
        SweepGeneration(gen1_);
        SweepGeneration(gen0_);

        // Move all GCGen0 objects to GCGen1
        while (gen0_.gen_)
        {
            GCObject *obj = gen0_.gen_;
            gen0_.gen_ = gen0_.gen_->next_;

            obj->generation_ = GCGen1;
            obj->next_ = gen1_.gen_;
            gen1_.gen_ = obj;
        }

        gen1_.count_ += gen0_.count_;
        gen0_.count_ = 0;
    }

    void GC::SweepGeneration(GenInfo &gen)
    {
        GCObject *alived = nullptr;

        while (gen.gen_)
        {
            GCObject *obj = gen.gen_;
            gen.gen_ = obj->next_;

            if (obj->gc_ == GCFlag_Black)
            {
                obj->gc_ = GCFlag_White;
                obj->next_ = alived;
                alived = obj;
            }
            else
            {
                delete obj;
                gen.count_--;
            }
        }

        gen.gen_ = alived;
    }

    void GC::AdjustThreshold(unsigned int alived_count, GenInfo &gen,
                             unsigned int min_threshold)
    {
        while (gen.threshold_count_ < 2 * alived_count)
            gen.threshold_count_ *= 2;
        while (gen.threshold_count_ >= 4 * alived_count)
            gen.threshold_count_ /= 2;

        if (gen.threshold_count_ < min_threshold)
            gen.threshold_count_ = min_threshold;
    }

    void GC::DestroyGeneration(GenInfo &gen)
    {
        while (gen.gen_)
        {
            GCObject *obj = gen.gen_;
            gen.gen_ = gen.gen_->next_;
            delete obj;
        }
        gen.count_ = 0;
    }
} // namespace luna
