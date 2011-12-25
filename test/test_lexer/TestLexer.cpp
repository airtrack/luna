#include "Lexer.h"
#include "Error.h"

using namespace lua;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage:%s file\n", argv[0]);
        return 0;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file)
    {
        printf("Can not open file %s.\n", argv[1]);
        return 0;
    }

    Source source(file);
    LexTable lex_table;
    Lexer lexer(&source, &lex_table);

    try
    {
        int index = 0;
        while ((index = lexer.GetToken()) >= 0)
        {
            printf("token[%d] type is %d, value is %s\n", index,
                lex_table[index]->type, lex_table[index]->value.c_str());
        }

        printf("Lex successful!\n");
    } catch (const LexError& error)
    {
        std::string err_str = LexError::ConvertToReadable(error);
        printf("%s\n", err_str.c_str());
    }

    return 0;
}
