#include "Visitor.h"
#include "State.h"
#include "Function.h"
#include <vector>
#include <stack>
#include <utility>
#include <assert.h>

#define EXP_VALUE_COUNT_ANY -1

namespace luna
{
    class NameScope;

    struct ScopeName
    {
        // name
        String *name_;
        // register index in function
        int register_;

        ScopeName() : name_(nullptr), register_(0) { }
        ScopeName(String *name, int reg) : name_(name), register_(reg) { }
    };

    struct ScopeNameList
    {
        std::vector<ScopeName> name_list_;
        NameScope *current_scope_;

        ScopeNameList() : current_scope_(nullptr) { }
    };

    class NameScope
    {
    public:
        explicit NameScope(ScopeNameList &name_list, Function *owner = nullptr)
            : name_list_(&name_list),
              previous_(name_list.current_scope_),
              start_(name_list.name_list_.size()),
              owner_(nullptr)
        {
            name_list_->current_scope_ = this;

            if (owner)
                owner_ = owner;
            else
                owner_ = previous_->owner_;
        }

        ~NameScope()
        {
            name_list_->name_list_.resize(start_);
            name_list_->current_scope_ = previous_;
        }

        // Get current scope level name by current name level index
        const ScopeName * GetScopeName(std::size_t index) const
        {
            if (start_ + index < name_list_->name_list_.size())
                return &name_list_->name_list_[start_ + index];
            return nullptr;
        }

        // Add name to scope if the name is not existed before,
        // '*reg' store the 'name' register if return false.
        bool AddScopeName(String *name, int *reg)
        {
            assert(reg);
            if (!IsBelongsToScope(name, reg))
            {
                name_list_->name_list_.push_back(ScopeName(name, *reg));
                return true;
            }

            return false;
        }

        // Get previous ScopeNameLevel
        NameScope * GetPrevious() const
        {
            return previous_;
        }

        // Is name in this scope, if 'name' exist, then '*reg' store the register
        bool IsBelongsToScope(const String *name, int *reg = nullptr) const
        {
            std::size_t end = name_list_->name_list_.size();
            for (std::size_t i = start_; i < end; ++i)
            {
                if (name_list_->name_list_[i].name_ == name)
                {
                    if (reg) *reg = name_list_->name_list_[i].register_;
                    return true;
                }
            }
            return false;
        }

        // Get the NameScope which the name belongs to
        std::pair<const NameScope *,
                  const Function *> GetBlongsToScope(const String *name) const
        {
            const NameScope *current = this;
            while (current)
            {
                if (current->IsBelongsToScope(name))
                    break;
                else
                    current = current->previous_;
            }

            const Function *func = current ? current->owner_ : nullptr;
            return std::make_pair(current, func);
        }

    private:
        // scope name list
        ScopeNameList *name_list_;

        // previous scope
        NameScope *previous_;

        // start index in name_list_
        std::size_t start_;

        // scope owner function
        Function *owner_;
    };

    // Local name and its register pair in function
    struct NameReg
    {
        int register_;
        const TokenDetail *token_;

        NameReg(int reg, const TokenDetail &t)
        : register_(reg), token_(&t) { }
    };

    // Code generate state for each function
    struct FunctionGenerateState
    {
        std::vector<NameReg> names_register_;
        std::stack<int> exp_value_count_;
        std::stack<int> exp_list_value_count_;

        void PushExpValueCount(int count)
        {
            exp_value_count_.push(count);
        }

        int PopExpValueCount()
        {
            int result = exp_value_count_.top();
            exp_value_count_.pop();
            return result;
        }

        void PushExpListValueCount(int count)
        {
            exp_list_value_count_.push(count);
        }

        int PopExpListValueCount()
        {
            int result = exp_list_value_count_.top();
            exp_list_value_count_.pop();
            return result;
        }
    };

    class GenerateState
    {
    public:
        GenerateState() { }
        GenerateState(const GenerateState&) = delete;
        void operator = (const GenerateState&) = delete;

        FunctionGenerateState * PushFunctionState()
        {
            std::unique_ptr<FunctionGenerateState> fgs(new FunctionGenerateState);
            func_states_.push_back(std::move(fgs));
            return fgs.get();
        }

        FunctionGenerateState * CurrentFunctionState()
        {
            if (func_states_.empty())
                return nullptr;
            else
                return func_states_.back().get();
        }

        void PopFunctionState()
        {
            func_states_.pop_back();
        }

    private:
        std::vector<std::unique_ptr<FunctionGenerateState>> func_states_;
    };

    class CodeGenerateVisitor : public Visitor
    {
    public:
        explicit CodeGenerateVisitor(State *state);

        virtual void Visit(Chunk *);
        virtual void Visit(Block *);
        virtual void Visit(ReturnStatement *);
        virtual void Visit(BreakStatement *);
        virtual void Visit(DoStatement *);
        virtual void Visit(WhileStatement *);
        virtual void Visit(RepeatStatement *);
        virtual void Visit(IfStatement *);
        virtual void Visit(ElseIfStatement *);
        virtual void Visit(ElseStatement *);
        virtual void Visit(NumericForStatement *);
        virtual void Visit(GenericForStatement *);
        virtual void Visit(FunctionStatement *);
        virtual void Visit(FunctionName *);
        virtual void Visit(LocalFunctionStatement *);
        virtual void Visit(LocalNameListStatement *);
        virtual void Visit(AssignmentStatement *);
        virtual void Visit(VarList *);
        virtual void Visit(Terminator *);
        virtual void Visit(BinaryExpression *);
        virtual void Visit(UnaryExpression *);
        virtual void Visit(FunctionBody *);
        virtual void Visit(ParamList *);
        virtual void Visit(NameList *);
        virtual void Visit(TableDefine *);
        virtual void Visit(TableIndexField *);
        virtual void Visit(TableNameField *);
        virtual void Visit(TableArrayField *);
        virtual void Visit(IndexAccessor *);
        virtual void Visit(MemberAccessor *);
        virtual void Visit(NormalFuncCall *);
        virtual void Visit(MemberFuncCall *);
        virtual void Visit(ExpressionList *);

    private:
        State *state_;

        // total name list
        ScopeNameList scope_name_list_;
        // generate state
        GenerateState gen_state_;

        // current function
        Function *func_;
        // current function generate state
        FunctionGenerateState *func_state_;
    };

    CodeGenerateVisitor::CodeGenerateVisitor(State *state)
        : state_(state),
          func_(nullptr),
          func_state_(nullptr)
    {
    }

    void CodeGenerateVisitor::Visit(Chunk *chunk)
    {
        auto func = state_->NewFunction();
        func->SetBaseInfo(chunk->module_, 0);
        func->SetSuperior(func_);
        func_ = func;

        func_state_ = gen_state_.PushFunctionState();

        chunk->block_->Accept(this);
    }

    void CodeGenerateVisitor::Visit(Block *block)
    {
        NameScope current(scope_name_list_, func_);
        int reg = func_->GetNextRegister();

        // Visit all statements
        for (auto &s : block->statements_)
            s->Accept(this);

        // Visit return statement if exist
        if (block->return_stmt_)
            block->return_stmt_->Accept(this);

        // Restore register
        func_->SetNextRegister(reg);
        func_->AddInstruction(Instruction::ACode(OpType_SetTop, reg), 0);
    }

    void CodeGenerateVisitor::Visit(LocalNameListStatement *local_name)
    {
        // Visit local names
        local_name->name_list_->Accept(this);

        int reg = func_->GetNextRegister();
        int names = func_state_->names_register_.size();

        // Visit exp list
        if (local_name->exp_list_)
        {
            func_state_->PushExpListValueCount(names);
            local_name->exp_list_->Accept(this);
        }

        // Set local name init value
        int exp_reg = reg;
        for (int i = 0; i < names; ++i, ++exp_reg)
        {
            int name_reg = func_state_->names_register_[i].register_;
            func_->AddInstruction(Instruction::ABCode(OpType_Move,
                                                      name_reg, exp_reg),
                                  func_state_->names_register_[i].token_->line_);
        }

        func_state_->names_register_.clear();

        // Restore register
        func_->SetNextRegister(reg);
        func_->AddInstruction(Instruction::ACode(OpType_SetTop, reg), 0);
    }

    void CodeGenerateVisitor::Visit(Terminator *term)
    {
        const TokenDetail &t = term->token_;
        int value_count = func_state_->PopExpValueCount();

        int index = 0;
        if (t.token_ == Token_Number)
            index = func_->AddConstNumber(t.number_);
        else if (t.token_ == Token_String)
            index = func_->AddConstString(t.str_);
        else
            assert(!"maybe miss some term type");

        if (value_count != 0)
        {
            int reg = func_->AllocaNextRegister();
            func_->AddInstruction(Instruction::ABCode(OpType_LoadConst,
                                                      reg, index), t.line_);
        }
    }

    void CodeGenerateVisitor::Visit(NameList *name_list)
    {
        // Add all names to local scope
        for (auto &n : name_list->names_)
        {
            assert(n.token_ == Token_Id);
            int reg = func_->GetNextRegister();
            if (scope_name_list_.current_scope_->AddScopeName(n.str_, &reg))
                func_->AllocaNextRegister();

            // Add name register, used by other Visit function to generate code
            func_state_->names_register_.push_back(NameReg(reg, n));
        }
    }

    void CodeGenerateVisitor::Visit(NormalFuncCall *func_call)
    {
        int reg = func_->GetNextRegister();

        // Load function
        func_state_->PushExpValueCount(1);
        func_call->caller_->Accept(this);

        // Prepare args
        if (func_call->args_)
            func_call->args_->Accept(this);

        func_->AddInstruction(Instruction::ACode(OpType_Call, reg), 0);
    }

    void CodeGenerateVisitor::Visit(ExpressionList *exp_list)
    {
        int value_count = func_state_->PopExpListValueCount();

        // Visit each expression
        int exp_count = exp_list->exp_list_.size();
        for (int i = 0; i < exp_count; ++i)
        {
            auto &exp = exp_list->exp_list_[i];

            if (value_count == 0)
            {
                func_state_->PushExpValueCount(0);
            }
            else
            {
                // Lastest exp set all remain value_count
                int count = i == exp_count - 1 ? value_count : 1;
                func_state_->PushExpValueCount(count);

                if (value_count != EXP_VALUE_COUNT_ANY)
                    value_count -= count;
            }

            exp->Accept(this);
        }
    }

    std::unique_ptr<Visitor> GenerateVisitor(State *state)
    {
        return std::unique_ptr<Visitor>(new CodeGenerateVisitor(state));
    }
} // namespace luna
