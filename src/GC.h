#ifndef GC_OBJECT_H
#define GC_OBJECT_H

namespace luna
{
    enum GCGeneration
    {
        GCGen0,         // Youngest generation
        GCGen1,         // Mesozoic generation
        GCGen2,         // Oldest generation
    };

    class GCObject
    {
        friend class GC;
    public:
        GCObject();
        virtual ~GCObject() = 0;

    private:
        // Pointing next GCObject in current generation
        GCObject *next_;
        // Generation flag
        unsigned int generation_ : 2;
        // GC flag
        unsigned int gc_ : 2;
    };

    class Table;
    class Function;
    class Closure;

    class GC
    {
    public:
        GC();

        // Alloc GC objects
        Table * NewTable(GCGeneration gen = GCGen0);
        Function * NewFunction(GCGeneration gen = GCGen2);
        Closure * NewClosure(GCGeneration gen = GCGen0);

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

        // Youngest generation
        GenInfo gen0_;
        // Mesozoic generation
        GenInfo gen1_;
        // Oldest generation
        GenInfo gen2_;
    };
} // namespace luna

#endif // GC_OBJECT_H
