#ifndef VM_H
#define VM_H

#include "Value.h"
#include "OpCode.h"

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

        // Execute next frame if return true
        bool Call(Value *a, Instruction i);
        void CallClosure(Value *a, int expect_result);
        void CallCFunction(Value *a, int expect_result);

        int GetCurrentInstructionLine() const;

        State *state_;
    };
} // namespace luna

#endif // VM_H
