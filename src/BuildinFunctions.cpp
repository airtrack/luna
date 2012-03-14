#include "BuildinFunctions.h"
#include "NativeFunctionRegister.h"
#include "types/Number.h"
#include "types/String.h"
#include <assert.h>
#include <iostream>

namespace buildin
{
    namespace internal
    {
        void PrintValue(const lua::StackValue *sv)
        {
            switch (sv->param.value->Type())
            {
            case lua::TYPE_NIL:
            case lua::TYPE_BOOL:
                std::cout << sv->param.value->Name();
                break;
            case lua::TYPE_NUMBER:
                std::cout << static_cast<const lua::Number *>(sv->param.value)->Get();
                break;
            case lua::TYPE_STRING:
                std::cout << static_cast<const lua::String *>(sv->param.value)->Get();
                break;
            case lua::TYPE_TABLE:
            case lua::TYPE_FUNCTION:
            case lua::TYPE_NATIVE_FUNCTION:
                std::cout << sv->param.value->Name() << ':' << sv->param.value;
                break;
            }
        }

        void Print(lua::NativeFunctionStackOperator *so)
        {
            int params = so->GetSize();
            for (int index = 0; index < params; ++index)
            {
                if (index > 0)
                    std::cout << '\t';

                const lua::StackValue *sv = so->GetStackValue(index);
                assert(sv && sv->type == lua::StackValueType_Value);
                PrintValue(sv);
            }

            std::cout << '\n';
        }
    } // namespace internal

    void RegisterBuildin(lua::State *state)
    {
        lua::NativeFunctionRegister nf_register(state);
        nf_register.RegisterGlobal("print", internal::Print);
    }
} // namespace buildin
