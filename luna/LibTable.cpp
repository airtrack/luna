#include "LibTable.h"
#include "State.h"
#include "Table.h"

namespace lib {
namespace table {

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
            { "pack", Pack },
            { "unpack", Unpack }
        };

        lib.RegisterTableFunction("table", table);
    }

} // namespace table
} // namespace lib
