#ifndef VALUE_H
#define VALUE_H

#include <string>

namespace lua
{
    enum TypeId
    {
        TYPE_NIL,
        TYPE_BOOL,
        TYPE_NUMBER,
        TYPE_STRING,
        TYPE_TABLE,
        TYPE_FUNCTION,
    };

    class DataPool;

    class Value
    {
    public:
        virtual ~Value() { }
        virtual int Type() const = 0;
        virtual std::string Name() const = 0;
        virtual std::size_t GetHash() const = 0;
        virtual bool IsEqual(Value *other) const = 0;

    private:
        DataPool *owner_pool_;
    };
} // namespace lua

#endif // VALUE_H
