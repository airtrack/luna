#include "CodeGenerate.h"
#include "State.h"
#include "Function.h"
#include "Guard.h"
#include <vector>
#include <stack>
#include <utility>
#include <assert.h>

namespace luna
{
    // Lexical function struct for code generator
    struct GenerateFunction
    {
        GenerateFunction *parent_;
        // Current function for code generate
        Function *function_;

        GenerateFunction() : parent_(nullptr), function_(nullptr) { }
    };

    class CodeGenerateVisitor : public Visitor
    {
    public:
        explicit CodeGenerateVisitor(State *state)
            : state_(state), current_function_(nullptr) { }

        ~CodeGenerateVisitor()
        {
            while (current_function_)
            {
                DeleteCurrentFunction();
            }
        }

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

        void EnterFunction()
        {
            auto function = new GenerateFunction;
            function->parent_ = current_function_;
            current_function_ = function;
        }

        void LeaveFunction()
        {
            DeleteCurrentFunction();
        }

        Function * GetCurrentFunction() const
        {
            return current_function_->function_;
        }

    private:
        State *state_;

        void DeleteCurrentFunction()
        {
            auto function = current_function_;
            current_function_ = current_function_->parent_;
            delete function;
        }

        // Current code generating function
        GenerateFunction *current_function_;
    };

#define CODE_GENERATE_GUARD(enter, leave)                               \
    Guard g([this]() { this->enter(); }, [this]() { this->leave(); })

    void CodeGenerateVisitor::Visit(Chunk *chunk, void *data)
    {
        CODE_GENERATE_GUARD(EnterFunction, LeaveFunction);
    }

    void CodeGenerateVisitor::Visit(Block *block, void *data)
    {
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
    }

    void CodeGenerateVisitor::Visit(AssignmentStatement *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(VarList *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(Terminator *term, void *data)
    {
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
    }

    void CodeGenerateVisitor::Visit(MemberFuncCall *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(FuncCallArgs *arg, void *data)
    {
    }

    void CodeGenerateVisitor::Visit(ExpressionList *exp_list, void *data)
    {
    }

    std::unique_ptr<Visitor> GenerateVisitor(State *state)
    {
        return std::unique_ptr<Visitor>(new CodeGenerateVisitor(state));
    }
} // namespace luna
