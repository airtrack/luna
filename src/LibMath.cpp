#include "LibMath.h"
#include <random>
#include <cmath>
#include <cstdlib>

namespace lib {
namespace math {

    int Abs(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_Number))
            return 0;

        api.PushNumber(std::abs(api.GetNumber(0)));
        return 1;
    }

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
        result_type operator() () { return std::rand(); }

        RandEngine() { }
        RandEngine(const RandEngine&) = delete;
        void operator = (const RandEngine&) = delete;
    };

    int Random(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(0, luna::ValueT_Number, luna::ValueT_Number))
            return 0;

        int params = api.GetStackSize();
        if (params == 0)
        {
            RandEngine engine;
            std::uniform_real_distribution<> dis;
            api.PushNumber(dis(engine));
        }
        else if (params == 1)
        {
            auto max = static_cast<unsigned long long>(api.GetNumber(0));

            RandEngine engine;
            std::uniform_int_distribution<unsigned long long> dis(1, max);
            api.PushNumber(static_cast<double>(dis(engine)));
        }
        else if (params >= 2)
        {
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
        if (!api.CheckArgs(1, luna::ValueT_Number))
            return 0;

        std::srand(static_cast<unsigned int>(api.GetNumber(0)));
        return 0;
    }

    void RegisterLibMath(luna::State *state)
    {
        luna::Library lib(state);
        luna::TableFuncReg math[] = {
            { "abs", Abs },
            { "random", Random },
            { "randomseed", RandomSeed }
        };

        lib.RegisterTableFunction("math", math);
    }

} // namespace math
} // namespace lib
