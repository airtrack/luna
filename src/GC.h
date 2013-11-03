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
        virtual ~GCObject() = 0;

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
        GC();

    private:
        // Youngest generation object list
        GCObject *gen0_;
        // Mesozoic generation object list
        GCObject *gen1_;
        // Oldest generation object list
        GCObject *gen2_;
    };
} // namespace luna

#endif // GC_OBJECT_H
