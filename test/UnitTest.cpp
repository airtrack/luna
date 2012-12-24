#include "UnitTest.h"

TEST_CASE(case1)
{
    EXPECT_TRUE(true);
}

TEST_CASE(case2)
{
    EXPECT_TRUE(false);
}

int main()
{
    UnitTestManager::GetInstance().RunAllTestCase();
    return 0;
}
