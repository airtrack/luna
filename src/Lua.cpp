#include "Parser.h"
#include "Error.h"
#include "State.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s file\n", argv[0]);
        return 0;
    }

    try
    {
        const char *file = argv[1];
        lua::State state;
        lua::Function *f = state.GetModuleLoader()->LoadModule(file);
        state.GetVM()->Run(f);
        printf("Run %s ok.\n", file);
    } catch (lua::OpenFileError& err)
    {
        printf("Can not open file %s.\n", err.file.c_str());
    } catch (lua::Error& err)
    {
        std::string err_str = lua::Error::ConvertToReadable(err);
        printf("%s\n", err_str.c_str());
    }

    return 0;
}
