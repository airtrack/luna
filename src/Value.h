#ifndef VALUE_H
#define VALUE_H

#include "GC.h"
#include <functional>

namespace luna
{
#define EXP_VALUE_COUNT_ANY -1

    class String;
    class Closure;
    class Upvalue;
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
        ValueT_Upvalue,
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
            Upvalue *upvalue_;
            Table *table_;
            CFunctionType cfunc_;
            double num_;
            bool bvalue_;
        };

        ValueT type_;

        Value() : obj_(nullptr), type_(ValueT_Nil) { }

        void SetNil() { obj_ = nullptr; type_ = ValueT_Nil; }
        void SetBool(bool bvalue) { bvalue_ = bvalue; type_ = ValueT_Bool; }

        void Accept(GCObjectVisitor *v) const;

        const char * TypeName() const;
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
                 (left.type_ == ValueT_Upvalue && left.upvalue_ == right.upvalue_) ||
                 (left.type_ == ValueT_Table && left.table_ == right.table_) ||
                 (left.type_ == ValueT_CFunction && left.cfunc_ == right.cfunc_));
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
            else if (t.type_ == luna::ValueT_String)
                return hash<void *>()(t.str_);
            else if (t.type_ == luna::ValueT_Closure)
                return hash<void *>()(t.closure_);
            else if (t.type_ == luna::ValueT_Upvalue)
                return hash<void *>()(t.upvalue_);
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
