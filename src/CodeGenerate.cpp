#include "CodeGenerate.h"
#include "State.h"
#include "Function.h"
#include "Guard.h"
#include <vector>
#include <stack>
#include <limits>
#include <utility>
#include <unordered_map>
#include <assert.h>

namespace luna
{
    struct LocalNameInfo
    {
        // Name register id
        int register_id_;
        // Name begin instruction
        int begin_pc_;
        // Name as upvalue or not
        bool as_upvalue_;

        explicit LocalNameInfo(int register_id = 0, int begin_pc = 0,
                               bool as_upvalue = false)
            : register_id_(register_id),
              begin_pc_(begin_pc),
              as_upvalue_(as_upvalue) { }
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

        // Prepare function data when enter each lexical function
        void EnterFunction()
        {
            auto function = new GenerateFunction;
            function->parent_ = current_function_;
            current_function_ = function;
            current_function_->function_ = state_->NewFunction();
        }

        // Clean up when leave lexical function
        void LeaveFunction()
        {
            DeleteCurrentFunction();
        }

        // Prepare some data when enter each lexical block
        void EnterBlock()
        {
            auto block = new GenerateBlock;
            block->parent_ = current_function_->current_block_;
            block->register_start_id_ = current_function_->register_id_;
            current_function_->current_block_ = block;
        }

        // Clean up when leave lexical block
        void LeaveBlock()
        {
            auto block = current_function_->current_block_;

            // Add all variables in block to the function local variable list
            auto function = current_function_->function_;
            auto end_pc = function->OpCodeSize();
            for (auto it = block->names_.begin(); it != block->names_.end(); ++it)
            {
                function->AddLocalVar(it->first, it->second.register_id_,
                                      it->second.begin_pc_, end_pc);
            }

            current_function_->current_block_ = block->parent_;
            current_function_->register_id_ = block->register_start_id_;
            delete block;
        }

        // Insert name into current local scope, replace its info when existed
        void InsertName(String *name, int register_id, bool as_upvalue)
        {
            assert(current_function_ && current_function_->current_block_);

            auto function = current_function_->function_;
            auto block = current_function_->current_block_;
            auto begin_pc = function->OpCodeSize();

            auto it = block->names_.find(name);
            if (it != block->names_.end())
            {
                // Add the same name variable to the function local variable list
                auto end_pc = function->OpCodeSize();
                function->AddLocalVar(name, it->second.register_id_,
                                      it->second.begin_pc_, end_pc);

                // New variable replace the old one
                it->second = LocalNameInfo(register_id, begin_pc, as_upvalue);
            }
            else
            {
                // Variable not existed, then insert into
                LocalNameInfo local(register_id, begin_pc, as_upvalue);
                block->names_.insert(std::make_pair(name, local));
            }
        }

        // Search name in current lexical function
        const LocalNameInfo * SearchLocalName(String *name) const
        {
            auto block = current_function_->current_block_;
            while (block)
            {
                auto it = block->names_.find(name);
                if (it != block->names_.end())
                    return &it->second;
                else
                    block = block->parent_;
            }

            return nullptr;
        }

        // Get current function data
        Function * GetCurrentFunction() const
        {
            return current_function_->function_;
        }

        // Generate one register id from current function
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

            // Delete all blocks in function
            while (function->current_block_)
            {
                auto block = function->current_block_;
                function->current_block_ = block->parent_;
                delete block;
            }

            current_function_ = function->parent_;
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
        // when end_register_ != EXP_VALUE_COUNT_ANY, otherwise fill any
        // count registers begin with start_register_
        int start_register_;
        int end_register_;

        ExpListData(int start_register, int end_register)
            : start_register_(start_register), end_register_(end_register) { }
    };

    // For expression and variable
    struct ExpVarData
    {
        // Need fill into range [start_register_, end_register_)
        // when end_register_ != EXP_VALUE_COUNT_ANY, otherwise fill any
        // count registers begin with start_register_
        int start_register_;
        int end_register_;

        ExpVarData(int start_register, int end_register)
            : start_register_(start_register), end_register_(end_register) { }
    };

    void CodeGenerateVisitor::Visit(Chunk *chunk, void *data)
    {
        CODE_GENERATE_GUARD(EnterFunction, LeaveFunction);
        {
            // Generate function code
            auto function = GetCurrentFunction();
            function->SetBaseInfo(chunk->module_, 0);

            CODE_GENERATE_GUARD(EnterBlock, LeaveBlock);
            chunk->block_->Accept(this, nullptr);

            // New one closure
            auto closure = state_->NewClosure();
            closure->SetPrototype(function);

            // Put closure on stack
            auto top = state_->stack_.top_++;
            top->closure_ = closure;
            top->type_ = ValueT_Closure;
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
        auto exp_var_data = static_cast<ExpVarData *>(data);
        auto register_id = exp_var_data->start_register_;
        auto end_register = exp_var_data->end_register_;

        // Just return when term is SemanticOp_Read and no registers to fill
        if (term->semantic_ == SemanticOp_Read &&
            end_register != EXP_VALUE_COUNT_ANY && register_id >= end_register)
            return ;

        auto function = GetCurrentFunction();
        if (term->token_.token_ == Token_Number || term->token_.token_ == Token_String)
        {
            // Load const to register
            auto index = 0;
            if (term->token_.token_ == Token_Number)
                index = function->AddConstNumber(term->token_.number_);
            else
                index = function->AddConstString(term->token_.str_);
            auto instruction = Instruction::ABCode(OpType_LoadConst, register_id++, index);
            function->AddInstruction(instruction, term->token_.line_);
        }
        else if (term->token_.token_ == Token_Id)
        {
            if (term->scoping_ == LexicalScoping_Global)
            {
                // Get value from global table by key index
                auto index = function->AddConstString(term->token_.str_);
                auto instruction = Instruction::ABCode(OpType_GetGlobal, register_id++, index);
                function->AddInstruction(instruction, term->token_.line_);
            }
            else if (term->scoping_ == LexicalScoping_Local)
            {
                // Load local variable value to dst register
                auto local = SearchLocalName(term->token_.str_);
                assert(local);
                auto instruction = Instruction::ABCode(OpType_Move, register_id++, local->register_id_);
                function->AddInstruction(instruction, term->token_.line_);
            }
        }

        // Fill nil into all remain registers
        // when end_register != EXP_VALUE_COUNT_ANY
        if (end_register != EXP_VALUE_COUNT_ANY)
        {
            while (register_id < end_register)
            {
                auto instruction = Instruction::ACode(OpType_LoadNil, register_id++);
                function->AddInstruction(instruction, term->token_.line_);
            }
        }
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
                auto instruction = Instruction::ACode(OpType_LoadNil, register_id);
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
        REGISTER_GENERATOR_GUARD();
        auto exp_var_data = static_cast<ExpVarData *>(data);
        auto start_register = exp_var_data ? exp_var_data->start_register_ : 0;
        auto end_register = exp_var_data ? exp_var_data->end_register_ : 0;

        // Generate code to get caller and its params
        auto caller_register = GenerateRegisterId();
        ExpVarData caller_data{ caller_register, caller_register + 1 };
        func_call->caller_->Accept(this, &caller_data);
        func_call->args_->Accept(this, nullptr);

        // Generate call instruction
        auto function = GetCurrentFunction();
        // Calculate expect results count of function call
        auto results = end_register == EXP_VALUE_COUNT_ANY ?
            EXP_VALUE_COUNT_ANY : end_register - start_register;
        auto instruction = Instruction::AsBxCode(OpType_Call,
                                                 caller_register, results);
        function->AddInstruction(instruction, func_call->line_);

        // Copy results of function call to dst registers
        // if end_register == EXP_VALUE_COUNT_ANY, then do not
        // copy results to dst registers, just keep it
        if (end_register != EXP_VALUE_COUNT_ANY)
        {
            auto src = caller_register;
            for (auto dst = start_register; dst < end_register; ++dst, ++src)
            {
                auto i = Instruction::ABCode(OpType_Move, dst, src);
                function->AddInstruction(i, func_call->line_);
            }
        }
    }

    void CodeGenerateVisitor::Visit(MemberFuncCall *, void *)
    {
    }

    void CodeGenerateVisitor::Visit(FuncCallArgs *arg, void *data)
    {
        if (arg->type_ == FuncCallArgs::ExpList)
        {
            if (arg->arg_)
            {
                auto start_register = GenerateRegisterId();
                ExpListData exp_list_data{ start_register, EXP_VALUE_COUNT_ANY };
                arg->arg_->Accept(this, &exp_list_data);
            }
        }
        else
        {
            // arg->type_ is FuncCallArgs::Table or FuncCallArgs::String
            auto start_register = GenerateRegisterId();
            ExpVarData exp_var_data{ start_register, start_register + 1 };
            arg->arg_->Accept(this, &exp_var_data);
        }
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
        int max_register = end_register == EXP_VALUE_COUNT_ANY ?
            std::numeric_limits<int>::max() : end_register;
        for (; i < count && register_id < max_register; ++i, ++register_id)
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
