#ifndef NATIVE_FUNCTION_H
#define NATIVE_FUNCTION_H

#include "Value.h"
#include <functional>

namespace lua
{
    class NativeFunction : public Value
    {
    public:
        typedef std::function<void ()> FuncType;

        explicit NativeFunction(const FuncType& func)
            : func_(func)
        {
        }

        virtual int Type() const
        {
            return TYPE_NATIVE_FUNCTION;
        }

        virtual std::string Name() const
        {
            return "function";
        }

        virtual std::size_t GetHash() const;
        virtual bool IsEqual(const Value *other) const;

        void Call()
        {
            func_();
        }

    private:
        FuncType func_;
    };
} // namespace lua

#endif // NATIVE_FUNCTION_H
