#ifndef DATA_POOL_H
#define DATA_POOL_H

#include "types/Value.h"
#include "types/Bool.h"
#include "types/Nil.h"
#include "types/Number.h"
#include "types/String.h"
#include "types/Table.h"
#include "types/Function.h"
#include "types/Closure.h"
#include "types/NativeFunction.h"
#include <string>

namespace lua
{
    class DataPool
    {
    public:
        DataPool();

        Nil * GetNil();
        Bool * GetTrue();
        Bool * GetFalse();
        Number * GetNumber(double number);
        String * GetString(const std::string& str);
        Table * GetTable();
        TableValue * GetTableValue(Value *value);
        Function * GetFunction(std::unique_ptr<UpValueNameSet> &&up_value_set);
        Closure * GetClosure(Function *func);
        NativeFunction * GetNativeFunction(const NativeFunction::FuncType& func);

    private:
        Nil nil_;
        Bool true_;
        Bool false_;
    };
} // namespace lua

#endif // DATA_POOL_H
