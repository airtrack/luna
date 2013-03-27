#include "State.h"
#include "VM.h"
#include "Bootstrap.h"
#include "Exception.h"
#include "BaseLib.h"
#include <stdio.h>

int main(int argc, const char **argv)
{
    if (argc < 2)
    {
        printf("usage: %s file\n", argv[0]);
        return 0;
    }

    try
    {
        luna::State state;
        luna::VM vm(&state);
        luna::Bootstrap bootstrap(&state);

        lib::base::RegisterBaseLib(&state);

        state.LoadModule(argv[1]);
        bootstrap.Prepare();
        vm.Execute();

        printf("Execute %s ok.\n", argv[1]);
    }
    catch (const luna::Exception &exp)
    {
        printf("load %s fail:\n\t%s\n", argv[1], exp.What().c_str());
    }

    return 0;
}
