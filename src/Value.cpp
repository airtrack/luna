#include "Value.h"
#include "Function.h"
#include "Table.h"
#include "String.h"
#include "Upvalue.h"

namespace luna
{
    void Value::Accept(GCObjectVisitor *v) const
    {
        switch (type_)
        {
            case ValueT_Nil:
            case ValueT_Bool:
            case ValueT_Number:
            case ValueT_CFunction:
                break;
            case ValueT_Obj:
                obj_->Accept(v);
                break;
            case ValueT_String:
                str_->Accept(v);
                break;
            case ValueT_Closure:
                closure_->Accept(v);
                break;
            case ValueT_Upvalue:
                upvalue_->Accept(v);
                break;
            case ValueT_Table:
                table_->Accept(v);
                break;
        }
    }
} // namespace luna
