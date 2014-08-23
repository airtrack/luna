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

    void ReadBytes(luna::StackAPI &api, std::FILE *file, int bytes)
    {
        if (bytes <= 0)
            api.PushString("");
        else
        {
            std::vector<char> buf(bytes);
            bytes = std::fread(&buf[0], sizeof(buf[0]), bytes, file);
            if (bytes == 0)
                api.PushNil();
            else
                api.PushString(&buf[0], bytes);
        }
    }

    void ReadByFormat(luna::StackAPI &api, std::FILE *file,
                      const std::string format)
    {
        if (format == "*n")
        {
            // Read a number
            double num = 0.0;
            if (std::fscanf(file, "%lg", &num) == 1)
                api.PushNumber(num);
            else
                api.PushNil();
        }
        else if (format == "*a")
        {
            // Read total content of file
            auto cur = std::ftell(file);
            std::fseek(file, 0, SEEK_END);
            auto bytes = std::ftell(file) - cur;
            std::fseek(file, cur, SEEK_SET);
            if (bytes == 0)
                api.PushString("");
            else
            {
                std::vector<char> buf(bytes);
                bytes = std::fread(&buf[0], sizeof(buf[0]), bytes, file);
                api.PushString(&buf[0], bytes);
            }
        }
        else if (format == "*l" || format == "*L")
        {
            // Read line
            const std::size_t part = 128;
            const std::size_t part_count = part + 1;

            std::size_t count = 0;
            std::vector<char> buf(part_count);
            do
            {
                if (!std::fgets(&buf[count], part_count, file))
                {
                    // EOF or an error occured
                    // if count is 0, push a nil as end of file
                    // otherwise, push the buf as string
                    if (count == 0)
                        api.PushNil();
                    else
                        api.PushString(&buf[0], count);
                    break;
                }
                else
                {
                    auto size = buf.size();
                    while (count < size - 1 &&
                            buf[count] != '\n' && buf[count] != '\0')
                        ++count;
                    // buf[size - 1] must be '\0'
                    if (count == size - 1)
                    {
                        // '\n' is not exist, extend buf to continue read
                        buf.resize(size + part);
                    }
                    else
                    {
                        // If buf[count] is '\n', keep '\n' when
                        // format is "*L"
                        if (buf[count] == '\n' && format == "*L") ++count;
                        api.PushString(&buf[0], count);
                        break;
                    }
                }
            } while (true);
        }
        else
            api.PushNil();
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

    int Read(luna::State *state)
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
            if (type == luna::ValueT_Number)
            {
                auto bytes = static_cast<int>(api.GetNumber(i));
                ReadBytes(api, file, bytes);
            }
            else if (type == luna::ValueT_String)
            {
                auto format = api.GetString(i)->GetStdString();
                ReadByFormat(api, file, format);
            }
            else
                api.PushNil();
        }

        return params - 1;
    }

    int Seek(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_UserData,
                           luna::ValueT_String, luna::ValueT_Number))
            return 0;

        auto user_data = api.GetUserData(0);
        auto file = reinterpret_cast<std::FILE *>(user_data->GetData());

        auto params = api.GetStackSize();
        if (params > 1)
        {
            auto whence = api.GetString(1)->GetStdString();
            long offset = 0;
            if (params > 2)
                offset = static_cast<long>(api.GetNumber(2));

            int res = 0;
            if (whence == "set")
                res = std::fseek(file, offset, SEEK_SET);
            else if (whence == "cur")
                res = std::fseek(file, offset, SEEK_CUR);
            else if (whence == "end")
                res = std::fseek(file, offset, SEEK_END);

            if (res != 0)
                return PushError(api);
        }

        auto pos = std::ftell(file);
        if (pos < 0)
            return PushError(api);

        api.PushNumber(pos);
        return 1;
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

    int Stdin(luna::State *state)
    {
        luna::StackAPI api(state);
        auto user_data = state->NewUserData();
        auto metatable = state->GetMetatable(METATABLE_FILE);
        user_data->Set(stdin, metatable);
        api.PushUserData(user_data);
        return 1;
    }

    int Stdout(luna::State *state)
    {
        luna::StackAPI api(state);
        auto user_data = state->NewUserData();
        auto metatable = state->GetMetatable(METATABLE_FILE);
        user_data->Set(stdout, metatable);
        api.PushUserData(user_data);
        return 1;
    }

    int Stderr(luna::State *state)
    {
        luna::StackAPI api(state);
        auto user_data = state->NewUserData();
        auto metatable = state->GetMetatable(METATABLE_FILE);
        user_data->Set(stderr, metatable);
        api.PushUserData(user_data);
        return 1;
    }

    void RegisterLibIO(luna::State *state)
    {
        luna::Library lib(state);
        luna::TableMemberReg file[] = {
            { "close", Close },
            { "flush", Flush },
            { "read", Read },
            { "seek", Seek },
            { "setvbuf", Setvbuf },
            { "write", Write }
        };

        lib.RegisterMetatable(METATABLE_FILE, file);

        luna::TableMemberReg io[] = {
            { "open", Open },
            { "stdin", Stdin },
            { "stdout", Stdout },
            { "stderr", Stderr }
        };

        lib.RegisterTableFunction("io", io);
    }

} // namespace io
} // namespace lib
