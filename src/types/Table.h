#ifndef TABLE_H
#define TABLE_H

#include "Value.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace lua
{
    class Table : public Value
    {
    public:
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

        void Assign(const Value *key, Value *value);

    private:
        struct TypeHash : public std::unary_function<Value *, std::size_t>
        {
            std::size_t operator() (const Value *value) const
            {
                return value->GetHash();
            }
        };

        struct TypeEqual : public std::binary_function<Value *, Value *, bool>
        {
            bool operator() (const Value *left, const Value *right) const
            {
                return left->IsEqual(right);
            }
        };

        typedef std::vector<Value *> ArrayType;
        typedef std::unordered_map<const Value *, Value *, TypeHash, TypeEqual> HashTableType;

        std::unique_ptr<ArrayType> array_;
        std::unique_ptr<HashTableType> hash_table_;
    };
} // namespace lua

#endif // TABLE_H
