#include "ModuleManager.h"
#include "Lex.h"
#include "Parser.h"
#include "State.h"
#include "Table.h"
#include "Exception.h"
#include "SemanticAnalysis.h"
#include "CodeGenerate.h"
#include "TextInStream.h"
#include <functional>

namespace luna
{
    ModuleManager::ModuleManager(State *state, Table *modules)
        : state_(state), modules_(modules)
    {
    }

    bool ModuleManager::IsLoaded(const std::string &module_name) const
    {
        auto value = GetModuleClosure(module_name);
        return !value.IsNil();
    }

    Value ModuleManager::GetModuleClosure(const std::string &module_name) const
    {
        Value key(state_->GetString(module_name));
        return modules_->GetValue(key);
    }

    void ModuleManager::LoadModule(const std::string &module_name)
    {
        io::text::InStream is(module_name);
        if (!is.IsOpen())
            throw OpenFileFail(module_name);

        Lexer lexer(state_, state_->GetString(module_name),
                    [&is] () { return is.GetChar(); });
        Load(lexer);

        // Add to modules' table
        Value key(state_->GetString(module_name));
        Value value = *(state_->stack_.top_ - 1);
        modules_->SetValue(key, value);
    }

    void ModuleManager::LoadString(const std::string &str, const std::string &name)
    {
        io::text::InStringStream is(str);
        Lexer lexer(state_, state_->GetString(name),
                    [&is] () { return is.GetChar(); });
        Load(lexer);
    }

    void ModuleManager::Load(Lexer &lexer)
    {
        // Parse to AST
        auto ast = Parse(&lexer);

        // Semantic analysis
        SemanticAnalysis(ast.get(), state_);

        // Generate code
        CodeGenerate(ast.get(), state_);
    }
} // namespace luna
