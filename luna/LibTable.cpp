#include "LibTable.h"
#include "State.h"
#include "Table.h"

namespace lib {
namespace table {

    int Insert(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(2, luna::ValueT_Table))
            return 0;

        auto params = api.GetStackSize();
        auto table = api.GetTable(0);
        auto index = table->ArraySize() + 1;
        int value = 1;

        if (params > 2)
        {
            if (!api.IsNumber(1))
            {
                api.ArgTypeError(1, luna::ValueT_Number);
                return 0;
            }

            index = static_cast<decltype(index)>(api.GetNumber(1));
            value = 2;
        }

        api.PushBool(table->InsertArrayValue(index, *api.GetValue(value)));
        return 1;
    }

    int Pack(luna::State *state)
    {
        luna::StackAPI api(state);

        auto table = state->NewTable();
        auto params = api.GetStackSize();
        for (int i = 0; i < params; ++i)
            table->SetArrayValue(i + 1, *api.GetValue(i));

        api.PushTable(table);
        return 1;
    }

    int Remove(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_Table, luna::ValueT_Number))
            return 0;

        auto table = api.GetTable(0);
        auto index = table->ArraySize();

        // There is no elements in array of table.
        if (index == 0)
        {
            api.PushBool(false);
            return 1;
        }

        auto params = api.GetStackSize();
        if (params > 1)
            index = static_cast<decltype(index)>(api.GetNumber(1));

        api.PushBool(table->EraseArrayValue(index));
        return 1;
    }

    int Unpack(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_Table,
                           luna::ValueT_Number, luna::ValueT_Number))
            return 0;

        auto params = api.GetStackSize();
        auto table = api.GetTable(0);

        int begin = 1;
        int end = table->ArraySize();
        if (params > 1)
            begin = static_cast<int>(api.GetNumber(1));
        if (params > 2)
            end = static_cast<int>(api.GetNumber(2));

        int count = 0;
        luna::Value key;
        key.type_ = luna::ValueT_Number;
        for (int i = begin; i <= end; ++i)
        {
            key.num_ = i;
            api.PushValue(table->GetValue(key));
            ++count;
        }

        return count;
    }

    void RegisterLibTable(luna::State *state)
    {
        luna::Library lib(state);
        luna::TableMemberReg table[] = {
            { "insert", Insert },
            { "pack", Pack },
            { "remove", Remove },
            { "unpack", Unpack }
        };

        lib.RegisterTableFunction("table", table);
    }

} // namespace table
} // namespace lib
