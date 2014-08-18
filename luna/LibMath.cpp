#include "LibMath.h"
#include <random>
#include <cmath>
#include <cstdlib>

namespace lib {
namespace math {

// Define one parameter one return value math function
#define MATH_FUNCTION(name, std_name)                       \
    int name(luna::State *state)                            \
    {                                                       \
        luna::StackAPI api(state);                          \
        if (!api.CheckArgs(1, luna::ValueT_Number))         \
            return 0;                                       \
        api.PushNumber(std::std_name(api.GetNumber(0)));    \
        return 1;                                           \
    }

// Define two parameters one return value math function
#define MATH_FUNCTION2(name, std_name)                      \
    int name(luna::State *state)                            \
    {                                                       \
        luna::StackAPI api(state);                          \
        if (!api.CheckArgs(2, luna::ValueT_Number,          \
                           luna::ValueT_Number))            \
            return 0;                                       \
        api.PushNumber(std::std_name(api.GetNumber(0),      \
                                     api.GetNumber(1)));    \
        return 1;                                           \
    }

    MATH_FUNCTION(Abs, abs)
    MATH_FUNCTION(Acos, acos)
    MATH_FUNCTION(Asin, asin)
    MATH_FUNCTION(Atan, atan)
    MATH_FUNCTION(Ceil, ceil)
    MATH_FUNCTION(Cos, cos)
    MATH_FUNCTION(Cosh, cosh)
    MATH_FUNCTION(Exp, exp)
    MATH_FUNCTION(Floor, floor)
    MATH_FUNCTION(Sin, sin)
    MATH_FUNCTION(Sinh, sinh)
    MATH_FUNCTION(Sqrt, sqrt)
    MATH_FUNCTION(Tan, tan)
    MATH_FUNCTION(Tanh, tanh)

    MATH_FUNCTION2(Atan2, atan2)
    MATH_FUNCTION2(Fmod, fmod)
    MATH_FUNCTION2(Ldexp, ldexp)
    MATH_FUNCTION2(Pow, pow)

    int Deg(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_Number))
            return 0;

        api.PushNumber(api.GetNumber(0) / M_PI * 180);
        return 1;
    }

    int Rad(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_Number))
            return 0;

        api.PushNumber(api.GetNumber(0) / 180 * M_PI);
        return 1;
    }

    int Log(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_Number, luna::ValueT_Number))
            return 0;

        auto l = std::log(api.GetNumber(0));
        if (api.GetStackSize() > 1)
        {
            auto b = std::log(api.GetNumber(1));
            l /= b;
        }

        api.PushNumber(l);
        return 1;
    }

    int Min(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_Number))
            return 0;

        auto min = api.GetNumber(0);
        auto params = api.GetStackSize();
        for (int i = 1; i < params; ++i)
        {
            if (!api.IsNumber(i))
            {
                api.ArgTypeError(i, luna::ValueT_Number);
                return 0;
            }

            auto n = api.GetNumber(i);
            if (n < min) min = n;
        }

        api.PushNumber(min);
        return 1;
    }

    int Max(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_Number))
            return 0;

        auto max = api.GetNumber(0);
        auto params = api.GetStackSize();
        for (int i = 1; i < params; ++i)
        {
            if (!api.IsNumber(i))
            {
                api.ArgTypeError(i, luna::ValueT_Number);
                return 0;
            }

            auto n = api.GetNumber(i);
            if (n > max) max = n;
        }

        api.PushNumber(max);
        return 1;
    }

    int Frexp(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_Number))
            return 0;

        int exp = 0;
        auto m = std::frexp(api.GetNumber(0), &exp);
        api.PushNumber(m);
        api.PushNumber(exp);
        return 2;
    }

    int Modf(luna::State *state)
    {
        luna::StackAPI api(state);
        if (!api.CheckArgs(1, luna::ValueT_Number))
            return 0;

        double ipart = 0.0;
        auto fpart = std::modf(api.GetNumber(0), &ipart);
        api.PushNumber(ipart);
        api.PushNumber(fpart);
        return 2;
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

        auto params = api.GetStackSize();
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
        luna::TableMemberReg math[] = {
            { "abs", Abs },
            { "acos", Acos },
            { "asin", Asin },
            { "atan", Atan },
            { "atan2", Atan2 },
            { "ceil", Ceil },
            { "cos", Cos },
            { "cosh", Cosh },
            { "deg", Deg },
            { "exp", Exp },
            { "floor", Floor },
            { "fmod", Fmod },
            { "frexp", Frexp },
            { "ldexp", Ldexp },
            { "log", Log },
            { "max", Max },
            { "min", Min },
            { "modf", Modf },
            { "pow", Pow },
            { "rad", Rad },
            { "random", Random },
            { "randomseed", RandomSeed },
            { "sin", Sin },
            { "sinh", Sinh },
            { "sqrt", Sqrt },
            { "tan", Tan },
            { "tanh", Tanh },
            { "huge", HUGE_VAL },
            { "pi", M_PI }
        };

        lib.RegisterTableFunction("math", math);
    }

} // namespace math
} // namespace lib
