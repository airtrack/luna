#include "UnitTest.h"
#include "../src/String.h"
#include "../src/StringPool.h"

TEST_CASE(string1)
{
    luna::String str1("abc");
    luna::String str2("abc");

    EXPECT_TRUE(str1 == str2);
    EXPECT_TRUE(str1 <= str2);
    EXPECT_TRUE(str1 >= str2);
    EXPECT_TRUE(str1.GetHash() == str2.GetHash());
    EXPECT_TRUE(str1.GetLength() == str2.GetLength());
    EXPECT_TRUE(str1.GetStdString() == str2.GetStdString());

    str1.SetValue("abcdefghijklmn");
    str2.SetValue("abcdefghijklmnopqrst");
    EXPECT_TRUE(str1 != str2);
    EXPECT_TRUE(str1 < str2);
    EXPECT_TRUE(str2 > str1);
    EXPECT_TRUE(str1.GetLength() < str2.GetLength());
    EXPECT_TRUE(str1.GetStdString() != str2.GetStdString());

    str1.SetValue("abc");
    str2.SetValue("def");
    EXPECT_TRUE(str1 != str2);
    EXPECT_TRUE(str1 < str2);
    EXPECT_TRUE(str2 > str1);
    EXPECT_TRUE(str1.GetLength() == str2.GetLength());
    EXPECT_TRUE(str1.GetStdString() != str2.GetStdString());
}

TEST_CASE(string2)
{
    luna::String str1("abc");
    luna::String str2("def");
    luna::String str3("abcdefghijklmn");
    luna::String str4("abcdefghijklmnopqrst");
    luna::StringPool pool;

    pool.AddString(&str1);
    pool.AddString(&str2);
    pool.AddString(&str3);
    pool.AddString(&str4);

    auto s1 = pool.GetString("abc");
    auto s2 = pool.GetString("def");
    auto s3 = pool.GetString("abcdefghijklmn");
    auto s4 = pool.GetString("abcdefghijklmnopqrst");
    EXPECT_TRUE(s1 == &str1);
    EXPECT_TRUE(s2 == &str2);
    EXPECT_TRUE(s3 == &str3);
    EXPECT_TRUE(s4 == &str4);

    auto s5 = pool.GetString("abcdef");
    EXPECT_TRUE(!s5);

    pool.DeleteString(&str1);
    pool.DeleteString(&str2);
    pool.DeleteString(&str3);
    pool.DeleteString(&str4);

    s1 = pool.GetString("abc");
    s2 = pool.GetString("def");
    s3 = pool.GetString("abcdefghijklmn");
    s4 = pool.GetString("abcdefghijklmnopqrst");
    EXPECT_TRUE(!s1);
    EXPECT_TRUE(!s2);
    EXPECT_TRUE(!s3);
    EXPECT_TRUE(!s4);
}
