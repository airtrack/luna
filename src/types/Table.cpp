#include "Table.h"
#include "Number.h"
#include "../DataPool.h"
#include <assert.h>
#include <iterator>

namespace lua
{
    Table::Table(DataPool *pool)
        : data_pool_(pool)
    {
    }

    std::size_t Table::GetHash() const
    {
        return std::hash<const Table *>()(this);
    }

    bool Table::IsEqual(const Value *other) const
    {
        return this == other;
    }

    std::size_t Table::GetArraySize() const
    {
        if (array_)
            return array_->size();
        return 0;
    }

    bool Table::HaveKey(const Value *key) const
    {
        if (!hash_table_)
            return false;

        HashTableType::iterator it = hash_table_->find(key);
        return it != hash_table_->end();
    }

    Value * Table::GetValue(const Value *key)
    {
        TableValue *table_value = GetTableValue(key);
        if (!table_value)
            return 0;
        return table_value->GetValue();
    }

    TableValue * Table::GetTableValue(const Value *key)
    {
        if (!hash_table_)
            return 0;

        HashTableType::iterator it = hash_table_->find(key);
        if (it == hash_table_->end())
            return 0;

        return it->second;
    }

    void Table::ArrayAssign(std::size_t array_index, Value *value)
    {
        // Array index start from 1
        assert(array_index >= 1);

        if (!array_)
            array_.reset(new ArrayType);

        std::size_t array_size = array_->size();
        if (array_index <= array_size)
            (*array_)[array_index - 1]->SetValue(value);
        else if (array_index == array_size + 1)
            array_->push_back(data_pool_->GetTableValue(value));
        else
            Assign(data_pool_->GetNumber(array_index), value);
    }

    void Table::Assign(const Value *key, Value *value)
    {
        TableValue *tv = GetTableValue(key);
        if (tv)
            tv->SetValue(value);
        else
            Assign(key, data_pool_->GetTableValue(value));
    }

    void Table::Assign(const Value *key, TableValue *table_value)
    {
        if (!hash_table_)
            hash_table_.reset(new HashTableType);

        (*hash_table_)[key] = table_value;
    }
} // namespace lua
