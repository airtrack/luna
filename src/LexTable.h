#ifndef LEX_TABLE_H
#define LEX_TABLE_H

#include <string>
#include <set>

namespace lua
{
    enum TokenType
    {
        KEY_WORD,
        OPERATOR,
        STRING,
        NUMBER,
    };

    class KeyWordSet
    {
    public:
        static void InitKeyWordSet();
        static bool IsKeyWord(const std::string& token);

    private:
        static std::set<std::string> key_words_;
    };

    class LexTable
    {
    public:
        struct Token
        {
            std::string value;
            int line_number;
            int column_number;
            TokenType type;
        };

        const Token * operator [] (std::size_t index) const;
        int InsertNewToken(const std::string& value, int line, int column, TokenType type);
    };
} // namespace lua

#endif // LEX_TABLE_H
