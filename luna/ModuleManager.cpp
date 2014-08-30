#include "ModuleManager.h"
#include "Lex.h"
#include "Parser.h"
#include "State.h"
#include "Exception.h"
#include "SemanticAnalysis.h"
#include "CodeGenerate.h"
#include "TextInStream.h"
#include <functional>

namespace luna
{
    ModuleManager::ModuleManager(State *state)
        : state_(state)
    {
    }

    void ModuleManager::LoadModule(const std::string &module_name)
    {
        io::text::InStream is(module_name);
        if (!is.IsOpen())
            throw OpenFileFail(module_name);

        Lexer lexer(state_, state_->GetString(module_name),
                    [&is] () { return is.GetChar(); });

        // Parse to AST
        auto ast = Parse(&lexer);

        // Semantic analysis
        SemanticAnalysis(ast.get(), state_);

        // Generate code
        CodeGenerate(ast.get(), state_);
    }
} // namespace luna
