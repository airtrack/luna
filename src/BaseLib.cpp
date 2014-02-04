#include "BaseLib.h"
#include "Table.h"
#include "String.h"
#include <string>
#include <iostream>
#include <stdio.h>

namespace lib {
namespace base {

    int Print(luna::State *state)
    {
        luna::StackAPI api(state);
        int params = api.GetStackSize();

        for (int i = 0; i < params; ++i)
        {
            luna::ValueT type = api.GetValueType(i);

            switch (type) {
                case luna::ValueT_Nil:
                    printf("nil");
                    break;
                case luna::ValueT_Bool:
                    printf("%s", api.GetBool(i) ? "true" : "false");
                    break;
                case luna::ValueT_Number:
                    printf("%g", api.GetNumber(i));
                    break;
                case luna::ValueT_String:
                    printf("%s", api.GetCString(i));
                    break;
                case luna::ValueT_Closure:
                    printf("function:\t%p", api.GetClosure(i));
                    break;
                case luna::ValueT_Table:
                    printf("table:\t%p", api.GetTable(i));
                    break;
                case luna::ValueT_CFunction:
                    printf("function:\t%p", api.GetCFunction(i));
                    break;
                default:
                    break;
            }

            if (i != params - 1)
                printf("\t");
        }

        printf("\n");
        return 0;
    }

    int DoIPairs(luna::State *state)
    {
        luna::StackAPI api(state);
        int params = api.GetStackSize();
        if (params < 2)
        {
            api.ArgCountError(2);
            return 0;
        }

        if (!api.IsTable(0))
        {
            api.ArgTypeError(0, luna::ValueT_Table);
            return 0;
        }

        if (!api.IsNumber(1))
        {
            api.ArgTypeError(1, luna::ValueT_Number);
            return 0;
        }

        luna::Table *t = api.GetTable(0);
        double num = api.GetNumber(1) + 1;

        luna::Value k;
        k.type_ = luna::ValueT_Number;
        k.num_ = num;
        luna::Value v = t->GetValue(k);

        if (v.type_ == luna::ValueT_Nil)
            return 0;

        api.PushValue(k);
        api.PushValue(v);
        return 2;
    }

    int IPairs(luna::State *state)
    {
        luna::StackAPI api(state);
        int params = api.GetStackSize();
        if (params < 1)
        {
            api.ArgCountError(1);
            return 0;
        }

        if (!api.IsTable(0))
        {
            api.ArgTypeError(0, luna::ValueT_Table);
            return 0;
        }

        luna::Table *t = api.GetTable(0);
        api.PushCFunction(DoIPairs);
        api.PushTable(t);
        api.PushNumber(0);
        return 3;
    }

    int DoPairs(luna::State *state)
    {
        luna::StackAPI api(state);
        int params = api.GetStackSize();
        if (params < 2)
        {
            api.ArgCountError(2);
            return 0;
        }

        if (!api.IsTable(0))
        {
            api.ArgTypeError(0, luna::ValueT_Table);
            return 0;
        }

        luna::Table *t = api.GetTable(0);
        luna::Value *last_key = api.GetValue(1);

        luna::Value key;
        luna::Value value;
        if (last_key->type_ == luna::ValueT_Nil)
            t->FirstKeyValue(key, value);
        else
            t->NextKeyValue(*last_key, key, value);

        api.PushValue(key);
        api.PushValue(value);
        return 2;
    }

    int Pairs(luna::State *state)
    {
        luna::StackAPI api(state);
        int params = api.GetStackSize();
        if (params < 1)
        {
            api.ArgCountError(1);
            return 0;
        }

        if (!api.IsTable(0))
        {
            api.ArgTypeError(0, luna::ValueT_Table);
            return 0;
        }

        luna::Table *t = api.GetTable(0);
        api.PushCFunction(DoPairs);
        api.PushTable(t);
        api.PushNil();
        return 3;
    }

    int GetLine(luna::State *state)
    {
        luna::StackAPI api(state);
        std::string line;
        std::getline(std::cin, line);
        api.PushString(line.c_str());
        return 1;
    }

    int GetStrChar(luna::State *state)
    {
        luna::StackAPI api(state);
        int params = api.GetStackSize();
        if (params < 2)
        {
            api.ArgCountError(2);
            return 0;
        }

        if (!api.IsString(0))
        {
            api.ArgTypeError(0, luna::ValueT_String);
            return 0;
        }

        if (!api.IsNumber(1))
        {
            api.ArgTypeError(1, luna::ValueT_Number);
            return 0;
        }

        const luna::String *s = api.GetString(0);
        int index = static_cast<int>(api.GetNumber(1));
        int len = s->GetLength();
        if (index >= 0 && index < len)
            api.PushNumber(*(s->GetCStr() + index));
        else
            api.PushNumber(0);
        return 1;
    }

    void RegisterBaseLib(luna::State *state)
    {
        luna::Library lib(state);
        lib.RegisterFunc("print", Print);
        lib.RegisterFunc("ipairs", IPairs);
        lib.RegisterFunc("pairs", Pairs);
        lib.RegisterFunc("getline", GetLine);
        lib.RegisterFunc("getstrchar", GetStrChar);
    }

} // namespace base
} // namespace lib
