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

    void Table::Mark()
    {
        if (IsSelfMarked())
            return ;

        MarkSelf();
        MarkArray();
        MarkHashTable();
    }

    std::size_t Table::GetArraySize() const
    {
        if (array_)
            return array_->size();
        return 0;
    }

    bool Table::HaveKey(const Value *key) const
    {
        if (ArrayHasKey(key))
            return true;

        if (HashTableHasKey(key))
            return true;

        return false;
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
        if (ArrayHasKey(key))
        {
            int index = static_cast<const Number *>(key)->GetInteger();
            return (*array_)[index - 1];
        }

        if (HashTableHasKey(key))
        {
            HashTableType::iterator it = hash_table_->find(key);
            return it->second;
        }

        return 0;
    }

    void Table::ArrayAssign(std::size_t array_index, Value *value)
    {
        // Array index start from 1
        assert(array_index >= 1);

        std::size_t array_size = GetArraySize();
        if (array_index <= array_size)
            (*array_)[array_index - 1]->SetValue(value);
        else
            ArrayAssign(array_index, data_pool_->GetTableValue(value));
    }

    void Table::ArrayAssign(std::size_t array_index, TableValue *table_value)
    {
        // Array index start from 1
        assert(array_index >= 1);

        if (!array_)
            array_.reset(new ArrayType);

        std::size_t array_size = array_->size();
        if (array_index <= array_size)
            (*array_)[array_index - 1] = table_value;
        else if (array_index == array_size + 1)
            array_->push_back(table_value);
        else
            Assign(data_pool_->GetNumber(array_index), table_value);
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
        if (key->Type() == TYPE_NUMBER)
        {
            const Number *number = static_cast<const Number *>(key);
            if (number->IsInteger())
            {
                int index = number->GetInteger();
                int array_size = GetArraySize();
                if (index >= 1 && index <= array_size + 1)
                    return ArrayAssign(index, table_value);
            }
        }

        if (!hash_table_)
            hash_table_.reset(new HashTableType);

        (*hash_table_)[key] = table_value;
    }

    bool Table::HashTableHasKey(const Value *key) const
    {
        if (!hash_table_)
            return false;

        return hash_table_->find(key) != hash_table_->end();
    }

    bool Table::ArrayHasKey(const Value *key) const
    {
        if (!array_)
            return false;

        if (key->Type() != TYPE_NUMBER)
            return false;

        const Number *number = static_cast<const Number *>(key);
        if (!number->IsInteger())
            return false;

        int index = number->GetInteger();
        int size = array_->size();
        if (index < 1 || index > size)
            return false;
        return true;
    }

    void Table::MarkArray()
    {
        if (!array_)
            return ;

        for (auto it = array_->begin(); it != array_->end(); ++it)
            (*it)->Mark();
    }

    void Table::MarkHashTable()
    {
        if (!hash_table_)
            return ;

        for (auto it = hash_table_->begin(); it != hash_table_->end(); ++it)
        {
            Value *key = const_cast<Value *>(it->first);
            key->Mark();
            it->second->Mark();
        }
    }
} // namespace lua
