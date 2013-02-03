#ifndef TABLE_H
#define TABLE_H

#include "Type.h"
#include "GCObject.h"
#include <vector>
#include <unordered_map>

namespace luna
{
    class Table : public GCObject
    {
    public:
        Table();

    private:
        typedef std::vector<Type> Array;
        typedef std::unordered_map<Type, Type> Hash;

        Array *array_;              // array part of table
        Hash *hash_;                // hash table part of table
    };
} // namespace luna

#endif // TABLE_H
