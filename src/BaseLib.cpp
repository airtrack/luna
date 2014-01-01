#include "BaseLib.h"
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
                    printf("%s", api.GetString(i));
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

    void RegisterBaseLib(luna::State *state)
    {
        luna::Library lib(state);
        lib.RegisterFunc("print", Print);
    }

} // namespace base
} // namespace lib
