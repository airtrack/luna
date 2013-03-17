#ifndef VALUE_H
#define VALUE_H

#include <functional>

namespace luna
{
    class GCObject;
    class String;
    class Closure;
    class Table;
    class State;

    typedef int (*CFunctionType)(State *);

    enum ValueT
    {
        ValueT_Nil,
        ValueT_Bool,
        ValueT_Number,
        ValueT_Obj,
        ValueT_String,
        ValueT_Closure,
        ValueT_Table,
        ValueT_CFunction,
    };

    // Value type of luna
    struct Value
    {
        union
        {
            GCObject *obj_;
            String *str_;
            Closure *closure_;
            Table *table_;
            CFunctionType cfunc_;
            double num_;
            bool bvalue_;
        };

        ValueT type_;

        Value() : obj_(nullptr), type_(ValueT_Nil) { }
    };

    inline bool operator == (const Value &left, const Value &right)
    {
        return left.type_ == right.type_ &&
                ((left.type_ == ValueT_Nil) ||
                 (left.type_ == ValueT_Bool && left.bvalue_ == right.bvalue_) ||
                 (left.type_ == ValueT_Number && left.num_ == right.num_) ||
                 (left.type_ == ValueT_Obj && left.obj_ == right.obj_) ||
                 (left.type_ == ValueT_String && left.str_ == right.str_) ||
                 (left.type_ == ValueT_Closure && left.closure_ == right.closure_) ||
                 (left.type_ == ValueT_Table && left.table_ == right.table_) ||
                 (left.type_ == ValueT_CFunction && left.cfunc_ == right.cfunc_));
    }

    inline bool operator != (const Value &left, const Value &right)
    {
        return !(left == right);
    }

    // Shared Upvalue used by Upvalue
    struct SharedUpvalue
    {
        Value value_;
        int shared_;

        SharedUpvalue() : shared_(0) { }

        void IncreaseShared() { ++shared_; }
        void DecreaseShared() { --shared_; }
    };

    // Upvalue type for Closure
    struct Upvalue
    {
        union
        {
            Value *stack_value_;
            SharedUpvalue *shared_;
        };

        enum { Stack, Shared } type_;

        Upvalue() : stack_value_(nullptr), type_(Stack) { }
    };
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
            else if (t.type_ == luna::ValueT_String)
                return hash<void *>()(t.str_);
            else if (t.type_ == luna::ValueT_Closure)
                return hash<void *>()(t.closure_);
            else if (t.type_ == luna::ValueT_Table)
                return hash<void *>()(t.table_);
            else if (t.type_ == luna::ValueT_CFunction)
                return hash<void *>()(reinterpret_cast<void *>(t.cfunc_));
            else
                return hash<void *>()(t.obj_);
        }
    };
} // namespace std

#endif // VALUE_H
