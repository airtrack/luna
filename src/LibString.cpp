#include "LibString.h"
#include "String.h"
#include <algorithm>
#include <string>
#include <cctype>
#include <cmath>

namespace lib {
namespace string {

    int Byte(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_String,
                           luna::ValueT_Number, luna::ValueT_Number))
            return 0;

        int params = api.GetStackSize();

        int i = 1;
        if (params >= 2)
            i = static_cast<int>(api.GetNumber(1));

        int j = i;
        if (params >= 3)
            j = static_cast<int>(api.GetNumber(2));

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

    int Len(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_String))
            return 0;

        api.PushNumber(api.GetString(0)->GetLength());
        return 1;
    }

    int Lower(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_String))
            return 0;

        auto str = api.GetString(0);
        auto size = str->GetLength();
        auto c_str = reinterpret_cast<const unsigned char *>(str->GetCStr());

        std::string lower;
        lower.reserve(size);
        for (std::size_t i = 0; i < size; ++i)
            lower.push_back(std::tolower(c_str[i]));

        api.PushString(lower);
        return 1;
    }

    int Reverse(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_String))
            return 0;

        auto str = api.GetString(0);
        auto size = str->GetLength();
        auto c_str = str->GetCStr();

        std::string reverse;
        reverse.reserve(size);
        for (; size > 0; --size)
            reverse.push_back(c_str[size - 1]);

        api.PushString(reverse);
        return 1;
    }

    int Sub(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(2, luna::ValueT_String,
                           luna::ValueT_Number, luna::ValueT_Number))
            return 0;

        auto str = api.GetString(0);
        int size = str->GetLength();
        auto c_str = str->GetCStr();
        auto start = static_cast<int>(api.GetNumber(1));
        auto end = size;

        auto params = api.GetStackSize();
        if (params <= 2)
        {
            if (start == 0)
            {
                start = 1;
            }
            else if (start < 0)
            {
                start += size;
                start = start < 0 ? 1 : start + 1;
            }
        }
        else
        {
            start = start == 0 ? 1 : std::abs(start);
            end = std::abs(static_cast<int>(api.GetNumber(2)));
            end = std::min(end, size);
        }

        std::string sub;
        for (int i = start; i <= end; ++i)
            sub.push_back(c_str[i - 1]);

        api.PushString(sub);
        return 1;
    }

    int Upper(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_String))
            return 0;

        auto str = api.GetString(0);
        auto size = str->GetLength();
        auto c_str = reinterpret_cast<const unsigned char *>(str->GetCStr());

        std::string upper;
        upper.reserve(size);
        for (std::size_t i = 0; i < size; ++i)
            upper.push_back(std::toupper(c_str[i]));

        api.PushString(upper);
        return 1;
    }

    void RegisterLibString(luna::State *state)
    {
        luna::Library lib(state);
        luna::TableMemberReg string[] = {
            { "byte", Byte },
            { "char", Char },
            { "len", Len },
            { "lower", Lower },
            { "reverse", Reverse },
            { "sub", Sub },
            { "upper", Upper }
        };
        lib.RegisterTableFunction("string", string);
    }

} // namespace string
} // namespace lib
