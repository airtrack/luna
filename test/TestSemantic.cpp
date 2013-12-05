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
    auto ast = Semantic("a = b");
}
