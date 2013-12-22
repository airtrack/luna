#include "CodeGenerate.h"
#include "State.h"
#include "Function.h"
#include <vector>
#include <stack>
#include <utility>
#include <assert.h>

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

        // Get name register, if 'name' is not in this scope, then return -1
        int GetNameRegister(const String *name) const
        {
            int reg = -1;
            IsBelongsToScope(name, &reg);
            return reg;
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

        // Get scope owner function
        Function * GetOwnerFunction() const
        {
            return owner_;
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
            if (exp_value_count_.empty())
                return 0;

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
            if (exp_list_value_count_.empty())
                return 0;

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
            return func_states_.back().get();
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

        virtual void Visit(Chunk *, void *);
        virtual void Visit(Block *, void *);
        virtual void Visit(ReturnStatement *, void *);
        virtual void Visit(BreakStatement *, void *);
        virtual void Visit(DoStatement *, void *);
        virtual void Visit(WhileStatement *, void *);
        virtual void Visit(RepeatStatement *, void *);
        virtual void Visit(IfStatement *, void *);
        virtual void Visit(ElseIfStatement *, void *);
        virtual void Visit(ElseStatement *, void *);
        virtual void Visit(NumericForStatement *, void *);
        virtual void Visit(GenericForStatement *, void *);
        virtual void Visit(FunctionStatement *, void *);
        virtual void Visit(FunctionName *, void *);
        virtual void Visit(LocalFunctionStatement *, void *);
        virtual void Visit(LocalNameListStatement *, void *);
        virtual void Visit(AssignmentStatement *, void *);
        virtual void Visit(VarList *, void *);
        virtual void Visit(Terminator *, void *);
        virtual void Visit(BinaryExpression *, void *);
        virtual void Visit(UnaryExpression *, void *);
        virtual void Visit(FunctionBody *, void *);
        virtual void Visit(ParamList *, void *);
        virtual void Visit(NameList *, void *);
        virtual void Visit(TableDefine *, void *);
        virtual void Visit(TableIndexField *, void *);
        virtual void Visit(TableNameField *, void *);
        virtual void Visit(TableArrayField *, void *);
        virtual void Visit(IndexAccessor *, void *);
        virtual void Visit(MemberAccessor *, void *);
        virtual void Visit(NormalFuncCall *, void *);
        virtual void Visit(MemberFuncCall *, void *);
        virtual void Visit(FuncCallArgs *, void *);
        virtual void Visit(ExpressionList *, void *);

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

    void CodeGenerateVisitor::Visit(Chunk *chunk, void *data)
    {
        // Generate function
        auto func = state_->NewFunction();
        func->SetBaseInfo(chunk->module_, 0);
        func->SetSuperior(func_);
        func_ = func;

        func_state_ = gen_state_.PushFunctionState();

        chunk->block_->Accept(this, data);

        // Generate closure
        auto cl = state_->NewClosure();
        cl->SetPrototype(func);

        // Add closure to stack
        state_->stack_.top_->closure_ = cl;
        state_->stack_.top_->type_ = ValueT_Closure;
        state_->stack_.top_++;
    }

    void CodeGenerateVisitor::Visit(Block *block, void *data)
    {
        NameScope current(scope_name_list_, func_);
        int reg = func_->GetNextRegister();

        // Visit all statements
        for (auto &s : block->statements_)
            s->Accept(this, data);

        // Visit return statement if exist
        if (block->return_stmt_)
            block->return_stmt_->Accept(this, data);

        // Restore register
        func_->SetNextRegister(reg);
        func_->AddInstruction(Instruction::ACode(OpType_SetTop, reg), 0);
    }

    void CodeGenerateVisitor::Visit(ReturnStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(BreakStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(DoStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(WhileStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(RepeatStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(IfStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(ElseIfStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(ElseStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(NumericForStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(GenericForStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(FunctionStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(FunctionName *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(LocalFunctionStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(LocalNameListStatement *local_name, void *data)
    {
        // Visit local names
        local_name->name_list_->Accept(this, data);

        int reg = func_->GetNextRegister();
        int names = func_state_->names_register_.size();

        // Visit exp list
        if (local_name->exp_list_)
        {
            func_state_->PushExpListValueCount(names);
            local_name->exp_list_->Accept(this, data);
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

    void CodeGenerateVisitor::Visit(AssignmentStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(VarList *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(Terminator *term, void *data)
    {
        const TokenDetail &t = term->token_;
        int value_count = func_state_->PopExpValueCount();

        if (t.token_ == Token_Number ||
            t.token_ == Token_String)
        {
            int index = 0;
            if (t.token_ == Token_Number)
                index = func_->AddConstNumber(t.number_);
            else
                index = func_->AddConstString(t.str_);

            if (value_count != 0)
            {
                int reg = func_->AllocaNextRegister();
                func_->AddInstruction(Instruction::ABCode(OpType_LoadConst,
                                                          reg, index), t.line_);
            }
        }
        else if (t.token_ == Token_Id)
        {
            auto s = scope_name_list_.current_scope_->GetBlongsToScope(t.str_);

            // Token_Id not in scope, then this id is in env table
            if (!s.first)
            {
                int index = func_->AddConstString(t.str_);
                if (value_count != 0)
                {
                    int reg = func_->AllocaNextRegister();
                    // Load key
                    func_->AddInstruction(Instruction::ABCode(OpType_LoadConst,
                                                              reg, index), t.line_);
                    // Get value from uptable
                    func_->AddInstruction(Instruction::ABCCode(OpType_GetUpTable,
                                                               reg, ENV_TABLE_INDEX, reg), t.line_);
                }
            }
            else
            {
                if (s.second == scope_name_list_.current_scope_->GetOwnerFunction())
                {
                    int src_reg = s.first->GetNameRegister(t.str_);
                    assert(src_reg >= 0);
                    if (value_count != 0)
                    {
                        int dst_reg = func_->AllocaNextRegister();
                        func_->AddInstruction(Instruction::ABCode(OpType_Move, dst_reg, src_reg), t.line_);
                    }
                }
                else
                {
                    assert(!"TODO: generate code for upvalue.");
                }
            }
        }
        else
            assert(!"maybe miss some term type.");
    }

    void CodeGenerateVisitor::Visit(BinaryExpression *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(UnaryExpression *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(FunctionBody *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(ParamList *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(NameList *name_list, void *data)
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

    void CodeGenerateVisitor::Visit(TableDefine *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(TableIndexField *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(TableNameField *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(TableArrayField *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(IndexAccessor *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(MemberAccessor *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(NormalFuncCall *func_call, void *data)
    {
        int reg = func_->GetNextRegister();
        int result_count = func_state_->PopExpValueCount();

        // Load function
        func_state_->PushExpValueCount(1);
        func_call->caller_->Accept(this, data);

        // Prepare args
        func_call->args_->Accept(this, data);

        func_->AddInstruction(Instruction::AsBxCode(OpType_Call, reg, result_count), 0);
    }

    void CodeGenerateVisitor::Visit(MemberFuncCall *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(FuncCallArgs *arg, void *data)
    {
        if (arg->type_ == FuncCallArgs::String ||
            arg->type_ == FuncCallArgs::Table)
        {
            func_state_->PushExpValueCount(1);
            arg->arg_->Accept(this, data);
        }
        else
        {
            // FuncCallArgs::ExpList
            assert(arg->type_ == FuncCallArgs::ExpList);
            if (arg->arg_)
            {
                func_state_->PushExpListValueCount(EXP_VALUE_COUNT_ANY);
                arg->arg_->Accept(this, data);
            }
        }
    }

    void CodeGenerateVisitor::Visit(ExpressionList *exp_list, void *data)
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

            exp->Accept(this, data);
        }
    }

    std::unique_ptr<Visitor> GenerateVisitor(State *state)
    {
        return std::unique_ptr<Visitor>(new CodeGenerateVisitor(state));
    }
} // namespace luna
