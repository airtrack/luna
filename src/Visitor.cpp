#include "Visitor.h"
#include "State.h"
#include "Function.h"
#include <vector>
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

        // Get current scope level name by current name level index
        const ScopeName * GetScopeName(std::size_t index) const
        {
            if (start_ + index < name_list_->name_list_.size())
                return &name_list_->name_list_[start_ + index];
            return nullptr;
        }

        // Add name to scope if the name is not existed before
        bool AddScopeName(String *name, int reg)
        {
            if (!IsBelongsToScope(name))
            {
                name_list_->name_list_.push_back(ScopeName(name, reg));
                return true;
            }

            return false;
        }

        // Get previous ScopeNameLevel
        NameScope * GetPrevious() const
        {
            return previous_;
        }

        // Is name in this scope
        bool IsBelongsToScope(const String *name) const
        {
            std::size_t end = name_list_->name_list_.size();
            for (std::size_t i = start_; i < end; ++i)
                if (name_list_->name_list_[i].name_ == name)
                    return true;
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
        ScopeNameList scope_name_list_;

        // current function
        Function *func_;
    };

    CodeGenerateVisitor::CodeGenerateVisitor(State *state)
        : state_(state),
          func_(nullptr)
    {
    }

    void CodeGenerateVisitor::Visit(Chunk *chunk)
    {
        auto func = state_->NewFunction();
        func->SetBaseInfo(chunk->module_, 0);
        func->SetSuperior(func_);
        func_ = func;

        chunk->block_->Accept(this);
    }

    void CodeGenerateVisitor::Visit(Block *block)
    {
        NameScope current(scope_name_list_, func_);

        // Visit all statements
        for (auto &s : block->statements_)
            s->Accept(this);

        // Visit return statement when existed
        if (block->return_stmt_)
            block->return_stmt_->Accept(this);
    }

    void CodeGenerateVisitor::Visit(LocalNameListStatement *local_name)
    {
        // Visit local names
        local_name->name_list_->Accept(this);

        // Visit exp list
        if (local_name->exp_list_)
            local_name->exp_list_->Accept(this);
    }

    void CodeGenerateVisitor::Visit(Terminator *term)
    {
    }

    void CodeGenerateVisitor::Visit(NameList *name_list)
    {
        // Add all names to local scope
        for (auto &n : name_list->names_)
        {
            assert(n.token_ == Token_Id);
            if (scope_name_list_.current_scope_->AddScopeName(
                    n.str_, func_->GetNextRegister()))
                func_->AllocaNextRegister();
        }
    }

    void CodeGenerateVisitor::Visit(ExpressionList *exp_list)
    {
        // Visit each expression
        for (auto &exp : exp_list->exp_list_)
            exp->Accept(this);
    }

    std::unique_ptr<Visitor> GenerateVisitor(State *state)
    {
        return std::unique_ptr<Visitor>(new CodeGenerateVisitor(state));
    }
} // namespace luna
