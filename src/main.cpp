#include <iostream>

#include "commander_cpp.hpp"

using namespace COMMANDER_CPP;

struct TestResult
{
    bool result;
    std::string errMsg;
};

class Test
{
public:
    virtual std::string id() = 0;
    virtual TestResult test() = 0;
};

class VersionTest : public Command, public Test
{
public:
    class LoggerImpl : public Logger
    {
    public:
        virtual Logger *print(const String &msg) override
        {
            if(msg != "1.2.3"){
                this->result = {false, "version is not 1.2.3"};
            }
            return this;
        };
        TestResult result = {true, ""};
    };

    VersionTest() : Command("",new LoggerImpl())
    {
        this->name(id())
            ->version("1.2.3");
    }
    virtual std::string id() override { return "VersionTest"; }
    virtual TestResult test() override
    {
        char* argv[] = {"testCommand", "--version"};
        this->parse(2, argv);
        char* argv1[] = {"testCommand", "--V"};
        this->parse(2, argv1);

        return static_cast<LoggerImpl*>(this->pLogger)->result;
    }
};

class DescriptionTest : public Command, public Test
{
public:
    class LoggerImpl : public Logger
    {
    public:
        virtual Logger *print(const String &msg) override
        {
            // std::cout << msg << std::endl;
            std::smatch res;
            std::regex reg(R"(测试命令行解析。)");
            if(!std::regex_search(msg, res, reg)){
                this->result = {false, "description is not: 测试命令行解析。"};
            }
            return this;
        };
        TestResult result = {true, ""};
    };

    DescriptionTest() : Command("",new LoggerImpl())
    {
        this->name(id())
            ->description("测试命令行解析。");
    }
    virtual std::string id() override { return "DescriptionTest"; }
    virtual TestResult test() override
    {
        char* argv[] = {"testCommand"};
        this->parse(1, argv);
        char* argv1[] = {"testCommand", "-h"};
        this->parse(2, argv1);
        char* argv2[] = {"testCommand", "--help"};
        this->parse(2, argv2);

        return static_cast<LoggerImpl *>(this->pLogger)->result;
    }
};

int main(int argc, char **argv)
{
    Command("test").option("-i --display-success-info", "显示信息")
    ->action([](Vector<Variant> args,Map<String, Variant> opts){
        Test *tests[] = {
            new VersionTest(),
            new DescriptionTest()};

        for (int i = 0; i < std::size(tests); i++)
        {
            TestResult res = tests[i]->test();
            if (!res.result)
            {
                std::cout << "test :" << tests[i]->id() << " failed: " << res.errMsg << std::endl;
            }
            else
            {
                if(opts.find("display-success-info") != opts.end())
                {
                    std::cout << "test :" << tests[i]->id() << " passed" << std::endl;
                }
            }
        } 
    })
    ->parse(argc, argv);
    return 0;
}
