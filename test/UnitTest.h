#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include <stdio.h>
#include <vector>
#include <string>

class UnitTestBase;

class UnitTestManager
{
public:
    UnitTestManager(const UnitTestManager&) = delete;
    void operator = (const UnitTestManager&) = delete;

    static UnitTestManager& GetInstance();
    void AddUnitTest(UnitTestBase *test);
    void RunAllTestCase();

private:
    UnitTestManager();

    std::vector<UnitTestBase *> all_;
};

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

UnitTestBase::UnitTestBase()
{
    UnitTestManager::GetInstance().AddUnitTest(this);
}

std::string UnitTestBase::GetTestName() const
{
    return test_name_;
}

std::vector<std::string> UnitTestBase::GetErrors() const
{
    return errors_;
}

bool UnitTestBase::IsTestOK() const
{
    return errors_.empty();
}

UnitTestManager::UnitTestManager()
{
}

UnitTestManager& UnitTestManager::GetInstance()
{
    static UnitTestManager instance;
    return instance;
}

void UnitTestManager::AddUnitTest(UnitTestBase *test)
{
    all_.push_back(test);
}

void UnitTestManager::RunAllTestCase()
{
    int pass = 0;
    int failed = 0;

    for (auto test : all_)
    {
        try
        {
            test->Run();
            if (test->IsTestOK())
            {
                ++pass;
                printf("\033[32m[%s] pass\033[0m\n", test->GetTestName().c_str());
            }
            else
            {
                ++failed;
                printf("\033[31m[%s] failed:\n", test->GetTestName().c_str());

                std::vector<std::string> errors = test->GetErrors();
                for (auto &error : errors)
                {
                    printf("\t%s\n", error.c_str());
                }

                printf("\033[0m");
            }
        }
        catch (...)
        {
            ++failed;
            printf("\033[31m[%s] catch exception\033[0m\n", test->GetTestName().c_str());
        }
    }

    printf("%d cases: %d passed, %d failed\n", pass + failed, pass, failed);
}

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

#endif // UNIT_TEST_H
