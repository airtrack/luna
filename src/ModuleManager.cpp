#include "ModuleManager.h"
#include "Lex.h"
#include "Parser.h"
#include "State.h"
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
        Lexer lexer(state_, state_->GetString(module_name),
                    std::bind(&io::text::InStream::GetChar, &is));

        Parser parser(state_);
        parser.Parse(&lexer);
    }
} // namespace luna
