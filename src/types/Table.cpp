#include "Table.h"
#include "Number.h"
#include <assert.h>
#include <iterator>

namespace lua
{
    std::size_t Table::GetHash() const
    {
        return std::hash<const Table *>()(this);
    }

    bool Table::IsEqual(const Value *other) const
    {
        return this == other;
    }

    int Table::GetArraySize() const
    {
        if (array_)
            return array_->size();
        return 0;
    }

    void Table::Set(int key, Value *value)
    {
        int array_size = GetArraySize();
        if (key >= 1 && key <= array_size + 1)
            SetArray(key, value);
        else
            SetHashTable(GenerateType(key), value);
    }

    void Table::Set(Value *key, Value *value)
    {
        if (key->Type() == TYPE_NUMBER)
        {
            Number *n = static_cast<Number *>(key);
            if (n->IsInteger())
                Set(n->ConvertToInteger(), value);
            else
                SetHashTable(key, value);
        }
        else
        {
            SetHashTable(key, value);
        }
    }

    void Table::Erase(int key)
    {
        int array_size = GetArraySize();
        if (key >= 1 && key <= array_size)
            EraseArray(key);
        else
            SetHashTable(GenerateType(key), 0);
    }

    void Table::SetArray(int key, Value *value)
    {
    }

    void Table::SetHashTable(Value *key, Value *value)
    {
    }

    Value * Table::GenerateType(int key)
    {
        return 0;
    }

    void Table::EraseArray(int key)
    {
        assert(key >= 1 && key <= GetArraySize());
        ArrayType::iterator it = array_->begin();
        std::advance(it, key - 1);
        array_->erase(it);
    }
} // namespace lua
