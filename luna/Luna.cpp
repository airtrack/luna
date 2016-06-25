#include "State.h"
#include "Exception.h"
#include "LibBase.h"
#include "LibIO.h"
#include "LibMath.h"
#include "LibString.h"
#include "LibTable.h"
#include <stdio.h>

void Repl(luna::State &state)
{
    printf("Luna 2.0 Copyright (C) 2014\n");

    for (;;)
    {
        try
        {
            printf("> ");

            char s[1024] = { 0 };
            auto input = fgets(s, sizeof(s), stdin);
            if (!input)
                break;
            state.DoString(input, "stdin");
        }
        catch (const luna::Exception &exp)
        {
            printf("%s\n", exp.What().c_str());
        }
    }
}

void ExecuteFile(const char **argv, luna::State &state)
{
    try
    {
        state.DoModule(argv[1]);
    }
    catch (const luna::OpenFileFail &exp)
    {
        printf("%s: can not open file %s\n", argv[0], exp.What().c_str());
    }
    catch (const luna::Exception &exp)
    {
        printf("%s\n", exp.What().c_str());
    }
}

int main(int argc, const char **argv)
{
    luna::State state;

    lib::base::RegisterLibBase(&state);
    lib::io::RegisterLibIO(&state);
    lib::math::RegisterLibMath(&state);
    lib::string::RegisterLibString(&state);
    lib::table::RegisterLibTable(&state);

    if (argc < 2)
    {
        Repl(state);
    }
    else
    {
        ExecuteFile(argv, state);
    }

    return 0;
}
