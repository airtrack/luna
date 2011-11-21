#ifndef LEX_TABLE_H
#define LEX_TABLE_H

#include <string>
#include <map>
#include <vector>

namespace lua
{
    enum TokenType
    {
        KEY_WORD,
        KW_AND,
        KW_BREAK,
        KW_DO,
        KW_ELSE,
        KW_ELSEIF,
        KW_END,
        KW_FALSE,
        KW_FOR,
        KW_FUNCTION,
        KW_IF,
        KW_IN,
        KW_LOCAL,
        KW_NIL,
        KW_NOT,
        KW_OR,
        KW_REPEAT,
        KW_RETUREN,
        KW_THEN,
        KW_TRUE,
        KW_UNTIL,
        KW_WHILE,
        STRING,
        NUMBER,
        IDENTIFIER,
        OPERATOR,
        OP_ADD,
        OP_MINUS,
        OP_MULTIPLY,
        OP_DIVIDE,
        OP_REMAINDER,
        OP_POWER,
        OP_COMMA,
        OP_SEMICOLON,
        OP_POUND,
        OP_LEFT_PARENTHESE,
        OP_RIGHT_PARENTHESE,
        OP_LEFT_BRACKET,
        OP_RIGHT_BRACKET,
        OP_LEFT_BRACE,
        OP_RIGHT_BRACE,
        OP_ASSIGN,
        OP_LESS,
        OP_GREATER,
        OP_EQUAL,
        OP_LESSEQUAL,
        OP_GREATEREQUAL,
        OP_NOTEQUAL,
        OP_DOT,
        OP_MERGE,
        OP_PARAM_LIST,
        OP_COLON,
    };

    class KeyWordSet
    {
    public:
        static void InitKeyWordSet();
        static TokenType GetTokenType(const std::string& token);

    private:
        static bool is_inited_;
        static std::map<std::string, TokenType> key_words_;
    };

    class LexTable
    {
    public:
        struct Token
        {
            // The token value
            std::string value;
            TokenType type;
        };

        LexTable();
        ~LexTable();

        // Get token by index
        const Token * operator [] (std::size_t index) const;

        // Insert new token into the LexTable, and return the new token's index
        int InsertNewToken(const std::string& value, TokenType type);

        // Clear all tokens
        void Clear();

    private:
        typedef std::vector<Token *> Table;
        Table table_;
    };
} // namespace lua

#endif // LEX_TABLE_H
