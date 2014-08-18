#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include <vector>
#include <string>

class UnitTestBase
{
public:
    UnitTestBase();

    UnitTestBase(const UnitTestBase&) = delete;
    void operator = (const UnitTestBase&) = delete;

    std::string GetTestName() const;
    std::vector<std::string> GetErrors() const;
    bool IsTestOK() const;
    virtual void Run() = 0;

protected:
    void Error(const std::string& error)
    {
        errors_.push_back(error + " failed!");
    }

    std::string test_name_;
    std::vector<std::string> errors_;
};

#define TEST_CASE(case_name)                                \
    class UnitTest_##case_name : public UnitTestBase        \
    {                                                       \
    public:                                                 \
        UnitTest_##case_name();                             \
        virtual void Run();                                 \
    } test_##case_name##obj;                                \
                                                            \
    UnitTest_##case_name::UnitTest_##case_name()            \
    {                                                       \
        test_name_ = #case_name;                            \
    }                                                       \
                                                            \
    void UnitTest_##case_name::Run()

#define EXPECT_TRUE(expr)                                   \
    do                                                      \
    {                                                       \
        if (!(expr))                                        \
            Error("'" #expr "'");                           \
    } while (0)

#define EXPECT_EXCEPTION(exception, stmt)                   \
    do                                                      \
    {                                                       \
        try                                                 \
        {                                                   \
            {stmt}                                          \
            Error("'" #stmt "' has no " #exception);        \
        }                                                   \
        catch (const exception&)                            \
        {                                                   \
        }                                                   \
    } while (0)

#endif // UNIT_TEST_H
