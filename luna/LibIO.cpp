#include "LibIO.h"
#include "State.h"
#include "String.h"
#include "UserData.h"
#include <cerrno>
#include <cstring>
#include <cstdio>

namespace lib {
namespace io {

#define METATABLE_FILE "file"

    void CloseFile(void *data)
    {
        std::fclose(reinterpret_cast<std::FILE *>(data));
    }

    int PushError(luna::StackAPI &api)
    {
        api.PushNil();
        api.PushString(std::strerror(errno));
        return 2;
    }

    int Close(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_UserData))
            return 0;

        auto user_data = api.GetUserData(0);
        CloseFile(user_data->GetData());
        user_data->MarkDestroyed();
        return 0;
    }

    int Flush(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_UserData))
            return 0;

        auto user_data = api.GetUserData(0);
        std::fflush(reinterpret_cast<std::FILE *>(user_data->GetData()));
        return 0;
    }

    int Setvbuf(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(2, luna::ValueT_UserData,
                           luna::ValueT_String, luna::ValueT_Number))
            return 0;

        auto user_data = api.GetUserData(0);
        auto mode = api.GetString(1)->GetStdString();

        std::size_t size = BUFSIZ;
        if (api.GetStackSize() > 2)
            size = static_cast<std::size_t>(api.GetNumber(2));

        auto file = reinterpret_cast<std::FILE *>(user_data->GetData());
        if (mode == "no")
            std::setvbuf(file, nullptr, _IONBF, 0);
        else if (mode == "full")
            std::setvbuf(file, nullptr, _IOFBF, size);
        else if (mode == "line")
            std::setvbuf(file, nullptr, _IOLBF, size);

        return 0;
    }

    int Write(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_UserData))
            return 0;

        auto user_data = api.GetUserData(0);
        auto file = reinterpret_cast<std::FILE *>(user_data->GetData());
        auto params = api.GetStackSize();
        for (int i = 1; i < params; ++i)
        {
            auto type = api.GetValueType(i);
            if (type == luna::ValueT_String)
            {
                auto str = api.GetString(i);
                auto c_str = str->GetCStr();
                auto len = str->GetLength();
                if (std::fwrite(c_str, len, 1, file) != 1)
                    return PushError(api);
            }
            else if (type == luna::ValueT_Number)
            {
                if (std::fprintf(file, "%.14g", api.GetNumber(i)) < 0)
                    return PushError(api);
            }
            else
            {
                api.ArgTypeError(i, luna::ValueT_String);
                return 0;
            }
        }

        api.PushUserData(user_data);
        return 1;
    }

    int Open(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_String, luna::ValueT_String))
            return 0;

        auto file_name = api.GetString(0);
        auto mode = "r";

        if (api.GetStackSize() > 1)
            mode = api.GetString(1)->GetCStr();

        auto file = std::fopen(file_name->GetCStr(), mode);
        if (!file)
            return PushError(api);

        auto user_data = state->NewUserData();
        auto metatable = state->GetMetatable(METATABLE_FILE);
        user_data->Set(file, metatable);
        user_data->SetDestroyer(CloseFile);
        api.PushUserData(user_data);
        return 1;
    }

    void RegisterLibIO(luna::State *state)
    {
        luna::Library lib(state);
        luna::TableMemberReg file[] = {
            { "close", Close },
            { "flush", Flush },
            { "setvbuf", Setvbuf },
            { "write", Write }
        };

        lib.RegisterMetatable(METATABLE_FILE, file);

        luna::TableMemberReg io[] = {
            { "open", Open }
        };

        lib.RegisterTableFunction("io", io);
    }

} // namespace io
} // namespace lib
