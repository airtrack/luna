#include "UnitTest.h"
#include <stdio.h>

class UnitTestManager
{
public:
    UnitTestManager(const UnitTestManager&) = delete;
    void operator = (const UnitTestManager&) = delete;

    static UnitTestManager& GetInstance()
    {
        static UnitTestManager instance;
        return instance;
    }
    
    void AddUnitTest(UnitTestBase *test)
    {
        all_.push_back(test);
    }
    
    void RunAllTestCase()
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

private:
    UnitTestManager() { }

    std::vector<UnitTestBase *> all_;
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

int main()
{
    UnitTestManager::GetInstance().RunAllTestCase();
    return 0;
}
