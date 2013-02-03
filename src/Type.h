#ifndef TYPE_H
#define TYPE_H

#include <functional>

namespace luna
{
    class GCObject;

    enum TypeT
    {
        TypeT_Nil,
        TypeT_Bool,
        TypeT_Number,
        TypeT_Obj,
    };

    struct Type
    {
        union
        {
            GCObject *obj_;
            double num_;
            bool bvalue_;
        };

        int type_;

        Type() : obj_(nullptr), type_(TypeT_Nil) { }
    };

    inline bool operator == (const Type &left, const Type &right)
    {
        return left.type_ == right.type_ &&
                ((left.type_ == TypeT_Nil) ||
                 (left.type_ == TypeT_Bool && left.bvalue_ == right.bvalue_) ||
                 (left.type_ == TypeT_Number && left.num_ == right.num_) ||
                 (left.type_ == TypeT_Obj && left.obj_ == right.obj_));
    }

    inline bool operator != (const Type &left, const Type &right)
    {
        return !(left == right);
    }
} // namespace luna

namespace std
{
    template<>
    struct hash<luna::Type> : public unary_function<luna::Type, size_t>
    {
        size_t operator () (const luna::Type &t) const
        {
            if (t.type_ == luna::TypeT_Nil)
                return hash<int>()(0);
            else if (t.type_ == luna::TypeT_Bool)
                return hash<bool>()(t.bvalue_);
            else if (t.type_ == luna::TypeT_Number)
                return hash<double>()(t.num_);
            else
                return hash<void *>()(t.obj_);
        }
    };
} // namespace std

#endif // TYPE_H
