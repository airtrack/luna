#include "CodeGenerate.h"
#include "State.h"
#include "String.h"
#include "Function.h"
#include "Exception.h"
#include "Guard.h"
#include <vector>
#include <stack>
#include <list>
#include <limits>
#include <utility>
#include <unordered_map>
#include <assert.h>

namespace luna
{
#define MAX_FUNCTION_REGISTER_COUNT 250
#define MAX_CLOSURE_UPVALUE_COUNT 250

#define CHECK_UPVALUE_MAX_COUNT(index, function)                        \
    if (index >= MAX_CLOSURE_UPVALUE_COUNT)                             \
    {                                                                   \
        throw CodeGenerateException(                                    \
            function->GetModule()->GetCStr(), function->GetLine(),      \
            "too many upvalues in function");                           \
    }

    struct LocalNameInfo
    {
        // Name register id
        int register_id_;
        // Name begin instruction
        int begin_pc_;

        explicit LocalNameInfo(int register_id = 0, int begin_pc = 0)
            : register_id_(register_id),
              begin_pc_(begin_pc) { }
    };

    // Loop AST info data in GenerateBlock
    struct LoopInfo
    {
        // Loop AST
        const SyntaxTree *loop_ast_;
        // Start instruction index
        int start_index_;

        LoopInfo() : loop_ast_(nullptr), start_index_(0) { }
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

        // Current loop ast info
        LoopInfo current_loop_;

        GenerateBlock() : parent_(nullptr), register_start_id_(0) { }
    };

    // Jump info for loop AST
    struct LoopJumpInfo
    {
        enum JumpType { JumpHead, JumpTail };
        // Owner loop AST
        const SyntaxTree *loop_ast_;
        // Jump to AST head or tail
        JumpType jump_type_;
        // Instruction need to be filled
        int instruction_index_;

        LoopJumpInfo(const SyntaxTree *loop_ast,
                     JumpType jump_type,
                     int instruction_index)
            : loop_ast_(loop_ast), jump_type_(jump_type),
              instruction_index_(instruction_index) { }
    };

    // Lexical function struct for code generator
    struct GenerateFunction
    {
        GenerateFunction *parent_;
        // Current block
        GenerateBlock *current_block_;
        // Current function for code generate
        Function *function_;
        // Index of current function in parent
        int func_index_;
        // Register id generator
        int register_id_;
        // Max register count used in current function
        int register_max_;
        // To be filled loop jump info
        std::list<LoopJumpInfo> loop_jumps_;

        GenerateFunction()
            : parent_(nullptr), current_block_(nullptr),
              function_(nullptr), func_index_(0),
              register_id_(0), register_max_(0) { }
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
            auto parent = current_function_;
            function->parent_ = parent;
            current_function_ = function;

            // New function is default on GCGen2, so barrier it
            current_function_->function_ = state_->NewFunction();
            CHECK_BARRIER(state_->GetGC(), current_function_->function_);

            if (parent)
            {
                auto index = parent->function_->AddChildFunction(function->function_);
                function->func_index_ = index;
                function->function_->SetSuperior(parent->function_);
                function->function_->SetModuleName(parent->function_->GetModule());
            }
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

        // Prepare data for loop AST
        void EnterLoop(const SyntaxTree *loop_ast)
        {
            // Start instruction index of loop
            int start_index = GetCurrentFunction()->OpCodeSize();
            auto block = current_function_->current_block_;
            block->current_loop_.loop_ast_ = loop_ast;
            block->current_loop_.start_index_ = start_index;
        }

        // Complete loop AST in current block
        void LeaveLoop()
        {
            auto function = GetCurrentFunction();
            // Instruction index after loop
            int end_index = function->OpCodeSize();

            auto &loop = current_function_->current_block_->current_loop_;
            if (loop.loop_ast_)
            {
                auto &loop_jumps = current_function_->loop_jumps_;
                auto it = loop_jumps.begin();
                while (it != loop_jumps.end())
                {
                    if (it->loop_ast_ == loop.loop_ast_)
                    {
                        // Calculate diff between current index with index of destination
                        int diff = 0;
                        if (it->jump_type_ == LoopJumpInfo::JumpHead)
                            diff = loop.start_index_ - it->instruction_index_;
                        else if (it->jump_type_ == LoopJumpInfo::JumpTail)
                            diff = end_index - it->instruction_index_;

                        // Get instruction and refill its jump diff
                        auto i = function->GetMutableInstruction(it->instruction_index_);
                        i->RefillsBx(diff);

                        // Remove it from loop_jumps when it refilled
                        loop_jumps.erase(it++);
                    }
                    else
                        ++it;
                }
            }
        }

        // Add one LoopJumpInfo, the instruction will be refilled
        // when the loop AST complete
        void AddLoopJumpInfo(const SyntaxTree *loop_ast, int instruction_index,
                             LoopJumpInfo::JumpType jump_type)
        {
            current_function_->loop_jumps_.push_back(LoopJumpInfo(loop_ast, jump_type,
                                                                  instruction_index));
        }

        // Insert name into current local scope, replace its info when existed
        void InsertName(String *name, int register_id)
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
                it->second = LocalNameInfo(register_id, begin_pc);
            }
            else
            {
                // Variable not existed, then insert into
                LocalNameInfo local(register_id, begin_pc);
                block->names_.insert(std::make_pair(name, local));
            }
        }

        // Search name in current lexical function
        const LocalNameInfo * SearchLocalName(String *name) const
        {
            return SearchFunctionLocalName(current_function_, name);
        }

        // Search name in lexical function
        const LocalNameInfo * SearchFunctionLocalName(GenerateFunction *function,
                                                      String *name) const
        {
            auto block = function->current_block_;
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

        // Prepare upvalue info when the name upvalue info not existed, and
        // return upvalue index, otherwise just return upvalue index
        // the name must reference a upvalue, otherwise will assert fail
        int PrepareUpvalue(String *name) const
        {
            // If the upvalue info existed, then return the index of the upvalue
            auto function = GetCurrentFunction();
            auto index = function->SearchUpvalue(name);
            if (index >= 0)
                return index;

            // Search start from parent
            std::stack<GenerateFunction *> parents;
            parents.push(current_function_->parent_);

            int register_index = -1;
            bool parent_local = false;
            while (!parents.empty())
            {
                auto current = parents.top();
                assert(current);
                if (register_index >= 0)
                {
                    // Find it, add it as upvalue to function,
                    // and continue backtrack
                    auto index = current->function_->AddUpvalue(name, parent_local,
                                                                register_index);
                    CHECK_UPVALUE_MAX_COUNT(index, current->function_);
                    register_index = index;
                    parent_local = false;
                    parents.pop();
                }
                else
                {
                    // Find name from local names
                    auto name_info = SearchFunctionLocalName(current, name);
                    if (name_info)
                    {
                        // Find it, get its register_id and start backtrack
                        register_index = name_info->register_id_;
                        parent_local = true;
                        parents.pop();
                    }
                    else
                    {
                        // Find it from current function upvalue list
                        auto index = current->function_->SearchUpvalue(name);
                        if (index >= 0)
                        {
                            // Find it, the name upvalue has been inserted,
                            // then get the upvalue index, and start backtrack
                            register_index = index;
                            parent_local = false;
                            parents.pop();
                        }
                        else
                        {
                            // Not find it, continue to search its parent
                            parents.push(current->parent_);
                        }
                    }
                }
            }

            // Add it as upvalue to current function
            assert(register_index >= 0);
            index = function->AddUpvalue(name, parent_local, register_index);
            CHECK_UPVALUE_MAX_COUNT(index, function);
            return index;
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
            if (IsRegisterCountOverflow())
            {
                throw CodeGenerateException(
                    GetCurrentFunction()->GetModule()->GetCStr(),
                    GetCurrentFunction()->GetLine(),
                    "too many local variables in function");
            }
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

        // Is register count overflow
        bool IsRegisterCountOverflow()
        {
            if (current_function_->register_id_ > current_function_->register_max_)
                current_function_->register_max_ = current_function_->register_id_;
            return current_function_->register_max_ > MAX_FUNCTION_REGISTER_COUNT;
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

        void FillRemainRegisterNil(int register_id, int end_register, int line)
        {
            // Fill nil into all remain registers
            // when end_register != EXP_VALUE_COUNT_ANY
            auto function = GetCurrentFunction();
            if (end_register != EXP_VALUE_COUNT_ANY)
            {
                while (register_id < end_register)
                {
                    auto instruction = Instruction::ACode(OpType_LoadNil, register_id++);
                    function->AddInstruction(instruction, line);
                }
            }
        }

        template<typename StatementType>
        void IfStatementGenerateCode(StatementType *if_stmt);

        template<typename TableFieldType>
        void SetTableFieldValue(TableFieldType *field,
                                int table_register,
                                int key_register,
                                int line);

        template<typename TableAccessorType, typename LoadKey>
        void AccessTableField(TableAccessorType *accessor,
                              void *data, int line,
                              const LoadKey &load_key);

        template<typename FuncCallType, typename CallerArgAdjuster>
        void FunctionCall(FuncCallType *func_call, void *data,
                          const CallerArgAdjuster &adjust_caller_arg);

        // Current code generating function
        GenerateFunction *current_function_;
    };

#define CODE_GENERATE_GUARD(enter, leave)                               \
    Guard g([this]() { this->enter(); }, [this]() { this->leave(); })

#define REGISTER_GENERATOR_GUARD()                                      \
    auto r = GetNextRegisterId();                                       \
    Guard g([]() { }, [=]() { this->ResetRegisterIdGenerator(r); })

#define LOOP_GUARD(loop_ast)                                            \
    Guard l([=]() { this->EnterLoop(loop_ast); },                       \
            [=]() { this->LeaveLoop(); })

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

    // For VarList AST
    struct VarListData
    {
        // VarList get results from range [start_register_, end_register_)
        int start_register_;
        int end_register_;

        VarListData(int start_register, int end_register)
            : start_register_(start_register), end_register_(end_register) { }
    };

    // For table field
    struct TableFieldData
    {
        // Table register
        int table_register_;

        // Array part index, start from 1
        unsigned int array_index_;

        explicit TableFieldData(int table_register)
            : table_register_(table_register),
              array_index_(1) { }
    };

    // For FuncCallArgs AST
    struct FuncCallArgsData
    {
        int arg_value_count_;

        FuncCallArgsData() : arg_value_count_(0) { }
    };

    // For FunctionName AST
    struct FunctionNameData
    {
        int func_register_;

        explicit FunctionNameData(int func_register)
            : func_register_(func_register) { }
    };

    template<typename StatementType>
    void CodeGenerateVisitor::IfStatementGenerateCode(StatementType *if_stmt)
    {
        auto function = GetCurrentFunction();
        int jmp_end_index = 0;
        {
            REGISTER_GENERATOR_GUARD();
            auto register_id = GenerateRegisterId();
            ExpVarData exp_var_data{ register_id, register_id + 1 };
            if_stmt->exp_->Accept(this, &exp_var_data);

            auto instruction = Instruction::AsBxCode(OpType_JmpFalse, register_id, 0);
            int jmp_index = function->AddInstruction(instruction, if_stmt->line_);

            {
                // True branch block generate code
                CODE_GENERATE_GUARD(EnterBlock, LeaveBlock);
                if_stmt->true_branch_->Accept(this, nullptr);
            }

            // Jmp to the end of if-elseif-else statement after excute block
            instruction = Instruction::AsBxCode(OpType_Jmp, 0, 0);
            jmp_end_index = function->AddInstruction(instruction, if_stmt->block_end_line_);

            // Refill OpType_JmpFalse instruction
            int index = function->OpCodeSize();
            function->GetMutableInstruction(jmp_index)->RefillsBx(index - jmp_index);
        }

        if (if_stmt->false_branch_)
            if_stmt->false_branch_->Accept(this, nullptr);

        // Refill OpType_Jmp instruction
        int end_index = function->OpCodeSize();
        function->GetMutableInstruction(jmp_end_index)->RefillsBx(end_index - jmp_end_index);
    }

    template<typename TableFieldType>
    void CodeGenerateVisitor::SetTableFieldValue(TableFieldType *field,
                                                 int table_register,
                                                 int key_register,
                                                 int line)
    {
        // Load value
        auto value_register = GenerateRegisterId();
        ExpVarData exp_var_data{ value_register, value_register + 1 };
        field->value_->Accept(this, &exp_var_data);

        // Set table field
        auto instruction = Instruction::ABCCode(OpType_SetTable, table_register,
                                                key_register, value_register);
        GetCurrentFunction()->AddInstruction(instruction, line);
    }

    template<typename TableAccessorType, typename LoadKey>
    void CodeGenerateVisitor::AccessTableField(TableAccessorType *accessor,
                                               void *data, int line,
                                               const LoadKey &load_key)
    {
        auto exp_var_data = static_cast<ExpVarData *>(data);
        auto register_id = exp_var_data->start_register_;
        auto end_register = exp_var_data->end_register_;
        auto function = GetCurrentFunction();

        int table_register = 0;
        int key_register = 0;
        int value_register = 0;
        OpType op_type;
        if (accessor->semantic_ == SemanticOp_Read)
        {
            // No more register, do nothing
            if (end_register != EXP_VALUE_COUNT_ANY && register_id >= end_register)
                return ;

            if (end_register != EXP_VALUE_COUNT_ANY && register_id + 1 < end_register)
                key_register = register_id + 1;
            else
                key_register = GenerateRegisterId();
            table_register = register_id;
            value_register = register_id;
            op_type = OpType_GetTable;
        }
        else
        {
            assert(accessor->semantic_ == SemanticOp_Write);
            assert(register_id + 1 == end_register);

            table_register = GenerateRegisterId();
            key_register = GenerateRegisterId();
            value_register = register_id;
            op_type = OpType_SetTable;
        }

        // Load table
        ExpVarData table_exp_var_data{ table_register, table_register + 1 };
        accessor->table_->Accept(this, &table_exp_var_data);

        // Load key
        load_key(key_register);

        // Set/Get table value by key
        auto instruction = Instruction::ABCCode(op_type, table_register,
                                                key_register, value_register);
        function->AddInstruction(instruction, line);

        if (accessor->semantic_ == SemanticOp_Read)
            FillRemainRegisterNil(register_id + 1, end_register, line);
    }

    template<typename FuncCallType, typename CallerArgAdjuster>
    void CodeGenerateVisitor::FunctionCall(FuncCallType *func_call, void *data,
                                           const CallerArgAdjuster &adjust_caller_arg)
    {
        REGISTER_GENERATOR_GUARD();
        auto exp_var_data = static_cast<ExpVarData *>(data);
        auto start_register = exp_var_data ? exp_var_data->start_register_ : 0;
        auto end_register = exp_var_data ? exp_var_data->end_register_ : 0;

        // Generate code to get caller
        int caller_register = 0;
        if (end_register == EXP_VALUE_COUNT_ANY)
            caller_register = start_register;
        else
            caller_register = GenerateRegisterId();
        {
            REGISTER_GENERATOR_GUARD();
            ExpVarData caller_data{ caller_register, caller_register + 1 };
            func_call->caller_->Accept(this, &caller_data);
        }

        // Adjust caller, and also adjust args,
        // return how many args adjusted
        int adjust_args = adjust_caller_arg(caller_register);

        FuncCallArgsData arg_data;
        func_call->args_->Accept(this, &arg_data);

        // Calculate total args
        int total_args = arg_data.arg_value_count_ == EXP_VALUE_COUNT_ANY ?
            EXP_VALUE_COUNT_ANY : arg_data.arg_value_count_ + adjust_args;

        // Calculate expect results count of function call
        auto results = end_register == EXP_VALUE_COUNT_ANY ?
            EXP_VALUE_COUNT_ANY : end_register - start_register;

        // Generate call instruction
        auto function = GetCurrentFunction();
        auto instruction = Instruction::ABCCode(OpType_Call,
                                                caller_register,
                                                total_args + 1,
                                                results + 1);
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

    void CodeGenerateVisitor::Visit(Chunk *chunk, void *data)
    {
        CODE_GENERATE_GUARD(EnterFunction, LeaveFunction);
        {
            // Generate function code
            auto function = GetCurrentFunction();
            function->SetModuleName(chunk->module_);
            function->SetLine(1);

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

    void CodeGenerateVisitor::Visit(ReturnStatement *ret_stmt, void *data)
    {
        int register_id = GetNextRegisterId();
        if (ret_stmt->exp_list_)
        {
            register_id = GenerateRegisterId();
            ExpListData exp_list_data{ register_id, EXP_VALUE_COUNT_ANY };
            ret_stmt->exp_list_->Accept(this, &exp_list_data);
        }

        auto function = GetCurrentFunction();
        auto instruction = Instruction::AsBxCode(OpType_Ret, register_id,
                                                 ret_stmt->exp_value_count_);
        function->AddInstruction(instruction, ret_stmt->line_);
    }

    void CodeGenerateVisitor::Visit(BreakStatement *break_stmt, void *data)
    {
        assert(break_stmt->loop_);
        auto function = GetCurrentFunction();
        auto instruction = Instruction::AsBxCode(OpType_Jmp, 0, 0);
        int index = function->AddInstruction(instruction, break_stmt->break_.line_);
        AddLoopJumpInfo(break_stmt->loop_, index, LoopJumpInfo::JumpTail);
    }

    void CodeGenerateVisitor::Visit(DoStatement *do_stmt, void *data)
    {
        CODE_GENERATE_GUARD(EnterBlock, LeaveBlock);
        do_stmt->block_->Accept(this, nullptr);
    }

    void CodeGenerateVisitor::Visit(WhileStatement *while_stmt, void *data)
    {
        CODE_GENERATE_GUARD(EnterBlock, LeaveBlock);
        LOOP_GUARD(while_stmt);

        auto register_id = GenerateRegisterId();
        ExpVarData exp_var_data{ register_id, register_id + 1 };
        while_stmt->exp_->Accept(this, &exp_var_data);

        // Jump to loop tail when expression is false
        auto function = GetCurrentFunction();
        auto instruction = Instruction::AsBxCode(OpType_JmpFalse, register_id, 0);
        int index = function->AddInstruction(instruction, while_stmt->first_line_);
        AddLoopJumpInfo(while_stmt, index, LoopJumpInfo::JumpTail);

        while_stmt->block_->Accept(this, nullptr);

        // Jump to loop head
        instruction = Instruction::AsBxCode(OpType_Jmp, 0, 0);
        index = function->AddInstruction(instruction, while_stmt->last_line_);
        AddLoopJumpInfo(while_stmt, index, LoopJumpInfo::JumpHead);
    }

    void CodeGenerateVisitor::Visit(RepeatStatement *repeat_stmt, void *data)
    {
        CODE_GENERATE_GUARD(EnterBlock, LeaveBlock);
        LOOP_GUARD(repeat_stmt);
        {
            REGISTER_GENERATOR_GUARD();
            repeat_stmt->block_->Accept(this, nullptr);
        }

        // Get exp value
        auto register_id = GenerateRegisterId();
        ExpVarData exp_var_data{ register_id, register_id + 1 };
        repeat_stmt->exp_->Accept(this, &exp_var_data);

        // Jump to head when exp value is true
        auto function = GetCurrentFunction();
        auto instruction = Instruction::AsBxCode(OpType_JmpFalse, register_id, 0);
        int index = function->AddInstruction(instruction, repeat_stmt->line_);
        AddLoopJumpInfo(repeat_stmt, index, LoopJumpInfo::JumpHead);
    }

    void CodeGenerateVisitor::Visit(IfStatement *if_stmt, void *data)
    {
        IfStatementGenerateCode(if_stmt);
    }

    void CodeGenerateVisitor::Visit(ElseIfStatement *elseif_stmt, void *data)
    {
        IfStatementGenerateCode(elseif_stmt);
    }

    void CodeGenerateVisitor::Visit(ElseStatement *else_stmt, void *data)
    {
        else_stmt->block_->Accept(this, nullptr);
    }

    void CodeGenerateVisitor::Visit(NumericForStatement *num_for, void *data)
    {
        CODE_GENERATE_GUARD(EnterBlock, LeaveBlock);

        auto var_register = GenerateRegisterId();
        auto limit_register = GenerateRegisterId();
        auto step_register = GenerateRegisterId();
        auto function = GetCurrentFunction();
        auto line = num_for->name_.line_;

        // Init name, limit, step
        {
            REGISTER_GENERATOR_GUARD();
            ExpVarData name_exp_data{ var_register, var_register + 1 };
            num_for->exp1_->Accept(this, &name_exp_data);
        }
        {
            REGISTER_GENERATOR_GUARD();
            ExpVarData limit_exp_data{ limit_register, limit_register + 1 };
            num_for->exp2_->Accept(this, &limit_exp_data);
        }
        {
            REGISTER_GENERATOR_GUARD();
            if (num_for->exp3_)
            {
                ExpVarData step_exp_data{ step_register, step_register + 1 };
                num_for->exp3_->Accept(this, &step_exp_data);
            }
            else
            {
                // Default step is 1
                auto instruction = Instruction::ACode(OpType_LoadInt, step_register);
                function->AddInstruction(instruction, line);
                // Int value 1
                instruction.opcode_ = 1;
                function->AddInstruction(instruction, line);
            }
        }

        // Init 'for' var, limit, step value
        auto instruction = Instruction::ABCCode(OpType_ForInit, var_register,
                                                limit_register, step_register);
        function->AddInstruction(instruction, line);

        auto name_register = GenerateRegisterId();
        InsertName(num_for->name_.str_, name_register);

        LOOP_GUARD(num_for);
        // Prepare name value
        instruction = Instruction::ABCode(OpType_Move, name_register, var_register);
        function->AddInstruction(instruction, line);

        // Check 'for', continue loop or not
        instruction = Instruction::ABCCode(OpType_ForStep, var_register,
                                           limit_register, step_register);
        function->AddInstruction(instruction, line);

        // Break loop, prepare to jump to the end of the loop
        instruction.opcode_ = 0;
        int index = function->AddInstruction(instruction, line);
        AddLoopJumpInfo(num_for, index, LoopJumpInfo::JumpTail);

        num_for->block_->Accept(this, nullptr);

        // var = var + step
        instruction = Instruction::ABCCode(OpType_Add, var_register,
                                           var_register, step_register);
        function->AddInstruction(instruction, line);

        // Jump to the begin of the loop
        instruction = Instruction::AsBxCode(OpType_Jmp, 0, 0);
        index = function->AddInstruction(instruction, line);
        AddLoopJumpInfo(num_for, index, LoopJumpInfo::JumpHead);
    }

    void CodeGenerateVisitor::Visit(GenericForStatement *gen_for, void *data)
    {
        CODE_GENERATE_GUARD(EnterBlock, LeaveBlock);

        // Init generic for statement data
        auto func_register = GenerateRegisterId();
        auto state_register = GenerateRegisterId();
        auto var_register = GenerateRegisterId();
        ExpListData exp_list_data{ func_register, var_register + 1 };
        gen_for->exp_list_->Accept(this, &exp_list_data);

        // Alloca registers for names
        auto name_start = GetNextRegisterId();
        NameListData name_list_data{ false };
        gen_for->name_list_->Accept(this, &name_list_data);
        auto name_end = GetNextRegisterId();
        assert(name_start < name_end);

        auto function = GetCurrentFunction();
        auto line = gen_for->line_;
        LOOP_GUARD(gen_for);
        {
            REGISTER_GENERATOR_GUARD();
            // Alloca temp registers for call iterate function
            auto temp_func = GenerateRegisterId();
            auto temp_state = GenerateRegisterId();
            auto temp_var = GenerateRegisterId();

            // Call iterate function
            auto move = [=](int dst, int src) {
                auto instruction = Instruction::ABCode(OpType_Move, dst, src);
                function->AddInstruction(instruction, line);
            };
            move(temp_func, func_register);
            move(temp_state, state_register);
            move(temp_var, var_register);

            auto instruction = Instruction::ABCCode(OpType_Call, temp_func,
                                                    2 + 1,  // Two args
                                                    name_end - name_start + 1);
            function->AddInstruction(instruction, line);

            // Copy results to registers of names
            for (auto name = name_start; name < name_end; ++name, ++temp_func)
                move(name, temp_func);

            // Break the loop when the first name value is nil
            instruction = Instruction::AsBxCode(OpType_JmpNil, name_start, 0);
            int index = function->AddInstruction(instruction, line);
            AddLoopJumpInfo(gen_for, index, LoopJumpInfo::JumpTail);

            // Copy first name value to var_register
            move(var_register, name_start);
        }
        gen_for->block_->Accept(this, nullptr);

        // Jump to loop start
        auto instruction = Instruction::AsBxCode(OpType_Jmp, 0, 0);
        int index = function->AddInstruction(instruction, line);
        AddLoopJumpInfo(gen_for, index, LoopJumpInfo::JumpHead);
    }

    void CodeGenerateVisitor::Visit(FunctionStatement *func_stmt, void *data)
    {
        REGISTER_GENERATOR_GUARD();
        auto func_register = GenerateRegisterId();
        ExpVarData exp_var_data{ func_register, func_register + 1 };
        func_stmt->func_body_->Accept(this, &exp_var_data);

        FunctionNameData name_data{ func_register };
        func_stmt->func_name_->Accept(this, &name_data);
    }

    void CodeGenerateVisitor::Visit(FunctionName *func_name, void *data)
    {
        assert(!func_name->names_.empty());
        auto func_register = static_cast<FunctionNameData *>(data)->func_register_;
        auto function = GetCurrentFunction();

        bool has_member = func_name->names_.size() > 1 ||
            func_name->member_name_.token_ == Token_Id;

        auto first_name = func_name->names_[0].str_;
        auto first_line = func_name->names_[0].line_;

        if (!has_member)
        {
            assert(func_name->names_.size() == 1);
            Instruction instruction;
            if (func_name->scoping_ == LexicalScoping_Global)
            {
                // Define a global function
                auto index = function->AddConstString(first_name);
                instruction = Instruction::ABxCode(OpType_SetGlobal, func_register, index);
            }
            else if (func_name->scoping_ == LexicalScoping_Upvalue)
            {
                // Change a upvalue to a function
                auto index = PrepareUpvalue(first_name);
                instruction = Instruction::ABCode(OpType_SetUpvalue, func_register, index);
            }
            else if (func_name->scoping_ == LexicalScoping_Local)
            {
                // Change a local variable to a function
                auto local_name = SearchLocalName(first_name);
                instruction = Instruction::ABCode(OpType_Move, local_name->register_id_,
                                                  func_register);
            }
            function->AddInstruction(instruction, first_line);
        }
        else
        {
            Instruction instruction;
            auto table_register = GenerateRegisterId();
            if (func_name->scoping_ == LexicalScoping_Global)
            {
                // Load global variable to table register
                auto index = function->AddConstString(first_name);
                instruction = Instruction::ABxCode(OpType_GetGlobal,
                                                   table_register, index);
            }
            else if (func_name->scoping_ == LexicalScoping_Upvalue)
            {
                // Load upvalue to table register
                auto index = PrepareUpvalue(first_name);
                instruction = Instruction::ABCode(OpType_GetUpvalue, table_register, index);
            }
            else if (func_name->scoping_ == LexicalScoping_Local)
            {
                // Load local variable to table register
                auto local_name = SearchLocalName(first_name);
                instruction = Instruction::ABCode(OpType_Move, table_register,
                                                  local_name->register_id_);
            }
            function->AddInstruction(instruction, first_line);

            bool member = func_name->member_name_.token_ == Token_Id;
            auto size = func_name->names_.size();
            auto count = member ? size : size - 1;
            auto key_register = GenerateRegisterId();

            auto load_key = [=](String *name, int line) {
                auto index = function->AddConstString(name);
                auto instruction = Instruction::ABxCode(OpType_LoadConst, key_register, index);
                function->AddInstruction(instruction, line);
            };

            for (std::size_t i = 1; i < count; ++i)
            {
                // Get value from table by key
                auto name = func_name->names_[i].str_;
                auto line = func_name->names_[i].line_;
                load_key(name, line);
                instruction = Instruction::ABCCode(OpType_GetTable, table_register,
                                                   key_register, table_register);
                function->AddInstruction(instruction, line);
            }

            // Set function as value of table by key 'token'
            const auto &token = member ? func_name->member_name_ : func_name->names_.back();
            load_key(token.str_, token.line_);
            instruction = Instruction::ABCCode(OpType_SetTable, table_register,
                                               key_register, func_register);
            function->AddInstruction(instruction, token.line_);
        }
    }

    void CodeGenerateVisitor::Visit(LocalFunctionStatement *l_func_stmt, void *data)
    {
        auto register_id = GenerateRegisterId();
        InsertName(l_func_stmt->name_.str_, register_id);
        ExpVarData exp_var_data{ register_id, register_id + 1 };
        l_func_stmt->func_body_->Accept(this, &exp_var_data);
    }

    void CodeGenerateVisitor::Visit(LocalNameListStatement *l_namelist_stmt, void *data)
    {
        // Generate code for expression list first, then expression list can get
        // variables which has the same name with variables defined in NameList
        // e.g.
        //     local i = 1
        //     local i = i -- i value is 1
        if (l_namelist_stmt->exp_list_)
        {
            // Reserve registers for NameList
            int start_register = GetNextRegisterId();
            int end_register = start_register + l_namelist_stmt->name_count_;
            Guard g([=]() { this->ResetRegisterIdGenerator(end_register); },
                    [=]() { this->ResetRegisterIdGenerator(start_register); });

            try
            {
                ExpListData exp_list_data{ start_register, end_register };
                l_namelist_stmt->exp_list_->Accept(this, &exp_list_data);
            }
            catch (const CodeGenerateException &)
            {
                throw CodeGenerateException(
                    GetCurrentFunction()->GetModule()->GetCStr(),
                    l_namelist_stmt->line_,
                    "expression of local name list is too complex");
            }
        }

        // NameList need init itself when ExpList is not existed
        NameListData name_list_data{ !l_namelist_stmt->exp_list_ };
        l_namelist_stmt->name_list_->Accept(this, &name_list_data);
    }

    void CodeGenerateVisitor::Visit(AssignmentStatement *assign_stmt, void *data)
    {
        REGISTER_GENERATOR_GUARD();

        // Reserve registers for var list
        int register_id = GetNextRegisterId();
        int end_register = register_id + assign_stmt->var_count_;
        ResetRegisterIdGenerator(end_register);
        if (IsRegisterCountOverflow())
        {
            throw CodeGenerateException(
                GetCurrentFunction()->GetModule()->GetCStr(),
                assign_stmt->line_,
                "assignment statement is too complex");
        }

        try
        {
            // Get exp list results placed into [register_id, end_register)
            ExpListData exp_list_data{ register_id, end_register };
            assign_stmt->exp_list_->Accept(this, &exp_list_data);
        }
        catch (const CodeGenerateException &)
        {
            // Exp list consume some registers, and register count overflow,
            // catch it, throw new exception to report assignment statement
            // is too complex
            throw CodeGenerateException(
                GetCurrentFunction()->GetModule()->GetCStr(),
                assign_stmt->line_,
                "assignment statement is too complex");
        }

        // Assign results to var list
        VarListData var_list_data{ register_id, end_register };
        assign_stmt->var_list_->Accept(this, &var_list_data);
    }

    void CodeGenerateVisitor::Visit(VarList *var_list, void *data)
    {
        auto var_list_data = static_cast<VarListData *>(data);
        int register_id = var_list_data->start_register_;
        int end_register = var_list_data->end_register_;
        int var_count = var_list->var_list_.size();
        assert(end_register - register_id == var_count);

        // Assign results to each variable
        for (int i = 0; i < var_count; ++i, ++register_id)
        {
            ExpVarData exp_var_data{ register_id, register_id + 1 };
            var_list->var_list_[i]->Accept(this, &exp_var_data);
        }
    }

    void CodeGenerateVisitor::Visit(Terminator *term, void *data)
    {
        auto exp_var_data = static_cast<ExpVarData *>(data);
        auto register_id = exp_var_data->start_register_;
        auto end_register = exp_var_data->end_register_;
        auto function = GetCurrentFunction();

        // Generate code for SemanticOp_Write
        if (term->semantic_ == SemanticOp_Write)
        {
            assert(term->token_.token_ == Token_Id);
            assert(register_id + 1 == end_register);
            if (term->scoping_ == LexicalScoping_Global)
            {
                auto index = function->AddConstString(term->token_.str_);
                auto instruction = Instruction::ABxCode(OpType_SetGlobal, register_id, index);
                function->AddInstruction(instruction, term->token_.line_);
            }
            else if (term->scoping_ == LexicalScoping_Local)
            {
                auto local = SearchLocalName(term->token_.str_);
                assert(local);
                auto instruction = Instruction::ABCode(OpType_Move, local->register_id_, register_id);
                function->AddInstruction(instruction, term->token_.line_);
            }
            else if (term->scoping_ == LexicalScoping_Upvalue)
            {
                auto index = PrepareUpvalue(term->token_.str_);
                auto instruction = Instruction::ABCode(OpType_SetUpvalue, register_id, index);
                function->AddInstruction(instruction, term->token_.line_);
            }
            return ;
        }

        // Generate code for SemanticOp_Read
        // Just return when term is SemanticOp_Read and no registers to fill
        if (term->semantic_ == SemanticOp_Read &&
            end_register != EXP_VALUE_COUNT_ANY && register_id >= end_register)
            return ;

        if (term->token_.token_ == Token_Number || term->token_.token_ == Token_String)
        {
            // Load const to register
            auto index = 0;
            if (term->token_.token_ == Token_Number)
                index = function->AddConstNumber(term->token_.number_);
            else
                index = function->AddConstString(term->token_.str_);
            auto instruction = Instruction::ABxCode(OpType_LoadConst, register_id++, index);
            function->AddInstruction(instruction, term->token_.line_);
        }
        else if (term->token_.token_ == Token_Id)
        {
            if (term->scoping_ == LexicalScoping_Global)
            {
                // Get value from global table by key index
                auto index = function->AddConstString(term->token_.str_);
                auto instruction = Instruction::ABxCode(OpType_GetGlobal, register_id++, index);
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
            else if (term->scoping_ == LexicalScoping_Upvalue)
            {
                // Get upvalue index
                auto index = PrepareUpvalue(term->token_.str_);
                auto instruction = Instruction::ABCode(OpType_GetUpvalue, register_id++, index);
                function->AddInstruction(instruction, term->token_.line_);
            }
        }
        else if (term->token_.token_ == Token_True || term->token_.token_ == Token_False)
        {
            auto bvalue = term->token_.token_ == Token_True ? 1 : 0;
            auto instruction = Instruction::ABCode(OpType_LoadBool, register_id++, bvalue);
            function->AddInstruction(instruction, term->token_.line_);
        }
        else if (term->token_.token_ == Token_Nil)
        {
            auto instruction = Instruction::ACode(OpType_LoadNil, register_id++);
            function->AddInstruction(instruction, term->token_.line_);
        }
        else if (term->token_.token_ == Token_VarArg)
        {
            // Copy vararg to registers which start from register_id
            auto expect_results = end_register == EXP_VALUE_COUNT_ANY ?
                EXP_VALUE_COUNT_ANY : end_register - register_id;
            auto instruction = Instruction::AsBxCode(OpType_VarArg, register_id, expect_results);
            function->AddInstruction(instruction, term->token_.line_);

            // All registers will be filled when executing, so do not
            // fill nil to remain registers
            register_id = end_register;
        }

        FillRemainRegisterNil(register_id, end_register, term->token_.line_);
    }

    void CodeGenerateVisitor::Visit(BinaryExpression *bin_exp, void *data)
    {
        auto exp_var_data = static_cast<ExpVarData *>(data);
        auto register_id = exp_var_data->start_register_;
        auto end_register = exp_var_data->end_register_;

        if (end_register != EXP_VALUE_COUNT_ANY && register_id >= end_register)
            return ;

        auto function = GetCurrentFunction();
        auto line = bin_exp->op_token_.line_;
        auto token = bin_exp->op_token_.token_;
        if (token == Token_And || token == Token_Or)
        {
            // Calculate left expression
            ExpVarData left_data{ register_id, register_id + 1 };
            bin_exp->left_->Accept(this, &left_data);

            // Do not calculate right expression when the result of left expression
            // satisfy semantics of operator
            auto op_type = token == Token_And ? OpType_JmpFalse : OpType_JmpTrue;
            auto instruction = Instruction::AsBxCode(op_type, register_id, 0);
            int index = function->AddInstruction(instruction, line);

            // Calculate right expression
            ExpVarData right_data{ register_id, register_id + 1 };
            bin_exp->right_->Accept(this, &right_data);

            int dst_index = function->OpCodeSize();
            function->GetMutableInstruction(index)->RefillsBx(dst_index - index);

            return FillRemainRegisterNil(register_id + 1, end_register, line);
        }

        int left_register = 0;
        // Generate code to calculate left expression
        {
            ExpVarData exp_var_data{ register_id, register_id + 1 };
            bin_exp->left_->Accept(this, &exp_var_data);
            left_register = register_id;
        }

        int right_register = 0;
        // Generate code to calculate right expression
        {
            if (end_register != EXP_VALUE_COUNT_ANY && register_id + 1 < end_register)
            {
                // If parent AST provide more than one register, then use the second
                // register as temp register of right expression
                ExpVarData exp_var_data{ register_id + 1, register_id + 2 };
                bin_exp->right_->Accept(this, &exp_var_data);
                right_register = register_id + 1;
            }
            else
            {
                // No more register, then generate a new register as temp register of
                // right expression
                REGISTER_GENERATOR_GUARD();
                right_register = GenerateRegisterId();
                ExpVarData exp_var_data{ right_register, right_register + 1 };
                bin_exp->right_->Accept(this, &exp_var_data);
            }
        }

        // Choose OpType by operator
        OpType op_type;
        switch (token) {
            case '+': op_type = OpType_Add; break;
            case '-': op_type = OpType_Sub; break;
            case '*': op_type = OpType_Mul; break;
            case '/': op_type = OpType_Div; break;
            case '^': op_type = OpType_Pow; break;
            case '%': op_type = OpType_Mod; break;
            case '<': op_type = OpType_Less; break;
            case '>': op_type = OpType_Greater; break;
            case Token_Concat: op_type = OpType_Concat; break;
            case Token_Equal: op_type = OpType_Equal; break;
            case Token_NotEqual: op_type = OpType_UnEqual; break;
            case Token_LessEqual: op_type = OpType_LessEqual; break;
            case Token_GreaterEqual: op_type = OpType_GreaterEqual; break;
            default: assert(0); break;
        }

        // Generate instruction to calculate
        auto instruction = Instruction::ABCCode(op_type, register_id++,
                                                left_register, right_register);
        function->AddInstruction(instruction, line);

        FillRemainRegisterNil(register_id, end_register, line);
    }

    void CodeGenerateVisitor::Visit(UnaryExpression *unexp, void *data)
    {
        auto exp_var_data = static_cast<ExpVarData *>(data);
        auto register_id = exp_var_data->start_register_;
        auto end_register = exp_var_data->end_register_;

        if (end_register != EXP_VALUE_COUNT_ANY && register_id >= end_register)
            return ;

        unexp->exp_->Accept(this, exp_var_data);

        // Choose OpType by operator
        OpType op_type;
        switch (unexp->op_token_.token_)
        {
            case '-': op_type = OpType_Neg; break;
            case '#': op_type = OpType_Len; break;
            case Token_Not: op_type = OpType_Not; break;
            default: assert(0); break;
        }

        // Generate instruction
        auto function = GetCurrentFunction();
        auto instruction = Instruction::ACode(op_type, register_id++);
        function->AddInstruction(instruction, unexp->op_token_.line_);

        FillRemainRegisterNil(register_id, end_register, unexp->op_token_.line_);
    }

    void CodeGenerateVisitor::Visit(FunctionBody *func_body, void *data)
    {
        int child_index = 0;
        {
            CODE_GENERATE_GUARD(EnterFunction, LeaveFunction);
            auto function = GetCurrentFunction();
            function->SetLine(func_body->line_);
            child_index = current_function_->func_index_;

            {
                CODE_GENERATE_GUARD(EnterBlock, LeaveBlock);
                // Child function generate code
                if (func_body->has_self_)
                {
                    auto register_id = GenerateRegisterId();
                    auto self = state_->GetString("self");
                    InsertName(self, register_id);

                    auto function = GetCurrentFunction();
                    function->AddFixedArgCount(1);
                }

                if (func_body->param_list_)
                    func_body->param_list_->Accept(this, nullptr);
                func_body->block_->Accept(this, nullptr);
            }
        }

        // Generate closure
        auto exp_var_data = static_cast<ExpVarData *>(data);
        auto register_id = exp_var_data->start_register_;
        auto end_register = exp_var_data->end_register_;
        if (end_register == EXP_VALUE_COUNT_ANY || register_id < end_register)
        {
            auto function = GetCurrentFunction();
            auto i = Instruction::ABxCode(OpType_Closure,
                                          register_id++,
                                          child_index);
            function->AddInstruction(i, func_body->line_);
        }

        FillRemainRegisterNil(register_id, end_register, func_body->line_);
    }

    void CodeGenerateVisitor::Visit(ParamList *param_list, void *data)
    {
        auto function = GetCurrentFunction();
        function->AddFixedArgCount(param_list->fix_arg_count_);
        if (param_list->vararg_) function->SetHasVararg();

        if (param_list->name_list_)
        {
            NameListData name_list_data{ false };
            param_list->name_list_->Accept(this, &name_list_data);
        }
    }

    void CodeGenerateVisitor::Visit(NameList *name_list, void *data)
    {
        auto need_init = static_cast<NameListData *>(data)->need_init_;

        auto size = name_list->names_.size();
        for (std::size_t i = 0; i < size; ++i)
        {
            auto register_id = GenerateRegisterId();
            InsertName(name_list->names_[i].str_, register_id);

            // Add init instructions when need
            if (need_init)
            {
                auto function = GetCurrentFunction();
                auto instruction = Instruction::ACode(OpType_LoadNil, register_id);
                function->AddInstruction(instruction, name_list->names_[i].line_);
            }
        }
    }

    void CodeGenerateVisitor::Visit(TableDefine *table, void *data)
    {
        auto exp_var_data = static_cast<ExpVarData *>(data);
        auto register_id = exp_var_data->start_register_;
        auto end_register = exp_var_data->end_register_;

        // No register, then do not generate code
        if (end_register != EXP_VALUE_COUNT_ANY && register_id >= end_register)
            return ;

        // New table
        auto function = GetCurrentFunction();
        auto instruction = Instruction::ACode(OpType_NewTable, register_id);
        function->AddInstruction(instruction, table->line_);

        if (!table->fields_.empty())
        {
            // Init table value
            TableFieldData field_data{ register_id };
            for (auto &field : table->fields_)
            {
                REGISTER_GENERATOR_GUARD();
                field->Accept(this, &field_data);
            }
        }

        FillRemainRegisterNil(register_id + 1, end_register, table->line_);
    }

    void CodeGenerateVisitor::Visit(TableIndexField *field, void *data)
    {
        auto field_data = static_cast<TableFieldData *>(data);
        auto table_register = field_data->table_register_;

        // Load key
        auto key_register = GenerateRegisterId();
        ExpVarData exp_var_data{ key_register, key_register + 1 };
        field->index_->Accept(this, &exp_var_data);

        SetTableFieldValue(field, table_register, key_register, field->line_);
    }

    void CodeGenerateVisitor::Visit(TableNameField *field, void *data)
    {
        auto field_data = static_cast<TableFieldData *>(data);
        auto table_register = field_data->table_register_;

        // Load key
        auto function = GetCurrentFunction();
        auto key_index = function->AddConstString(field->name_.str_);
        auto key_register = GenerateRegisterId();
        auto instruction = Instruction::ABxCode(OpType_LoadConst, key_register, key_index);
        function->AddInstruction(instruction, field->name_.line_);

        SetTableFieldValue(field, table_register, key_register, field->name_.line_);
    }

    void CodeGenerateVisitor::Visit(TableArrayField *field, void *data)
    {
        auto field_data = static_cast<TableFieldData *>(data);
        auto table_register = field_data->table_register_;

        // Load key
        auto function = GetCurrentFunction();
        auto key_register = GenerateRegisterId();
        auto instruction = Instruction::ACode(OpType_LoadInt, key_register);
        function->AddInstruction(instruction, field->line_);
        instruction.opcode_ = field_data->array_index_++;
        function->AddInstruction(instruction, field->line_);

        SetTableFieldValue(field, table_register, key_register, field->line_);
    }

    void CodeGenerateVisitor::Visit(IndexAccessor *accessor, void *data)
    {
        AccessTableField(accessor, data, accessor->line_,
                         [=](int key_register) {
                             ExpVarData data{ key_register, key_register + 1 };
                             accessor->index_->Accept(this, &data);
                         });
    }

    void CodeGenerateVisitor::Visit(MemberAccessor *accessor, void *data)
    {
        AccessTableField(accessor, data, accessor->member_.line_,
                         [=](int key_register) {
                             auto function = GetCurrentFunction();
                             auto key_index = function->
                                AddConstString(accessor->member_.str_);
                             auto instruction = Instruction::
                                ABxCode(OpType_LoadConst, key_register, key_index);
                             function->AddInstruction(instruction,
                                                      accessor->member_.line_);
                         });
    }

    void CodeGenerateVisitor::Visit(NormalFuncCall *func_call, void *data)
    {
        FunctionCall(func_call, data, [](int) { return 0; });
    }

    void CodeGenerateVisitor::Visit(MemberFuncCall *func_call, void *data)
    {
        FunctionCall(func_call, data, [=](int caller_register) {
            auto function = GetCurrentFunction();
            // Copy table to arg_register as first argument
            auto arg_register = GenerateRegisterId();
            auto instruction = Instruction::ABCode(OpType_Move, arg_register, caller_register);
            function->AddInstruction(instruction, func_call->member_.line_);

            {
                REGISTER_GENERATOR_GUARD();
                // Get key
                auto index = function->AddConstString(func_call->member_.str_);
                auto key_register = GenerateRegisterId();
                instruction = Instruction::ABxCode(OpType_LoadConst, key_register, index);
                function->AddInstruction(instruction, func_call->member_.line_);

                // Get caller function from table
                instruction = Instruction::ABCCode(OpType_GetTable, caller_register,
                                                   key_register, caller_register);
                function->AddInstruction(instruction, func_call->member_.line_);
            }

            return 1;
        });
    }

    void CodeGenerateVisitor::Visit(FuncCallArgs *arg, void *data)
    {
        static_cast<FuncCallArgsData *>(data)->arg_value_count_ = arg->arg_value_count_;

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

        // When parent do not limit register count, reset register
        // id generator as consume some registers, and check register
        // count overflow or not
        auto register_consumer = [=](int id) {
            if (end_register == EXP_VALUE_COUNT_ANY)
                ResetRegisterIdGenerator(id);
            if (IsRegisterCountOverflow())
            {
                throw CodeGenerateException(
                    GetCurrentFunction()->GetModule()->GetCStr(), exp_list->line_,
                    "too many local variables or too complex expression");
            }
        };

        assert(!exp_list->exp_list_.empty());
        int count = exp_list->exp_list_.size() - 1;

        // Each expression consume one register
        int i = 0;
        int max_register = end_register == EXP_VALUE_COUNT_ANY ?
            std::numeric_limits<int>::max() : end_register;
        for (; i < count && register_id < max_register; ++i, ++register_id)
        {
            register_consumer(register_id + 1);

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
        register_consumer(register_id + 1);
        REGISTER_GENERATOR_GUARD();
        ExpVarData exp_var_data{ register_id, end_register };
        exp_list->exp_list_.back()->Accept(this, &exp_var_data);
    }

    void CodeGenerate(SyntaxTree *root, State *state)
    {
        assert(root && state);
        CodeGenerateVisitor code_generator(state);
        root->Accept(&code_generator, nullptr);
    }
} // namespace luna
