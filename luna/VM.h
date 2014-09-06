#ifndef VM_H
#define VM_H

#include "Value.h"
#include "OpCode.h"
#include <utility>

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

        void GenerateClosure(Value *a, Instruction i);
        void CopyVarArg(Value *a, Instruction i);
        void Return(Value *a, Instruction i);

        void Concat(Value *dst, Value *op1, Value *op2);
        void ForInit(Value *var, Value *limit, Value *step);

        // Debug help functions
        std::pair<const char *, const char *>
        GetOperandNameAndScope(const Value *a) const;

        std::pair<const char *, int> GetCurrentInstructionPos() const;

        void CheckType(const Value *v, ValueT type, const char *op) const;

        void CheckArithType(const Value *v1, const Value *v2,
                            const char *op) const;

        void CheckInequalityType(const Value *v1, const Value *v2,
                                 const char *op) const;

        void CheckTableType(const Value *t, const Value *k,
                            const char *op, const char *desc) const;

        void ReportTypeError(const Value *v, const char *op) const;

        State *state_;
    };
} // namespace luna

#endif // VM_H
