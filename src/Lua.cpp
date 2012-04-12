#include "Parser.h"
#include "Error.h"
#include "State.h"
#include "BuildinFunctions.h"
#include <iostream>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " file" <<std::endl;
        return 0;
    }

    try
    {
        lua::State state;
        buildin::RegisterBuildin(&state);

        const char *file = argv[1];
        lua::Bootstrap *boot = state.GetModuleLoader()->LoadModule(file);
        state.GetVM()->Run(boot);
    } catch (const lua::OpenFileError& err)
    {
        std::cout << "Can not open file " << err.file << std::endl;
    } catch (const lua::Error& err)
    {
        std::cout << lua::Error::ConvertToReadable(err) << std::endl;
    } catch (const lua::RuntimeError& err)
    {
        std::cout << err.info << std::endl;
    }

    return 0;
}
