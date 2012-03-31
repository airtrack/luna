#ifndef TABLE_H
#define TABLE_H

#include "Value.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace lua
{
    class DataPool;

    class TableValue : public Value
    {
    public:
        explicit TableValue(Value *value)
            : value_(value)
        {
        }

        virtual int Type() const
        {
            return value_->Type();
        }

        virtual std::string Name() const
        {
            return value_->Name();
        }

        virtual std::size_t GetHash() const
        {
            return value_->GetHash();
        }

        virtual bool IsEqual(const Value *other) const
        {
            return value_->IsEqual(other);
        }

        Value * GetValue()
        {
            return value_;
        }

        void SetValue(Value *value)
        {
            value_ = value;
        }

    private:
        Value *value_;
    };

    class Table : public Value
    {
    public:
        explicit Table(DataPool *pool);

        virtual int Type() const
        {
            return TYPE_TABLE;
        }

        virtual std::string Name() const
        {
            return "table";
        }

        virtual std::size_t GetHash() const;
        virtual bool IsEqual(const Value *other) const;

        bool HaveKey(const Value *key) const;

        Value * GetValue(const Value *key);
        TableValue * GetTableValue(const Value *key);

        void Assign(const Value *key, Value *value);
        void Assign(const Value *key, TableValue *table_value);

    private:
        typedef std::vector<TableValue *> ArrayType;
        typedef std::unordered_map<const Value *, TableValue *, ValueHasher, ValueEqualer> HashTableType;

        DataPool *data_pool_;
        std::unique_ptr<ArrayType> array_;
        std::unique_ptr<HashTableType> hash_table_;
    };
} // namespace lua

#endif // TABLE_H