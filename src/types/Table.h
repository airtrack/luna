#ifndef TABLE_H
#define TABLE_H

#include "Value.h"
#include <vector>
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

        int GetArraySize() const;
        void Set(int key, Value *value);
        void Set(Value *key, Value *value);
        void Erase(int key);

    private:
        struct TypeHash : public std::unary_function<Value *, std::size_t>
        {
            std::size_t operator() (Value *value) const
            {
                return value->GetHash();
            }
        };

        struct TypeEqual : public std::binary_function<Value *, Value *, bool>
        {
            bool operator() (Value *left, Value *right) const
            {
                return left->IsEqual(right);
            }
        };

        typedef std::vector<Value *> ArrayType;
        typedef std::unordered_map<Value *, Value *, TypeHash, TypeEqual> HashTableType;

        void SetArray(int key, Value *value);
        void SetHashTable(Value *key, Value *value);
        Value * GenerateType(int key);
        void EraseArray(int key);

        ArrayType *array_;
        HashTableType *hash_table_;
    };
} // namespace lua

#endif // TABLE_H
