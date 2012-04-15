#include "NativeFunctionStackOperator.h"
#include "State.h"
#include <assert.h>

namespace lua
{
    NativeFunctionStackOperator::NativeFunctionStackOperator(State *state)
        : state_(state),
          input_(0),
          pushed_(0)
    {
        PopInputParams();
        PopCaller();
    }

    NativeFunctionStackOperator::~NativeFunctionStackOperator()
    {
        PushReturnValueCounter();
    }

    std::size_t NativeFunctionStackOperator::GetSize() const
    {
        return input_.Size();
    }

    const StackValue * NativeFunctionStackOperator::GetStackValue(int index) const
    {
        return input_.GetStackValue(index);
    }

    bool NativeFunctionStackOperator::IsNil(int index) const
    {
        return IsType(index, TYPE_NIL);
    }

    bool NativeFunctionStackOperator::IsBool(int index) const
    {
        return IsType(index, TYPE_BOOL);
    }

    bool NativeFunctionStackOperator::IsNumber(int index) const
    {
        return IsType(index, TYPE_NUMBER);
    }

    bool NativeFunctionStackOperator::IsString(int index) const
    {
        return IsType(index, TYPE_STRING);
    }

    bool NativeFunctionStackOperator::IsTable(int index) const
    {
        return IsType(index, TYPE_TABLE);
    }

    bool NativeFunctionStackOperator::ToBool(int index) const
    {
        const StackValue *sv = input_.GetStackValue(index);
        if (!sv || sv->type != StackValueType_Value)
            return false;
        if (sv->param.value->Type() != TYPE_BOOL)
            return false;
        return static_cast<const Bool *>(sv->param.value)->Get();
    }

    double NativeFunctionStackOperator::ToNumber(int index) const
    {
        const StackValue *sv = input_.GetStackValue(index);
        if (!sv || sv->type != StackValueType_Value)
            return 0;
        if (sv->param.value->Type() != TYPE_NUMBER)
            return 0;
        return static_cast<const Number *>(sv->param.value)->Get();
    }

    std::string NativeFunctionStackOperator::ToString(int index) const
    {
        const StackValue *sv = input_.GetStackValue(index);
        if (!sv || sv->type != StackValueType_Value)
            return "";
        if (sv->param.value->Type() != TYPE_STRING)
            return "";
        return static_cast<const String *>(sv->param.value)->Get();
    }

    void NativeFunctionStackOperator::PushNil()
    {
        state_->GetStack()->Push(state_->GetDataPool()->GetNil());
        ++pushed_;
    }

    void NativeFunctionStackOperator::PushBool(bool b)
    {
        Bool *v = b
            ? state_->GetDataPool()->GetTrue()
            : state_->GetDataPool()->GetFalse();
        state_->GetStack()->Push(v);
        ++pushed_;
    }

    void NativeFunctionStackOperator::PushNumber(double number)
    {
        state_->GetStack()->Push(state_->GetDataPool()->GetNumber(number));
        ++pushed_;
    }

    void NativeFunctionStackOperator::PushString(const std::string& str)
    {
        state_->GetStack()->Push(state_->GetDataPool()->GetString(str));
        ++pushed_;
    }

    void NativeFunctionStackOperator::PushTable()
    {
        state_->GetStack()->Push(state_->GetDataPool()->GetTable());
        ++pushed_;
    }

    bool NativeFunctionStackOperator::IsType(int index, TypeId type) const
    {
        const StackValue *sv = input_.GetStackValue(index);
        if (!sv)
            return false;
        return sv->type == StackValueType_Value && sv->param.value->Type() == type;
    }

    void NativeFunctionStackOperator::PopInputParams()
    {
        Stack *stack = state_->GetStack();
        StackValue *sv = stack->Top();
        assert(sv && sv->type == StackValueType_Counter);
        int total = sv->param.counter.total;
        stack->Pop();

        for (int index = -total; index < 0; ++index)
        {
            StackValue *v = stack->GetStackValue(index);
            assert(v && v->type == StackValueType_Value);
            input_.Push(v->param.value);
        }

        if (total > 0)
            stack->Pop(total);
    }

    void NativeFunctionStackOperator::PopCaller()
    {
        Stack *stack = state_->GetStack();
        StackValue *sv = stack->Top();
        assert(sv && sv->type == StackValueType_Counter);
        int total = sv->param.counter.total;

        stack->Pop(total + 1);
    }

    void NativeFunctionStackOperator::PushReturnValueCounter()
    {
        state_->GetStack()->Push(pushed_, 0);
    }
} // namespace lua
