#include "LibString.h"
#include "String.h"
#include <string>

namespace lib {
namespace string {

    int Byte(luna::State *state)
    {
        luna::StackAPI api(state);
        int params = api.GetStackSize();
        if (params < 1)
        {
            api.ArgCountError(1);
            return 0;
        }

        if (!api.IsString(0))
        {
            api.ArgTypeError(0, luna::ValueT_String);
            return 0;
        }

        int i = 1;
        if (params >= 2)
        {
            if (!api.IsNumber(1))
            {
                api.ArgTypeError(1, luna::ValueT_Number);
                return 0;
            }
            i = static_cast<int>(api.GetNumber(1));
        }

        int j = i;
        if (params >= 3)
        {
            if (!api.IsNumber(2))
            {
                api.ArgTypeError(2, luna::ValueT_Number);
                return 0;
            }
            j = static_cast<int>(api.GetNumber(2));
        }

        if (i <= 0)
            return 0;

        const luna::String *str = api.GetString(0);
        const char *s = str->GetCStr();
        int len = str->GetLength();
        int count = 0;

        for (int index = i - 1; index < j; ++index)
        {
            if (index >= 0 && index < len)
            {
                api.PushNumber(s[index]);
                ++count;
            }
        }
        return count;
    }

    int Char(luna::State *state)
    {
        luna::StackAPI api(state);
        int params = api.GetStackSize();

        std::string str;
        for (int i = 0; i < params; ++i)
        {
            if (!api.IsNumber(i))
            {
                api.ArgTypeError(i, luna::ValueT_Number);
                return 0;
            }
            else
            {
                str.push_back(static_cast<int>(api.GetNumber(i)));
            }
        }

        api.PushString(str);
        return 1;
    }

    void RegisterLibString(luna::State *state)
    {
        luna::Library lib(state);
        luna::TableFuncReg string[] = {
            { "byte", Byte },
            { "char", Char }
        };
        lib.RegisterTableFunction("string", string);
    }

} // namespace string
} // namespace lib
