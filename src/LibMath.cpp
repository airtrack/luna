#include "LibMath.h"
#include <random>
#include <math.h>
#include <stdlib.h>

namespace lib {
namespace math {

    // Rand engine for math.random function
    class RandEngine
    {
    public:
        typedef unsigned int result_type;
#ifdef _MSC_VER
        static result_type min() { return 0; }
        static result_type max() { return RAND_MAX; }
#else
        static constexpr result_type min() { return 0; }
        static constexpr result_type max() { return RAND_MAX; }
#endif // _MSC_VER
        result_type operator() () { return rand(); }

        RandEngine() { }
        RandEngine(const RandEngine&) = delete;
        void operator = (const RandEngine&) = delete;
    };

    int Random(luna::State *state)
    {
        luna::StackAPI api(state);
        int params = api.GetStackSize();

        if (params == 0)
        {
            RandEngine engine;
            std::uniform_real_distribution<> dis;
            api.PushNumber(dis(engine));
        }
        else if (params == 1)
        {
            if (!api.IsNumber(0))
            {
                api.ArgTypeError(0, luna::ValueT_Number);
                return 0;
            }
            auto max = static_cast<unsigned long long>(api.GetNumber(0));

            RandEngine engine;
            std::uniform_int_distribution<unsigned long long> dis(1, max);
            api.PushNumber(static_cast<double>(dis(engine)));
        }
        else if (params >= 2)
        {
            if (!api.IsNumber(0))
            {
                api.ArgTypeError(0, luna::ValueT_Number);
                return 0;
            }
            if (!api.IsNumber(1))
            {
                api.ArgTypeError(1, luna::ValueT_Number);
                return 0;
            }

            auto min = static_cast<long long>(api.GetNumber(0));
            auto max = static_cast<long long>(api.GetNumber(1));

            RandEngine engine;
            std::uniform_int_distribution<long long> dis(min, max);
            api.PushNumber(static_cast<double>(dis(engine)));
        }

        return 1;
    }

    int RandomSeed(luna::State *state)
    {
        luna::StackAPI api(state);
        int params = api.GetStackSize();
        if (params < 1)
        {
            api.ArgCountError(1);
            return 0;
        }

        if (!api.IsNumber(0))
        {
            api.ArgTypeError(0, luna::ValueT_Number);
            return 0;
        }

        srand(static_cast<unsigned int>(api.GetNumber(0)));
        return 0;
    }

    void RegisterLibMath(luna::State *state)
    {
        luna::Library lib(state);
        luna::TableFuncReg math[] = {
            { "random", Random },
            { "randomseed", RandomSeed }
        };

        lib.RegisterTableFunction("math", math);
    }

} // namespace math
} // namespace lib
