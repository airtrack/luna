#include "Function.h"
#include <limits>

namespace luna
{
    Function::Function()
        : module_(nullptr), line_(0), args_(0),
          is_vararg_(false), superior_(nullptr)
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

            for (const auto &var : local_vars_)
                var.name_->Accept(v);

            for (auto child : child_funcs_)
                child->Accept(v);

            for (const auto &upvalue : upvalues_)
                upvalue.name_->Accept(v);
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

    Instruction * Function::GetMutableInstruction(std::size_t index)
    {
        return &opcodes_[index];
    }

    std::size_t Function::AddInstruction(Instruction i, int line)
    {
        opcodes_.push_back(i);
        opcode_lines_.push_back(line);
        return opcodes_.size() - 1;
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

    void Function::SetModuleName(String *module)
    {
        module_ = module;
    }

    void Function::SetLine(int line)
    {
        line_ = line;
    }

    void Function::SetSuperior(Function *superior)
    {
        superior_ = superior;
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

    void Function::AddLocalVar(String *name, int register_id,
                               int begin_pc, int end_pc)
    {
        local_vars_.push_back(LocalVarInfo(name, register_id, begin_pc, end_pc));
    }

    int Function::AddChildFunction(Function *child)
    {
        child_funcs_.push_back(child);
        return child_funcs_.size() - 1;
    }

    int Function::AddUpvalue(String *name, bool parent_local, int register_index)
    {
        upvalues_.push_back(UpvalueInfo(name, parent_local, register_index));
        return upvalues_.size() - 1;
    }

    int Function::SearchUpvalue(String *name) const
    {
        int size = upvalues_.size();
        for (int i = 0; i < size; ++i)
        {
            if (upvalues_[i].name_ == name)
                return i;
        }

        return -1;
    }

    Function * Function::GetChildFunction(int index) const
    {
        return child_funcs_[index];
    }

    String * Function::SearchLocalVar(int register_id, int pc) const
    {
        String *name = nullptr;
        int begin_pc = std::numeric_limits<int>::min();
        int end_pc = std::numeric_limits<int>::max();

        for (const auto &var : local_vars_)
        {
            if (var.register_id_ == register_id &&
                var.begin_pc_ <= pc && pc < var.end_pc_)
            {
                if (var.begin_pc_ >= begin_pc && var.end_pc_ <= end_pc)
                {
                    name = var.name_;
                    begin_pc = var.begin_pc_;
                    end_pc = var.end_pc_;
                }
            }
        }

        return name;
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
} // namespace luna
