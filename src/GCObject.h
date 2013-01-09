#ifndef GC_OBJECT_H
#define GC_OBJECT_H

namespace luna
{
    class GCObject
    {
    public:
        virtual ~GCObject() = 0;
    };
} // namespace luna

#endif // GC_OBJECT_H
