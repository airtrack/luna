#include "LexTable.h"

namespace lua
{
    // static variable
    bool KeyWordSet::is_inited_ = false;
    std::map<std::string, TokenType> KeyWordSet::key_words_;

    // KeyWordSet -------------------------------------------------------------

    void KeyWordSet::InitKeyWordSet()
    {
        if (!is_inited_)
        {
            key_words_.insert(std::make_pair("and", KW_AND));
            key_words_.insert(std::make_pair("break", KW_BREAK));
            key_words_.insert(std::make_pair("do", KW_DO));
            key_words_.insert(std::make_pair("else", KW_ELSE));
            key_words_.insert(std::make_pair("elseif", KW_ELSEIF));
            key_words_.insert(std::make_pair("end", KW_END));
            key_words_.insert(std::make_pair("false", KW_FALSE));
            key_words_.insert(std::make_pair("for", KW_FOR));
            key_words_.insert(std::make_pair("function", KW_FUNCTION));
            key_words_.insert(std::make_pair("if", KW_IF));
            key_words_.insert(std::make_pair("in", KW_IN));
            key_words_.insert(std::make_pair("local", KW_LOCAL));
            key_words_.insert(std::make_pair("nil", KW_NIL));
            key_words_.insert(std::make_pair("not", KW_NOT));
            key_words_.insert(std::make_pair("or", KW_OR));
            key_words_.insert(std::make_pair("repeat", KW_REPEAT));
            key_words_.insert(std::make_pair("return", KW_RETUREN));
            key_words_.insert(std::make_pair("then", KW_THEN));
            key_words_.insert(std::make_pair("true", KW_TRUE));
            key_words_.insert(std::make_pair("until", KW_UNTIL));
            key_words_.insert(std::make_pair("while", KW_WHILE));
        }
    }

    TokenType KeyWordSet::GetTokenType(const std::string& token)
    {
        std::map<std::string, TokenType>::iterator it = key_words_.find(token);
        if (it != key_words_.end())
            return it->second;
        return IDENTIFIER;
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

    int LexTable::InsertNewToken(const std::string& value, TokenType type)
    {
        int index = table_.size();

        Token *token = new Token;
        token->value = value;
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
