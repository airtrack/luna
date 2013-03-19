#ifndef VM_H
#define VM_H

#include "Value.h"

namespace luna
{
    class State;

    class VM
    {
    public:
        explicit VM(State *state);

        void Execute();

    private:
        void ExecuteFrame();
        void CallClosure(Value *a);
        void CallCFunction(Value *a);

        State *state_;
    };
} // namespace luna

#endif // VM_H
