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

    bool Table::HaveKey(const Value *key) const
    {
        if (!hash_table_)
            return false;

        HashTableType::iterator it = hash_table_->find(key);
        return it != hash_table_->end();
    }
} // namespace lua
