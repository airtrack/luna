#include "CodeGenerate.h"
#include "State.h"
#include "Function.h"
#include "Guard.h"
#include <vector>
#include <stack>
#include <utility>
#include <unordered_map>
#include <assert.h>

namespace luna
{
    struct LocalNameInfo
    {
        // Name register id
        int register_id_;
        // Name as upvalue or not
        bool as_upvalue_;

        explicit LocalNameInfo(int register_id = 0, bool as_upvalue = false)
            : register_id_(register_id), as_upvalue_(as_upvalue) { }
    };

    // Lexical block struct for code generator
    struct GenerateBlock
    {
        GenerateBlock *parent_;
        // Current block register start id
        int register_start_id_;
        // Local names
        // Same names are the same instance String, so using String
        // pointer as key is fine
        std::unordered_map<String *, LocalNameInfo> names_;

        GenerateBlock() : parent_(nullptr), register_start_id_(0) { }
    };

    // Lexical function struct for code generator
    struct GenerateFunction
    {
        GenerateFunction *parent_;
        // Current block
        GenerateBlock *current_block_;
        // Current function for code generate
        Function *function_;
        // Register id generator
        int register_id_;
        // Max register count used in current function
        int register_max_;

        GenerateFunction()
            : parent_(nullptr), current_block_(nullptr),
              function_(nullptr), register_id_(0), register_max_(0) { }
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

        void EnterBlock()
        {
            auto block = new GenerateBlock;
            block->parent_ = current_function_->current_block_;
            block->register_start_id_ = current_function_->register_id_;
            current_function_->current_block_ = block;
        }

        void LeaveBlock()
        {
            auto block = current_function_->current_block_;
            current_function_->current_block_ = block->parent_;
            current_function_->register_id_ = block->register_start_id_;
            delete block;
        }

        void InsertName(String *name, int register_id, bool as_upvalue)
        {
            assert(current_function_ && current_function_->current_block_);
            current_function_->current_block_->names_.
                insert(std::make_pair(name, LocalNameInfo(register_id, as_upvalue)));
        }

        const LocalNameInfo * SearchName(String *name) const
        {
            return nullptr;
        }

        Function * GetCurrentFunction() const
        {
            return current_function_->function_;
        }

        int GenerateRegisterId()
        {
            int id = current_function_->register_id_++;
            if (current_function_->register_id_ > current_function_->register_max_)
                current_function_->register_max_ = current_function_->register_id_;
            return id;
        }

        // Get next register id, do not change register generator
        int GetNextRegisterId() const
        {
            return current_function_->register_id_;
        }

        // Reset register id generator, then next GenerateRegisterId
        // use the new id generator to generate register id
        void ResetRegisterIdGenerator(int generator)
        {
            current_function_->register_id_ = generator;
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

#define REGISTER_GENERATOR_GUARD()                                      \
    auto r = GetNextRegisterId();                                       \
    Guard g([]() { }, [=]() { this->ResetRegisterIdGenerator(r); })

    // For NameList AST
    struct NameListData
    {
        // NameList need init itself or not
        bool need_init_;

        explicit NameListData(bool need_init) : need_init_(need_init) { }
    };

    // For ExpList AST
    struct ExpListData
    {
        // ExpList need fill into range [start_register_, end_register_)
        int start_register_;
        int end_register_;

        ExpListData(int start_register, int end_register)
            : start_register_(start_register), end_register_(end_register) { }
    };

    // For expression and variable
    struct ExpVarData
    {
        // Need fill into range [start_register_, end_register_)
        int start_register_;
        int end_register_;

        ExpVarData(int start_register, int end_register)
            : start_register_(start_register), end_register_(end_register) { }
    };

    void CodeGenerateVisitor::Visit(Chunk *chunk, void *data)
    {
        CODE_GENERATE_GUARD(EnterFunction, LeaveFunction);
        {
            auto function = GetCurrentFunction();
            function->SetBaseInfo(chunk->module_, 0);

            CODE_GENERATE_GUARD(EnterBlock, LeaveBlock);
            chunk->block_->Accept(this, nullptr);
        }
    }

    void CodeGenerateVisitor::Visit(Block *block, void *data)
    {
        for (auto &stmt : block->statements_)
            stmt->Accept(this, nullptr);
        if (block->return_stmt_)
            block->return_stmt_->Accept(this, nullptr);
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

    void CodeGenerateVisitor::Visit(LocalNameListStatement *l_namelist_stmt, void *data)
    {
        auto start_register = GetNextRegisterId();

        // NameList need init itself when ExpList is not existed
        NameListData name_list_data{ !l_namelist_stmt->exp_list_ };
        l_namelist_stmt->name_list_->Accept(this, &name_list_data);

        auto end_register = GetNextRegisterId();

        if (l_namelist_stmt->exp_list_)
        {
            ExpListData exp_list_data{ start_register, end_register };
            l_namelist_stmt->exp_list_->Accept(this, &exp_list_data);
        }
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
        assert(name_list->names_.size() == name_list->names_ref_.size());
        auto need_init = static_cast<NameListData *>(data)->need_init_;

        auto size = name_list->names_.size();
        for (std::size_t i = 0; i < size; ++i)
        {
            auto register_id = GenerateRegisterId();
            auto as_upvalue = name_list->names_ref_[i].is_upvalue_;
            InsertName(name_list->names_[i].str_, register_id, as_upvalue);

            // Add init instructions when need
            if (need_init)
            {
                auto function = GetCurrentFunction();
                auto instruction = Instruction::ABCode(OpType_LoadNil, register_id,
                                                       as_upvalue ? 1 : 0);
                function->AddInstruction(instruction, name_list->names_[i].line_);
            }
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
    }

    void CodeGenerateVisitor::Visit(MemberFuncCall *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(FuncCallArgs *arg, void *data)
    {
    }

    void CodeGenerateVisitor::Visit(ExpressionList *exp_list, void *data)
    {
        auto exp_list_data = static_cast<ExpListData *>(data);
        auto register_id = exp_list_data->start_register_;
        auto end_register = exp_list_data->end_register_;

        assert(!exp_list->exp_list_.empty());
        int count = exp_list->exp_list_.size() - 1;

        // Each expression consume one register
        int i = 0;
        for (; i < count && register_id < end_register; ++i, ++register_id)
        {
            REGISTER_GENERATOR_GUARD();
            ExpVarData exp_var_data{ register_id, register_id + 1 };
            exp_list->exp_list_[i]->Accept(this, &exp_var_data);
        }

        // No more register
        for (; i < count; ++i)
        {
            REGISTER_GENERATOR_GUARD();
            ExpVarData exp_var_data{ 0, 0 };
            exp_list->exp_list_[i]->Accept(this, &exp_var_data);
        }

        // Last expression consume all remain registers
        REGISTER_GENERATOR_GUARD();
        ExpVarData exp_var_data{ register_id, end_register };
        exp_list->exp_list_.back()->Accept(this, &exp_var_data);
    }

    std::unique_ptr<Visitor> GenerateVisitor(State *state)
    {
        return std::unique_ptr<Visitor>(new CodeGenerateVisitor(state));
    }
} // namespace luna
