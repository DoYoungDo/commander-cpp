#include <iostream>
#include <functional>

#include "commander_cpp.hpp"

using namespace COMMANDER_CPP;

struct TestResult
{
    bool result;
    std::string errMsg;

    TestResult merge(TestResult other)
    {
        this->result = this->result && other.result;
        this->errMsg += this->errMsg.empty() ? other.errMsg : ("\n" + other.errMsg);
        return *this;
    }
};

class Test
{
public:
    virtual std::string id() = 0;
    virtual TestResult test() = 0;
};
class TestLogger : public Logger
{
public:
    std::function<void(const std::string &msg)> checkDebug = nullptr;
    std::function<void(const std::string &msg)> checkWarn = nullptr;
    std::function<void(const std::string &msg)> checkError = nullptr;
    std::function<void(const std::string &msg)> checkPrint = nullptr;

    virtual Logger *debug(const String &msg)
    {
        if(checkDebug != nullptr){
            checkDebug(msg);
        }
        return this;
    }
    virtual Logger *warn(const String &msg)
    {
        if(checkWarn != nullptr){
            checkWarn(msg);
        }
        return this;
    }
    virtual Logger *error(const String &msg)
    {
        if(checkError != nullptr){
            checkError(msg);
        }
        return this;
    }
    virtual Logger *print(const String &msg) {
        if(checkPrint != nullptr){
            checkPrint(msg);
        }
        return this;
    }
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

class OptionTest : public Command , public Test
{
public:
    OptionTest() : Command("",new TestLogger())
    {
        this->name(id())
            ->description("测试选项解析。")
            ->option("-s --mustSimgleOption <values>", "必须单个值")
            ->option("-m --mustMultiOption <values...>", "至少一个值");
    }
    virtual std::string id() override { return "OptionTest"; }
    virtual TestResult test() override
    {
        std::vector<TestResult> results;
        TestLogger *logger = static_cast<TestLogger*>(this->pLogger);

        do
        {
            bool hasError = false;
            logger->checkError = [&](const std::string &msg)
            {
                // std::cout << msg << std::endl;
                if (msg == "option: mustSimgleOption need a value, but got zero.")
                {
                    logger->checkError = nullptr;
                    hasError = true;
                }
            };

            char *argv[] = {"testCommand", "-s"};
            this->parse(2, argv);

            results.push_back(hasError ? TestResult{true, ""} : TestResult{false, "未正确报错：option: mustSimgleOption need a value, but got zero."});
        } while (false);

        do
        {
            bool hasError = false;
            logger->checkWarn = [&](const std::string &msg)
            {
                // std::cout << msg << std::endl;
                if (msg == "unknown identifier: otherValue")
                {
                    logger->checkError = nullptr;
                    hasError = true;
                }
            };

            char *argv[] = {"testCommand", "-s", "value", "otherValue"};
            this->parse(4, argv);

            results.push_back(hasError ? TestResult{true, ""} : TestResult{false, "未正确报错：unknown identifier: otherValue"});
        } while (false);

        do
        {
            logger->checkWarn = [&](const std::string &msg)
            {
                results.push_back(TestResult{false, "意外的警告：" + msg});
            };
            logger->checkError = [&](const std::string &msg)
            {
                results.push_back(TestResult{false, "意外的错误：" + msg});
            };
            this->action([&](Vector<Variant> args, Map<String, Variant> opts)
                         {
                             auto it = opts.find("mustSimgleOption");
                             if (it == opts.end())
                             {
                                 results.push_back(TestResult{false, "未解析到option: mustMultiOption"});
                                 return;
                             }

                             if (!std::holds_alternative<String>(it->second))
                             {
                                 results.push_back(TestResult{false, "option: mustMultiOption,未解析到值"});
                                 return;
                             }
                             
                             std::string * v = std::get_if<String>(&it->second);
                             if (!v)
                             {
                                 results.push_back(TestResult{false, "option: mustMultiOption 的值不是一个字符串"});
                                 return;
                             }

                             if(*v != "value")
                             {
                                 results.push_back(TestResult{false, "option: mustMultiOption 的值不是预期值:value"});
                             } });

            char *argv[] = {"testCommand", "-s", "value"};
            this->parse(3, argv);
        } while (false);


        TestResult result({true, ""});
        for(auto res : results){
            result = result.merge(res);
        }
        return result;
    }
};

class ArgumentTest : public Command, public Test
{
public:
    ArgumentTest() : Command()
    {
        this->name(id())
        ->argument("<from>", "from argument")
        ->argument("[to...]", "to arguments");
    }
    virtual std::string id() override { return "ArgumentTest"; }
    virtual TestResult test() override
    {
        TestResult result = {false, ""};
        class LoggerImpl : public Logger
        {
        public:
            virtual Logger *print(const String &msg) override
            {
                std::cout << msg << std::endl;
                // std::smatch res;
                // std::regex reg(R"(测试命令行解析。)");
                // if (!std::regex_search(msg, res, reg))
                // {
                //     this->result = {false, "description is not: 测试命令行解析。"};
                // }
                return this;
            };
            TestResult result = {true, ""};
        };
        // this->action([](Vector<Variant> args, Map<String, Variant> opts) {});
        this->pLogger = new LoggerImpl();
        char* argv[] = {"testCommand"};
        this->parse(1, argv);
        // char* argv1[] = {"testCommand", "-h"};
        // this->parse(2, argv1);
        // char* argv2[] = {"testCommand", "--help"};
        // this->parse(2, argv2);

        return result;
    }
};

int main(int argc, char **argv)
{
    Command("test").option("-i --display-success-info", "显示信息")
    ->action([](Vector<Variant> args,Map<String, Variant> opts){
        Test *tests[] = {
            new VersionTest(),
            new DescriptionTest(),
            new OptionTest(),
            // new ArgumentTest()
        };

        for (int i = 0; i < std::size(tests); i++)
        {
            TestResult res = tests[i]->test();
            if (!res.result)
            {
                std::cout << "test :" << tests[i]->id() << " 失败: " << res.errMsg << std::endl;
            }
            else
            {
                if(opts.find("display-success-info") != opts.end())
                {
                    std::cout << "test :" << tests[i]->id() << " 成功" << std::endl;
                }
            }
        } 
    })
    ->parse(argc, argv);
    return 0;
}
