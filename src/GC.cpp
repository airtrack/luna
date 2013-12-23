#include "GC.h"
#include "Table.h"
#include "Function.h"
#include "Upvalue.h"
#include "String.h"
#include <assert.h>
#include <time.h>

namespace luna
{
    GCObject::GCObject()
        : next_(nullptr), generation_(GCGen0), gc_(0), gc_obj_type_(0)
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
        virtual bool Visit(Upvalue *u) { return VisitObj(u); }
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
        virtual bool Visit(Upvalue *u) { return VisitObj(u); }
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
        virtual bool Visit(Upvalue *u) { return VisitObj(u); }
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

#define GC_LOG(log)                             \
    do                                          \
    {                                           \
        if (log_stream_.is_open())              \
        {                                       \
            log_stream_ << log << std::endl;    \
        }                                       \
    } while (0)

    GC::GC(const GCObjectDeleter &obj_deleter, bool log)
        : obj_deleter_(obj_deleter)
    {
        gen0_.threshold_count_ = kGen0InitThresholdCount;
        gen1_.threshold_count_ = kGen1InitThresholdCount;

        if (log)
        {
            log_stream_.open("gc.log");
        }
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
        t->gc_obj_type_ = GCObjectType_Table;
        SetObjectGen(t, gen);
        return t;
    }

    Function * GC::NewFunction(GCGeneration gen)
    {
        auto f = new Function;
        f->gc_obj_type_ = GCObjectType_Function;
        SetObjectGen(f, gen);
        return f;
    }

    Closure * GC::NewClosure(GCGeneration gen)
    {
        auto c = new Closure;
        c->gc_obj_type_ = GCObjectType_Closure;
        SetObjectGen(c, gen);
        return c;
    }

    Upvalue * GC::NewUpvalue(GCGeneration gen)
    {
        auto u = new Upvalue;
        u->gc_obj_type_ = GCObjectType_Upvalue;
        SetObjectGen(u, gen);
        return u;
    }

    String * GC::NewString(GCGeneration gen)
    {
        auto s = new String;
        s->gc_obj_type_ = GCObjectType_String;
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
            unsigned int gen0_count = gen0_.count_;
            unsigned int gen0_threshold = gen0_.threshold_count_;
            unsigned int gen1_count = gen1_.count_;
            unsigned int gen1_threshold = gen1_.threshold_count_;
            unsigned int gen2_count = gen2_.count_;
            unsigned int gen2_threshold = gen2_.threshold_count_;

            const char *gc_name = "";
            clock_t start = clock();
            if (gen1_.count_ >= gen1_.threshold_count_)
            {
                gc_name = "major";
                MajorGC();
            }
            else
            {
                gc_name = "minor";
                MinorGC();
            }

            clock_t duration = clock() - start;
            unsigned int microseconds = duration * 1000000 / CLOCKS_PER_SEC;
            GC_LOG(gc_name << "[" << microseconds << " microseconds]: " <<
                   gen0_count << " " << gen0_threshold << " | " <<
                   gen1_count << " " << gen1_threshold << " | " <<
                   gen2_count << " " << gen2_threshold << " - " <<
                   gen0_.count_ << " " << gen0_.threshold_count_ << " | " <<
                   gen1_.count_ << " " << gen1_.threshold_count_ << " | " <<
                   gen2_.count_ << " " << gen2_.threshold_count_);
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
        AdjustThreshold(alived_gen0_count, gen0_, kGen0InitThresholdCount,
                        kGen0MaxThresholdCount);
    }

    void GC::MajorGC()
    {
        MajorGCMark();
        MajorGCSweep();

        barriered_.clear();
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
                obj_deleter_(obj, obj->gc_obj_type_);
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

        // Adjust GCGen0 threshold count
        AdjustThreshold(gen0_.count_, gen0_, kGen0InitThresholdCount,
                        kGen0MaxThresholdCount);

        gen1_.count_ += gen0_.count_;
        gen0_.count_ = 0;

        // Adjust GCGen1 threshold count
        AdjustThreshold(gen1_.count_, gen1_, kGen1InitThresholdCount,
                        kGen1MaxThresholdCount);
        if (gen1_.count_ >= kGen1MaxThresholdCount)
        {
            gen1_.threshold_count_ = gen1_.count_ + kGen1MaxThresholdCount;
        }
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
                obj_deleter_(obj, obj->gc_obj_type_);
                gen.count_--;
            }
        }

        gen.gen_ = alived;
    }

    void GC::AdjustThreshold(unsigned int alived_count, GenInfo &gen,
                             unsigned int min_threshold,
                             unsigned int max_threshold)
    {
        if (alived_count != 0)
        {
            while (gen.threshold_count_ < 2 * alived_count)
                gen.threshold_count_ *= 2;
            while (gen.threshold_count_ >= 4 * alived_count)
                gen.threshold_count_ /= 2;
        }

        if (gen.threshold_count_ < min_threshold)
            gen.threshold_count_ = min_threshold;
        else if (gen.threshold_count_ > max_threshold)
            gen.threshold_count_ = max_threshold;
    }

    void GC::DestroyGeneration(GenInfo &gen)
    {
        while (gen.gen_)
        {
            GCObject *obj = gen.gen_;
            gen.gen_ = gen.gen_->next_;
            obj_deleter_(obj, obj->gc_obj_type_);
        }
        gen.count_ = 0;
    }
} // namespace luna
