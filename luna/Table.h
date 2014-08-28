#ifndef TABLE_H
#define TABLE_H

#include "GC.h"
#include "Value.h"
#include <memory>
#include <vector>
#include <unordered_map>

namespace luna
{
    // Table has array part and hash table part.
    class Table : public GCObject
    {
    public:
        Table();

        virtual void Accept(GCObjectVisitor *v);

        // Set array value by index, return true if success.
        // 'index' start from 1, if 'index' == ArraySize() + 1,
        // then append value to array.
        bool SetArrayValue(std::size_t index, const Value &value);

        // If 'index' == ArraySize() + 1, then append value to array,
        // otherwise shifting up all values which start from 'index',
        // and insert value to 'index' of array.
        // Return true when insert success.
        bool InsertArrayValue(std::size_t index, const Value &value);

        // Erase the value by 'index' in array if 'index' is legal,
        // shifting down all values which start from 'index' + 1.
        // Return true when erase success.
        bool EraseArrayValue(std::size_t index);

        // Add key-value into table.
        // If key is number and key fit with array, then insert into array,
        // otherwise insert into hash table.
        void SetValue(const Value &key, const Value &value);

        // Get Value of key from array first,
        // if key is number, then get the value from array when key number
        // is fit with array as index, otherwise try search in hash table.
        // Return value is 'nil' if 'key' is not existed.
        Value GetValue(const Value &key) const;

        // Get first key-value pair of table, return true if table is not empty.
        bool FirstKeyValue(Value &key, Value &value);

        // Get the next key-value pair by current 'key', return false if there
        // is no key-value pair any more.
        bool NextKeyValue(const Value &key, Value &next_key, Value &next_value);

        // Return the number of array part elements.
        std::size_t ArraySize() const;

    private:
        typedef std::vector<Value> Array;
        typedef std::unordered_map<Value, Value> Hash;

        // Combine AppendToArray and MergeFromHashToArray
        void AppendAndMergeFromHashToArray(const Value &value);

        // Append value to array.
        void AppendToArray(const Value &value);

        // Try to move values from hash to array which keys start from
        // ArraySize() + 1
        void MergeFromHashToArray();

        // Move hash table key-value pair to array which key is number and key
        // fit with array, return true if move success.
        bool MoveHashToArray(const Value &key);

        std::unique_ptr<Array> array_;              // array part of table
        std::unique_ptr<Hash> hash_;                // hash table part of table
    };
} // namespace luna

#endif // TABLE_H
