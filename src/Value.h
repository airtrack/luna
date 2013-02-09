#ifndef VALUE_H
#define VALUE_H

#include <functional>

namespace luna
{
    class GCObject;

    enum ValueT
    {
        ValueT_Nil,
        ValueT_Bool,
        ValueT_Number,
        ValueT_Obj,
    };

    // Value type of lua
    struct Value
    {
        union
        {
            GCObject *obj_;
            double num_;
            bool bvalue_;
        };

        int type_;

        Value() : obj_(nullptr), type_(ValueT_Nil) { }
    };

    inline bool operator == (const Value &left, const Value &right)
    {
        return left.type_ == right.type_ &&
                ((left.type_ == ValueT_Nil) ||
                 (left.type_ == ValueT_Bool && left.bvalue_ == right.bvalue_) ||
                 (left.type_ == ValueT_Number && left.num_ == right.num_) ||
                 (left.type_ == ValueT_Obj && left.obj_ == right.obj_));
    }

    inline bool operator != (const Value &left, const Value &right)
    {
        return !(left == right);
    }
} // namespace luna

namespace std
{
    template<>
    struct hash<luna::Value> : public unary_function<luna::Value, size_t>
    {
        size_t operator () (const luna::Value &t) const
        {
            if (t.type_ == luna::ValueT_Nil)
                return hash<int>()(0);
            else if (t.type_ == luna::ValueT_Bool)
                return hash<bool>()(t.bvalue_);
            else if (t.type_ == luna::ValueT_Number)
                return hash<double>()(t.num_);
            else
                return hash<void *>()(t.obj_);
        }
    };
} // namespace std

#endif // VALUE_H
