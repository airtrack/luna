#include "Visitor.h"
#include "State.h"
#include "Function.h"
#include <vector>

namespace luna
{
    class ScopeNameLevel;
    class ScopeNameManager;

    class ScopeNameLevel
    {
    public:
        explicit ScopeNameLevel(ScopeNameManager &manager);
        ~ScopeNameLevel();

        // Get current scope level name by current name level index
        String * GetScopeName(std::size_t index) const;

        // Add name to scope
        void AddScopeName(String *name);

    private:
        // level owner manager
        ScopeNameManager *manager_;

        // previous scope
        ScopeNameLevel *previous_;

        // start index in manager_
        std::size_t start_;
    };

    class ScopeNameManager
    {
        friend class ScopeNameLevel;
    public:
        ScopeNameManager() : current_level_(nullptr) { }

        ScopeNameManager(const ScopeNameManager&) = delete;
        void operator = (const ScopeNameManager&) = delete;

    private:
        // Set and get current scope level
        ScopeNameLevel * GetCurrentLevel() const;
        void SetCurrentLevel(ScopeNameLevel *level);

        // Get name by index, ignore scope name level
        String * GetName(std::size_t index) const;

        // Get name count
        std::size_t GetNameCount() const;

        // Add name
        void AddName(String *name);

        std::vector<String *> name_list_;
        ScopeNameLevel *current_level_;
    };

    ScopeNameLevel::ScopeNameLevel(ScopeNameManager &manager)
        : manager_(&manager), previous_(nullptr), start_(0)
    {
        previous_ = manager_->GetCurrentLevel();
        start_ = manager_->GetNameCount();
        manager_->SetCurrentLevel(this);
    }

    ScopeNameLevel::~ScopeNameLevel()
    {
        manager_->SetCurrentLevel(previous_);
    }

    String * ScopeNameLevel::GetScopeName(std::size_t index) const
    {
        return manager_->GetName(start_ + index);
    }

    void ScopeNameLevel::AddScopeName(String *name)
    {
        manager_->AddName(name);
    }

    ScopeNameLevel * ScopeNameManager::GetCurrentLevel() const
    {
        return current_level_;
    }

    void ScopeNameManager::SetCurrentLevel(ScopeNameLevel *level)
    {
        current_level_ = level;
    }

    String * ScopeNameManager::GetName(std::size_t index) const
    {
        if (index < name_list_.size())
            return name_list_[index];
        return nullptr;
    }

    std::size_t ScopeNameManager::GetNameCount() const
    {
        return name_list_.size();
    }

    void ScopeNameManager::AddName(String *name)
    {
        name_list_.push_back(name);
    }

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
