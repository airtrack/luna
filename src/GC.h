#ifndef GC_OBJECT_H
#define GC_OBJECT_H

#include <functional>
#include <deque>

namespace luna
{
    enum GCGeneration
    {
        GCGen0,         // Youngest generation
        GCGen1,         // Mesozoic generation
        GCGen2,         // Oldest generation
    };

    enum GCFlag
    {
        GCFlag_White,
        GCFlag_Black,
    };

    class Table;
    class Function;
    class Closure;
    class String;

    class GCObjectVisitor
    {
    public:
        // Need visit all GC object members when return true
        virtual bool Visit(Table *) = 0;
        virtual bool Visit(Function *) = 0;
        virtual bool Visit(Closure *) = 0;
        virtual bool Visit(String *) = 0;
    };

    class GCObject
    {
        friend class GC;
    public:
        GCObject();
        virtual ~GCObject() = 0;

        virtual void Accept(GCObjectVisitor *) = 0;

    private:
        // Pointing next GCObject in current generation
        GCObject *next_;
        // Generation flag
        unsigned int generation_ : 2;
        // GC flag
        unsigned int gc_ : 2;
    };

    class GC
    {
    public:
        typedef std::function<void ()> RootTravelType;

        GC();

        // Set minor and major root travel functions
        void SetRootTraveller(const RootTravelType &minor, const RootTravelType &major);

        // Alloc GC objects
        Table * NewTable(GCGeneration gen = GCGen0);
        Function * NewFunction(GCGeneration gen = GCGen2);
        Closure * NewClosure(GCGeneration gen = GCGen0);

        // Set GC object barrier
        void SetBarrier(GCObject *obj);

    private:
        struct GenInfo
        {
            // Pointing to GC object list
            GCObject *gen_;
            // Count of GC objects
            unsigned int count_;
            // Current max count of GC objects
            unsigned int max_count_;

            GenInfo() : gen_(nullptr), count_(0), max_count_(0) { }
        };

        void SetObjectGen(GCObject *obj, GCGeneration gen);

        // Run minor and major GC
        void MinorGC();
        void MajorGC();

        static const unsigned int kGen0InitMaxCount = 512;
        static const unsigned int kGen1InitMaxCount = 512;

        // Youngest generation
        GenInfo gen0_;
        // Mesozoic generation
        GenInfo gen1_;
        // Oldest generation
        GenInfo gen2_;

        // Minor root traveller
        RootTravelType minor_traveller_;
        // Major root traveller
        RootTravelType major_traveller_;

        // Barriered GC objects
        std::deque<GCObject *> barriered_;
    };
} // namespace luna

#endif // GC_OBJECT_H
