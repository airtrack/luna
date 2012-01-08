#include "Parser.h"
#include "Error.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s file\n", argv[0]);
        return 0;
    }
#ifdef _MSC_VER
    FILE *file = 0;
    fopen_s(&file, argv[1], "r");
#else
    FILE *file = fopen(argv[1], "r");
#endif
    if (!file)
    {
        printf("Can not open file %s.\n", argv[1]);
        return 0;
    }

    try
    {
        lua::Source source(file);
        lua::Parser parser(&source);
        parser.Parse();
        printf("Parse ok, construct a parse tree.\n");
    } catch (lua::LexError& err)
    {
        std::string err_str = lua::LexError::ConvertToReadable(err);
        printf("%s\n", err_str.c_str());
    } catch (lua::ParserError& err)
    {
        std::string err_str = lua::ParserError::ConvertToReadable(err);
        printf("%s\n", err_str.c_str());
    }

    return 0;
}
