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
} // namespace luna

#endif // GC_OBJECT_H
