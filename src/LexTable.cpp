#include "LexTable.h"

namespace
{
    const char *lua_key_word[] = {
        "and",
        "break",
        "do",
        "else",
        "elseif",
        "end",
        "false",
        "for",
        "function",
        "if",
        "in",
        "local",
        "nil",
        "not",
        "or",
        "repeat",
        "return",
        "then",
        "true",
        "until",
        "while"
    };
} // namespace

namespace lua
{
    // static variable
    bool KeyWordSet::is_inited_ = false;
    std::set<std::string> KeyWordSet::key_words_;

    // KeyWordSet -------------------------------------------------------------

    void KeyWordSet::InitKeyWordSet()
    {
        if (!is_inited_)
        {
            int total = sizeof(lua_key_word) / sizeof(lua_key_word[0]);
            for (int i = 0; i < total; ++i)
                key_words_.insert(lua_key_word[i]);
            is_inited_ = true;
        }
    }

    bool KeyWordSet::IsKeyWord(const std::string& token)
    {
        return key_words_.find(token) != key_words_.end();
    }

    // LexTable ---------------------------------------------------------------

    LexTable::LexTable()
    {
    }

    LexTable::~LexTable()
    {
        Clear();
    }

    const LexTable::Token * LexTable::operator[] (std::size_t index) const
    {
        return table_[index];
    }

    int LexTable::InsertNewToken(const std::string& value, int line, int column, TokenType type)
    {
        int index = table_.size();

        Token *token = new Token;
        token->value = value;
        token->line_number = line;
        token->column_number = column;
        token->type = type;
        table_.push_back(token);

        return index;
    }

    void LexTable::Clear()
    {
        for (Table::iterator it = table_.begin(); it != table_.end(); ++it)
        {
            delete *it;
        }

        table_.clear();
    }
} // namespace lua
