#include "UnitTest.h"
#include "TestCommon.h"
#include "SemanticAnalysis.h"

namespace
{
    std::unique_ptr<luna::SyntaxTree> Semantic(const std::string &s)
    {
        ParserWrapper parser(s);
        auto ast = parser.Parse();
        luna::SemanticAnalysis(ast.get());
        return std::move(ast);
    }
} // namespace

TEST_CASE(semantic1)
{
    auto ast = Semantic("a, b = c, d");
    auto a = ASTFind<luna::Terminator>(ast, NameFinder("a"));
    auto b = ASTFind<luna::Terminator>(ast, NameFinder("b"));
    auto c = ASTFind<luna::Terminator>(ast, NameFinder("c"));
    auto d = ASTFind<luna::Terminator>(ast, NameFinder("d"));
    EXPECT_TRUE(a->semantic_ == luna::SemanticOp_Write);
    EXPECT_TRUE(b->semantic_ == luna::SemanticOp_Write);
    EXPECT_TRUE(c->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(d->semantic_ == luna::SemanticOp_Read);
}

TEST_CASE(semantic2)
{
    auto ast = Semantic("f(a, b)");
    auto f = ASTFind<luna::Terminator>(ast, NameFinder("f"));
    auto a = ASTFind<luna::Terminator>(ast, NameFinder("a"));
    auto b = ASTFind<luna::Terminator>(ast, NameFinder("b"));
    EXPECT_TRUE(f->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(a->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(b->semantic_ == luna::SemanticOp_Read);
}

TEST_CASE(semantic3)
{
    auto ast = Semantic("m:f(a, b)");
    auto m = ASTFind<luna::Terminator>(ast, NameFinder("m"));
    auto a = ASTFind<luna::Terminator>(ast, NameFinder("a"));
    auto b = ASTFind<luna::Terminator>(ast, NameFinder("b"));
    EXPECT_TRUE(m->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(a->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(b->semantic_ == luna::SemanticOp_Read);
}

TEST_CASE(semantic4)
{
    auto ast = Semantic("t.m.n = a");
    auto t_m_n = ASTFind<luna::MemberAccessor>(ast, [](luna::MemberAccessor *ma) {
        return ma->member_.str_->GetStdString() == "n";
    });
    auto t_m = ASTFind<luna::MemberAccessor>(ast, [](luna::MemberAccessor *ma) {
        return ma->member_.str_->GetStdString() == "m";
    });
    auto t = ASTFind<luna::Terminator>(ast, NameFinder("t"));
    auto a = ASTFind<luna::Terminator>(ast, NameFinder("a"));
    EXPECT_TRUE(t_m_n->semantic_ == luna::SemanticOp_Write);
    EXPECT_TRUE(t_m->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(t->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(a->semantic_ == luna::SemanticOp_Read);
}

TEST_CASE(semantic5)
{
    auto ast = Semantic("t[i][j] = a");
    auto t_i_j = ASTFind<luna::IndexAccessor>(ast, [](luna::IndexAccessor *) {
        return true;
    });
    auto t_i = ASTFind<luna::IndexAccessor>(t_i_j->table_, [](luna::IndexAccessor *) {
        return true;
    });
    auto t = ASTFind<luna::Terminator>(ast, NameFinder("t"));
    auto i = ASTFind<luna::Terminator>(ast, NameFinder("i"));
    auto j = ASTFind<luna::Terminator>(ast, NameFinder("j"));
    auto a = ASTFind<luna::Terminator>(ast, NameFinder("a"));
    EXPECT_TRUE(t_i_j->semantic_ == luna::SemanticOp_Write);
    EXPECT_TRUE(t_i->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(t->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(i->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(j->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(a->semantic_ == luna::SemanticOp_Read);
}

TEST_CASE(semantic6)
{
    auto ast = Semantic("a = t.m.n");
    auto t_m_n = ASTFind<luna::MemberAccessor>(ast, [](luna::MemberAccessor *ma) {
        return ma->member_.str_->GetStdString() == "n";
    });
    auto t_m = ASTFind<luna::MemberAccessor>(ast, [](luna::MemberAccessor *ma) {
        return ma->member_.str_->GetStdString() == "m";
    });
    auto t = ASTFind<luna::Terminator>(ast, NameFinder("t"));
    auto a = ASTFind<luna::Terminator>(ast, NameFinder("a"));
    EXPECT_TRUE(t_m_n->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(t_m->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(t->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(a->semantic_ == luna::SemanticOp_Write);
}

TEST_CASE(semantic7)
{
    auto ast = Semantic("a = t[i][j]");
    auto t_i_j = ASTFind<luna::IndexAccessor>(ast, [](luna::IndexAccessor *) {
        return true;
    });
    auto t_i = ASTFind<luna::IndexAccessor>(t_i_j->table_, [](luna::IndexAccessor *) {
        return true;
    });
    auto t = ASTFind<luna::Terminator>(ast, NameFinder("t"));
    auto i = ASTFind<luna::Terminator>(ast, NameFinder("i"));
    auto j = ASTFind<luna::Terminator>(ast, NameFinder("j"));
    auto a = ASTFind<luna::Terminator>(ast, NameFinder("a"));
    EXPECT_TRUE(t_i_j->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(t_i->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(t->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(i->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(j->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(a->semantic_ == luna::SemanticOp_Write);
}

TEST_CASE(semantic8)
{
    auto ast = Semantic("t = { [i] = a, m = b, c }");
    auto i = ASTFind<luna::Terminator>(ast, NameFinder("i"));
    auto a = ASTFind<luna::Terminator>(ast, NameFinder("a"));
    auto b = ASTFind<luna::Terminator>(ast, NameFinder("b"));
    auto c = ASTFind<luna::Terminator>(ast, NameFinder("c"));
    EXPECT_TRUE(i->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(a->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(b->semantic_ == luna::SemanticOp_Read);
    EXPECT_TRUE(c->semantic_ == luna::SemanticOp_Read);
}

TEST_CASE(semantic9)
{
    EXPECT_TRUE(Semantic("a = 1 + 1"));
    EXPECT_TRUE(Semantic("a = 1 + b"));
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = 1 - {}");
    });
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = 1 * nil");
    });
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = 1 / true");
    });
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = 1 % 'str'");
    });
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = 1 ^ ...");
    });
}

TEST_CASE(semantic10)
{
    EXPECT_TRUE(Semantic("a = 1 > 2"));
    EXPECT_TRUE(Semantic("a = 'str' >= 'str'"));
    EXPECT_TRUE(Semantic("a = 1 > b"));
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = 1 < 'str'");
    });
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = true <= false");
    });
}

TEST_CASE(semantic11)
{
    EXPECT_TRUE(Semantic("a = 'str' .. 'str'"));
    EXPECT_TRUE(Semantic("a = 1 .. 'str'"));
    EXPECT_TRUE(Semantic("a = 'str' .. 1"));
    EXPECT_TRUE(Semantic("a = 'str' .. b"));
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = 1 .. 1");
    });
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = true .. nil");
    });
}

TEST_CASE(semantic12)
{
    EXPECT_TRUE(Semantic("a = -(1 + 1)"));
    EXPECT_TRUE(Semantic("a = #{1, 2, 3}"));
    EXPECT_TRUE(Semantic("a = #'str'"));
    EXPECT_TRUE(Semantic("a = not a"));
    EXPECT_TRUE(Semantic("a = -a"));
    EXPECT_TRUE(Semantic("a = #a"));
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = -'str'");
    });
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = #1");
    });
}

TEST_CASE(semantic13)
{
    EXPECT_TRUE(Semantic("a = -#{1, 2, 3} + 1"));
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = (1 > 2) + 1");
    });
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = (1 ~= 2) > 1");
    });
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("a = not a >= true");
    });
}

TEST_CASE(semantic14)
{
    auto ast = Semantic("function test() "
                        "    local a = 1 "
                        "    a = f() "
                        "    return function() return a end "
                        "end");

    auto a = ASTFind<luna::Terminator>(ast, NameFinder("a"));
    auto f = ASTFind<luna::Terminator>(ast, NameFinder("f"));
    EXPECT_TRUE(a->scoping_ == luna::LexicalScoping_Local);
    EXPECT_TRUE(f->scoping_ == luna::LexicalScoping_Global);

    auto ret = ASTFind<luna::ReturnStatement>(ast, [](luna::ReturnStatement *) {
        return true;
    });
    auto a2 = ASTFind<luna::Terminator>(ret->exp_list_, NameFinder("a"));
    EXPECT_TRUE(a2->scoping_ == luna::LexicalScoping_Upvalue);
}

TEST_CASE(semantic15)
{
    auto ast = Semantic("for i = 1, 10 do print(i) end");
    auto i = ASTFind<luna::Terminator>(ast, NameFinder("i"));
    EXPECT_TRUE(i->scoping_ == luna::LexicalScoping_Local);
}

TEST_CASE(semantic16)
{
    auto ast = Semantic("for i, j in f() do print(i, j) end");
    auto i = ASTFind<luna::Terminator>(ast, NameFinder("i"));
    auto j = ASTFind<luna::Terminator>(ast, NameFinder("j"));
    EXPECT_TRUE(i->scoping_ == luna::LexicalScoping_Local);
    EXPECT_TRUE(j->scoping_ == luna::LexicalScoping_Local);
}

TEST_CASE(semantic17)
{
    auto ast = Semantic("repeat local i = 1 until i == 1");
    auto i = ASTFind<luna::Terminator>(ast, NameFinder("i"));
    EXPECT_TRUE(i->scoping_ == luna::LexicalScoping_Local);
}

TEST_CASE(semantic18)
{
    auto ast = Semantic("while i == 1 do local i = 1 end");
    auto i = ASTFind<luna::Terminator>(ast, NameFinder("i"));
    EXPECT_TRUE(i->scoping_ == luna::LexicalScoping_Global);
}

TEST_CASE(semantic19)
{
    auto ast = Semantic("if i == 1 then local i = 1 elseif j == 1 then local j = 1 end");
    auto i = ASTFind<luna::Terminator>(ast, NameFinder("i"));
    auto j = ASTFind<luna::Terminator>(ast, NameFinder("j"));
    EXPECT_TRUE(i->scoping_ == luna::LexicalScoping_Global);
    EXPECT_TRUE(j->scoping_ == luna::LexicalScoping_Global);
}

TEST_CASE(semantic20)
{
    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("do break end");
    });

    EXPECT_EXCEPTION(luna::SemanticException, {
        Semantic("while true do local f = function() break end end");
    });

    auto ast = Semantic("while true do break end");
    auto b = ASTFind<luna::BreakStatement>(ast, [](luna::BreakStatement *) {
        return true;
    });
    auto w = ASTFind<luna::WhileStatement>(ast, [](luna::WhileStatement *) {
        return true;
    });
    EXPECT_TRUE(b->loop_ == w);

    ast = Semantic("repeat break until true");
    b = ASTFind<luna::BreakStatement>(ast, [](luna::BreakStatement *) {
        return true;
    });
    auto r = ASTFind<luna::RepeatStatement>(ast, [](luna::RepeatStatement *) {
        return true;
    });
    EXPECT_TRUE(b->loop_ == r);

    ast = Semantic("for i = 1, 10 do break end");
    b = ASTFind<luna::BreakStatement>(ast, [](luna::BreakStatement *) {
        return true;
    });
    auto nf = ASTFind<luna::NumericForStatement>(ast, [](luna::NumericForStatement *) {
        return true;
    });
    EXPECT_TRUE(b->loop_ == nf);

    ast = Semantic("for k, v in pairs(t) do break end");
    b = ASTFind<luna::BreakStatement>(ast, [](luna::BreakStatement *) {
        return true;
    });
    auto gf = ASTFind<luna::GenericForStatement>(ast, [](luna::GenericForStatement *) {
        return true;
    });
    EXPECT_TRUE(b->loop_ == gf);
}
