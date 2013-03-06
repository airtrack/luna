#include "Visitor.h"
#include "State.h"
#include "Function.h"
#include <vector>

namespace luna
{
    class ScopeNameLevel;

    struct ScopeNameList
    {
        std::vector<String *> name_list_;
        ScopeNameLevel *current_level_;

        ScopeNameList() : current_level_(nullptr) { }
    };

    class ScopeNameLevel
    {
    public:
        explicit ScopeNameLevel(ScopeNameList &name_list)
            : name_list_(&name_list),
              previous_(name_list.current_level_),
              start_(name_list.name_list_.size())
        {
            name_list_->current_level_ = this;
        }

        ~ScopeNameLevel()
        {
            name_list_->name_list_.resize(start_);
            name_list_->current_level_ = previous_;
        }

        // Get current scope level name by current name level index
        String * GetScopeName(std::size_t index) const
        {
            if (start_ + index < name_list_->name_list_.size())
                return name_list_->name_list_[start_ + index];
            return nullptr;
        }

        // Add name to scope
        void AddScopeName(String *name)
        {
            name_list_->name_list_.push_back(name);
        }

    private:
        // scope name list
        ScopeNameList *name_list_;

        // previous scope
        ScopeNameLevel *previous_;

        // start index in name_list_
        std::size_t start_;
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
        // Visit all statements
        for (auto &s : block->statements_)
            s->Accept(this);

        // Visit return statement when existed
        if (block->return_stmt_)
            block->return_stmt_->Accept(this);
    }

    std::unique_ptr<Visitor> GenerateVisitor(State *state)
    {
        return std::unique_ptr<Visitor>(new CodeGenerateVisitor(state));
    }
} // namespace luna
