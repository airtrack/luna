#include "Parser.h"
#include "Lex.h"
#include "State.h"
#include "Exception.h"
#include <assert.h>

namespace
{
    using namespace luna;

    class ParserImpl
    {
    public:
        ParserImpl(State *state, Lexer *lexer)
            : state_(state), lexer_(lexer)
        {
        }

        std::unique_ptr<SyntaxTree> Parse()
        {
            return ParseExp();
        }

        std::unique_ptr<SyntaxTree> ParseExp(std::unique_ptr<SyntaxTree> left = std::unique_ptr<SyntaxTree>(),
                                             TokenDetail op = TokenDetail(),
                                             int left_priority = 0)
        {
            std::unique_ptr<SyntaxTree> exp;
            LookAhead();

            if (look_ahead_.token_ == '-' || look_ahead_.token_ == '#' || look_ahead_.token_ == Token_Not)
            {
                NextToken();
                std::unique_ptr<UnaryExpression> unexp(new UnaryExpression);
                unexp->op_token_ = current_;
                unexp->exp_ = ParseExp(std::unique_ptr<SyntaxTree>(), TokenDetail(), 90);
                exp = std::move(unexp);
            }
            else if (IsMainExp(look_ahead_))
                exp = ParseMainExp();
            else
                throw ParseException("unexpect token for exp.", look_ahead_);

            while (true)
            {
                int right_priority = GetOpPriority(LookAhead());
                if (left_priority < right_priority)
                {
                    exp = ParseExp(std::move(exp), NextToken(), right_priority);
                }
                else if (left_priority == right_priority)
                {
                    if (left_priority == 0)
                        return exp;
                    assert(left);
                    left = std::unique_ptr<BinaryExpression>(
                        new BinaryExpression(std::move(left), std::move(exp), op));
                    op = NextToken();
                    exp = ParseMainExp();
                }
                else
                {
                    if (left)
                        exp = std::unique_ptr<BinaryExpression>(
                            new BinaryExpression(std::move(left), std::move(exp), op));
                    return exp;
                }
            }
        }

        std::unique_ptr<SyntaxTree> ParseMainExp()
        {
            std::unique_ptr<SyntaxTree> exp;

            NextToken();
            switch (current_.token_)
            {
                case Token_Nil:
                case Token_False:
                case Token_True:
                case Token_Number:
                case Token_String:
                case Token_VarArg:
                    exp.reset(new Terminator(current_));
                    break;
                default:
                    throw ParseException("unexpect token for exp.", current_);
            }

            return exp;
        }

    private:
        TokenDetail& NextToken()
        {
            if (look_ahead_.token_ != Token_EOF)
            {
                current_ = look_ahead_;
                look_ahead_ = TokenDetail();
            }
            else
            {
                lexer_->GetToken(&current_);
            }

            return current_;
        }

        TokenDetail& LookAhead()
        {
            if (look_ahead_.token_ == Token_EOF)
                lexer_->GetToken(&look_ahead_);
            return look_ahead_;
        }

        bool IsMainExp(const TokenDetail &t) const
        {
            int token = t.token_;
            return
                token == Token_Nil ||
                token == Token_False ||
                token == Token_True ||
                token == Token_Number ||
                token == Token_String ||
                token == Token_VarArg ||
                token == Token_Function ||
                token == Token_Id ||
                token == '(' ||
                token == '{';
        }

        int GetOpPriority(const TokenDetail &t) const
        {
            switch (t.token_)
            {
                case '^': return 100;
                case '*':
                case '/':
                case '%': return 80;
                case '+':
                case '-': return 70;
                case Token_Concat: return 60;
                case '>':
                case '<':
                case Token_BigEqual:
                case Token_LessEqual:
                case Token_NotEqual:
                case Token_Equal: return 50;
                case Token_And: return 40;
                case Token_Or: return 30;
                default: return 0;
            }
        }

        State *state_;
        Lexer *lexer_;
        TokenDetail current_;
        TokenDetail look_ahead_;
    };
} // namespace

namespace luna
{
    Parser::Parser(State *state)
        : state_(state)
    {
    }

    std::unique_ptr<SyntaxTree> Parser::Parse(Lexer *lexer)
    {
        ParserImpl impl(state_, lexer);
        return impl.Parse();
    }
} // namespace luna
