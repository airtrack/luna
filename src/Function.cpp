#include "Function.h"

namespace luna
{
    Function::Function()
        : module_(nullptr), line_(0), args_(0), is_vararg_(false),
          reg_allocator_(0), reg_count_(0), superior_(nullptr)
    {
    }

    void Function::Accept(GCObjectVisitor *v)
    {
        if (v->Visit(this))
        {
            if (module_)
                module_->Accept(v);
            if (superior_)
                superior_->Accept(v);

            for (const auto &value : const_values_)
                value.Accept(v);
        }
    }

    const Instruction * Function::GetOpCodes() const
    {
        return opcodes_.empty() ? nullptr : &opcodes_[0];
    }

    std::size_t Function::OpCodeSize() const
    {
        return opcodes_.size();
    }

    void Function::AddInstruction(Instruction i, int line)
    {
        opcodes_.push_back(i);
        opcode_lines_.push_back(line);
    }

    void Function::SetHasVararg()
    {
        is_vararg_ = true;
    }

    bool Function::HasVararg() const
    {
        return is_vararg_;
    }

    void Function::SetFixedArgCount(int count)
    {
        args_ = count;
    }

    int Function::FixedArgCount() const
    {
        return args_;
    }

    void Function::SetBaseInfo(String *module, int line)
    {
        module_ = module;
        line_ = line;
    }

    void Function::SetSuperior(Function *superior)
    {
        superior_ = superior;
    }

    int Function::GetNextRegister() const
    {
        return reg_allocator_;
    }

    int Function::AllocaNextRegister()
    {
        int reg = reg_allocator_++;

        // Adjust register count
        if (reg_allocator_ > reg_count_)
            reg_count_ = reg_allocator_;

        return reg;
    }

    void Function::SetNextRegister(int reg)
    {
        reg_allocator_ = reg;
        if (reg_allocator_ > reg_count_)
            reg_count_ = reg_allocator_;
    }

    int Function::GetRegisterCount() const
    {
        return reg_count_;
    }

    void Function::SetRegisterCount(int count)
    {
        if (count > reg_count_)
            reg_count_ = count;
    }

    int Function::GetFreeRegisterCount() const
    {
        return kMaxRegisterCount - reg_allocator_;
    }

    int Function::AddConstNumber(double num)
    {
        Value v;
        v.type_ = ValueT_Number;
        v.num_ = num;
        return AddConstValue(v);
    }

    int Function::AddConstString(String *str)
    {
        Value v;
        v.type_ = ValueT_String;
        v.str_ = str;
        return AddConstValue(v);
    }

    int Function::AddConstValue(const Value &v)
    {
        const_values_.push_back(v);
        return const_values_.size() - 1;
    }

    Value * Function::GetConstValue(int i)
    {
        return &const_values_[i];
    }

    int Function::GetInstructionLine(int i) const
    {
        return opcode_lines_[i];
    }

    Closure::Closure()
        : prototype_(nullptr)
    {
    }

    void Closure::Accept(GCObjectVisitor *v)
    {
        if (v->Visit(this))
        {
            prototype_->Accept(v);

            for (const auto &upvalue : upvalues_)
                upvalue->Accept(v);
        }
    }

    Function * Closure::GetPrototype() const
    {
        return prototype_;
    }

    void Closure::SetPrototype(Function *prototype)
    {
        prototype_ = prototype;
    }

    Value * Closure::GetUpvalue(std::size_t index) const
    {
        int size = upvalues_.size();
        if (index < size)
        {
            auto upvalue = upvalues_[index];
            return upvalue->GetValue();
        }

        return nullptr;
    }
} // namespace luna
