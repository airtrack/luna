#ifndef LEX_TABLE_H
#define LEX_TABLE_H

#include <string>
#include <set>
#include <vector>

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
        static bool is_inited_;
        static std::set<std::string> key_words_;
    };

    class LexTable
    {
    public:
        struct Token
        {
            // The token value
            std::string value;

            // Start line and column
            int line_number;
            int column_number;

            TokenType type;
        };

        LexTable();
        ~LexTable();

        // Get token by index
        const Token * operator [] (std::size_t index) const;

        // Insert new token into the LexTable, and return the new token's index
        int InsertNewToken(const std::string& value, int line, int column, TokenType type);

        // Clear all tokens
        void Clear();

    private:
        typedef std::vector<Token *> Table;
        Table table_;
    };
} // namespace lua

#endif // LEX_TABLE_H
