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
