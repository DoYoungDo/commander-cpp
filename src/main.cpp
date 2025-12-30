#include <functional>
#include <iostream>

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

inline TestResult mergeAll(const std::vector<TestResult> &vec)
{
    TestResult result{true, ""};
    for (const auto &r : vec)
        result = result.merge(r);
    return result;
}

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
        if (checkDebug != nullptr)
            checkDebug(msg);
        return this;
    }
    virtual Logger *warn(const String &msg)
    {
        if (checkWarn != nullptr)
            checkWarn(msg);
        return this;
    }
    virtual Logger *error(const String &msg)
    {
        if (checkError != nullptr)
            checkError(msg);
        return this;
    }
    virtual Logger *print(const String &msg)
    {
        if (checkPrint != nullptr)
            checkPrint(msg);
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
            if (msg != "1.2.3")
                this->result = {false, "version is not 1.2.3"};
            return this;
        }
        TestResult result = {true, ""};
    };

    VersionTest() : Command("", new LoggerImpl())
    {
        this->name(id())->version("1.2.3");
    }
    virtual std::string id() override
    {
        return "VersionTest";
    }
    virtual TestResult test() override
    {
        char *argv[] = {"testCommand", "--version"};
        this->parse(2, argv);
        char *argv1[] = {"testCommand", "--V"};
        this->parse(2, argv1);

        return static_cast<LoggerImpl *>(this->pLogger)->result;
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
            if (!std::regex_search(msg, res, reg))
                this->result = {false, "description is not: 测试命令行解析。"};
            return this;
        }
        TestResult result = {true, ""};
    };

    DescriptionTest() : Command("", new LoggerImpl())
    {
        this->name(id())->description("测试命令行解析。");
    }
    virtual std::string id() override
    {
        return "DescriptionTest";
    }
    virtual TestResult test() override
    {
        char *argv[] = {"testCommand"};
        this->parse(1, argv);
        char *argv1[] = {"testCommand", "-h"};
        this->parse(2, argv1);
        char *argv2[] = {"testCommand", "--help"};
        this->parse(2, argv2);

        return static_cast<LoggerImpl *>(this->pLogger)->result;
    }
};

class OptionTest : public Command, public Test
{
  public:
    OptionTest() : Command("", new TestLogger())
    {
        this->name(id())
            ->description("测试选项解析。")
            ->option("-s --mustSimgleOption <values>", "必须单个值")
            ->option("-m --mustMultiOption <values...>", "至少一个值");
    }
    virtual std::string id() override
    {
        return "OptionTest";
    }
    virtual TestResult test() override
    {
        std::vector<TestResult> results;
        TestLogger *logger = static_cast<TestLogger *>(this->pLogger);

        do
        {
            bool hasError = false;
            logger->checkError = [&](const std::string &msg) {
                // std::cout << msg << std::endl;
                if (msg == "option: mustSimgleOption need a value, but got zero.")
                {
                    logger->checkError = nullptr;
                    hasError = true;
                }
            };

            char *argv[] = {"testCommand", "-s"};
            this->parse(2, argv);

            results.push_back(hasError ? TestResult{true, ""}
                                       : TestResult{false, "未正确报错：option: mustSimgleOption "
                                                           "need a value, but got zero."});
        } while (false);

        do
        {
            bool hasError = false;
            logger->checkWarn = [&](const std::string &msg) {
                // std::cout << msg << std::endl;
                if (msg == "unknown identifier: otherValue")
                {
                    logger->checkWarn = nullptr;
                    hasError = true;
                }
            };

            char *argv[] = {"testCommand", "-s", "value", "otherValue"};
            this->parse(4, argv);

            results.push_back(hasError ? TestResult{true, ""}
                                       : TestResult{false, "未正确报错：unknown identifier: otherValue"});
        } while (false);

        do
        {
            logger->checkWarn = [&](const std::string &msg) {
                results.push_back(TestResult{false, "意外的警告：" + msg});
            };
            logger->checkError = [&](const std::string &msg) {
                results.push_back(TestResult{false, "意外的错误：" + msg});
            };
            this->action([&](Vector<Variant> args, Map<String, Variant> opts) {
                auto it = opts.find("mustSimgleOption");
                if (it == opts.end())
                {
                    results.push_back(TestResult{false, "未解析到option: mustSimgleOption"});
                    return;
                }

                if (!std::holds_alternative<String>(it->second))
                {
                    results.push_back(TestResult{false, "option: mustSimgleOption 未解析到值"});
                    return;
                }

                auto v = std::get_if<String>(&it->second);
                if (!v)
                {
                    results.push_back(TestResult{false, "option: mustSimgleOption 的值不是一个字符串"});
                    return;
                }

                if (*v != "value")
                {
                    results.push_back(TestResult{false, "option: mustSimgleOption 的值不是预期值:value"});
                }
            });

            char *argv[] = {"testCommand", "-s", "value"};
            this->parse(3, argv);
        } while (false);

        return mergeAll(results);
    }
};

class ArgumentTest : public Command, public Test
{
  public:
    ArgumentTest() : Command("", new TestLogger())
    {
        this->name(id())->argument("<from>", "from argument")->argument("[to...]", "to arguments");
    }
    virtual std::string id() override
    {
        return "ArgumentTest";
    }
    virtual TestResult test() override
    {
        std::vector<TestResult> results;
        TestLogger *logger = static_cast<TestLogger *>(this->pLogger);

        do
        {
            this->action([&](Vector<Variant> args, Map<String, Variant> opts) {
                if (args.size() < 1)
                {
                    results.push_back(TestResult{false, "缺少必需的from参数"});
                    return;
                }

                auto from = std::get_if<String>(&args[0]);
                if (!from || *from != "source")
                {
                    results.push_back(TestResult{false, "from参数值不是预期值:source"});
                    return;
                }

                if (args.size() > 1)
                {
                    auto to = std::get_if<String>(&args[1]);
                    if (!to || *to != "target1")
                    {
                        results.push_back(TestResult{false, "to参数值不是预期值:target1"});
                        return;
                    }
                }

                results.push_back(TestResult{true, ""});
            });

            char *argv[] = {"testCommand", "source", "target1"};
            this->parse(3, argv);
        } while (false);

        do
        {
            this->action([&](Vector<Variant> args, Map<String, Variant> opts) {
                if (args.size() < 1)
                {
                    results.push_back(TestResult{false, "缺少必需的from参数"});
                    return;
                }

                auto from = std::get_if<String>(&args[0]);
                if (!from || *from != "source")
                {
                    results.push_back(TestResult{false, "from参数值不是预期值:source"});
                    return;
                }

                if (args.size() > 2)
                {
                    auto to2 = std::get_if<String>(&args[2]);
                    if (!to2 || *to2 != "target2")
                    {
                        results.push_back(TestResult{false, "to参数值不是预期值:target2"});
                        return;
                    }
                }

                results.push_back(TestResult{true, ""});
            });

            char *argv[] = {"testCommand", "source", "target1", "target2"};
            this->parse(4, argv);
        } while (false);

        do
        {
            bool hasError = false;
            logger->checkError = [&](const std::string &msg) {
                if (msg == "argument: from is required, but got empty.")
                    hasError = true;
            };
            char *argv[] = {"testCommand"};
            this->parse(1, argv);

            if (!hasError)
            {
                results.push_back(TestResult{false, "未正确报错：argument: from is required, but got empty."});
            }
        } while (false);

        return mergeAll(results);
    }
};

class SubCommandTest : public Command, public Test
{
  public:
    SubCommandTest() : Command("", new TestLogger())
    {
        this->name(id())->description("测试子命令功能");
    }
    virtual std::string id() override
    {
        return "SubCommandTest";
    }
    virtual TestResult test() override
    {
        std::vector<TestResult> results;
        TestLogger *logger = static_cast<TestLogger *>(this->pLogger);

        // 测试子命令解析 - add 命令
        do
        {
            bool addCommandExecuted = false;
            this->command("add <file>", "添加文件")->action([&](Vector<Variant> args, Map<String, Variant> opts) {
                if (args.size() > 0)
                {
                    auto file = std::get_if<String>(&args[0]);
                    if (file && *file == "test.txt")
                        addCommandExecuted = true;
                }
            });

            char *argv[] = {"testCommand", "add", "test.txt"};
            this->parse(3, argv);

            results.push_back(addCommandExecuted ? TestResult{true, ""} : TestResult{false, "add子命令未正确执行"});
        } while (false);

        // 测试子命令解析 - remove 命令
        do
        {
            bool removeCommandExecuted = false;
            this->command("remove <file>", "删除文件")->action([&](Vector<Variant> args, Map<String, Variant> opts) {
                if (args.size() > 0)
                {
                    auto file = std::get_if<String>(&args[0]);
                    if (file && *file == "test.txt")
                        removeCommandExecuted = true;
                }
            });

            char *argv[] = {"testCommand", "remove", "test.txt"};
            this->parse(3, argv);

            results.push_back(removeCommandExecuted ? TestResult{true, ""}
                                                    : TestResult{false, "remove子命令未正确执行"});
        } while (false);

        // 测试未知子命令的错误处理
        do
        {
            bool hasWarn = false;
            logger->checkWarn = [&](const std::string &msg) {
                if (msg.find("unknown identifier") != std::string::npos)
                    hasWarn = true;
            };

            char *argv[] = {"testCommand", "unknown"};
            this->parse(2, argv);

            results.push_back(hasWarn ? TestResult{true, ""} : TestResult{false, "未正确报告未知子命令的警告"});
        } while (false);

        return mergeAll(results);
    }
};

class DefaultValueTest : public Command, public Test
{
  public:
    DefaultValueTest() : Command("", new TestLogger())
    {
        this->name(id())
            ->description("测试默认值功能")
            ->option("-n --number <num>", "数字选项", 42)
            ->option("-b --boolean", "布尔选项", true)
            ->option("-s --string <text>", "字符串选项", "default")
            ->argument("[optional]", "可选参数", "optional_default");
    }
    virtual std::string id() override
    {
        return "DefaultValueTest";
    }
    virtual TestResult test() override
    {
        std::vector<TestResult> results;
        TestLogger *logger = static_cast<TestLogger *>(this->pLogger);

        this->action([&](Vector<Variant> args, Map<String, Variant> opts) {
            // 测试默认值
            auto numberIt = opts.find("number");
            if (numberIt != opts.end())
            {
                auto num = std::get_if<int>(&numberIt->second);
                if (num && *num == 42)
                    results.push_back(TestResult{true, ""});
                else
                    results.push_back(TestResult{false, "数字默认值不正确"});
            }

            auto booleanIt = opts.find("boolean");
            if (booleanIt != opts.end())
            {
                auto b = std::get_if<bool>(&booleanIt->second);
                if (b && *b == true)
                    results.push_back(TestResult{true, ""});
                else
                    results.push_back(TestResult{false, "布尔默认值不正确"});
            }

            auto stringIt = opts.find("string");
            if (stringIt != opts.end())
            {
                auto s = std::get_if<String>(&stringIt->second);
                if (s && *s == "default")
                    results.push_back(TestResult{true, ""});
                else
                    results.push_back(TestResult{false, "字符串默认值不正确"});
            }

            if (args.size() > 0)
            {
                auto arg = std::get_if<String>(&args[0]);
                if (arg && *arg == "optional_default")
                    results.push_back(TestResult{true, ""});
                else
                    results.push_back(TestResult{false, "参数默认值不正确"});
            }
        });

        char *argv[] = {"testCommand"};
        this->parse(1, argv);

        return mergeAll(results);
    }
};

class MultiValueOptionTest : public Command, public Test
{
  public:
    MultiValueOptionTest() : Command("", new TestLogger())
    {
        this->name(id())->description("测试多值选项")->option("-f --files <files...>", "文件列表");
    }
    virtual std::string id() override
    {
        return "MultiValueOptionTest";
    }
    virtual TestResult test() override
    {
        std::vector<TestResult> results;
        TestLogger *logger = static_cast<TestLogger *>(this->pLogger);

        this->action([&](Vector<Variant> args, Map<String, Variant> opts) {
            auto filesIt = opts.find("files");
            if (filesIt == opts.end())
            {
                results.push_back(TestResult{false, "未找到files选项"});
                return;
            }

            auto filesVec = std::get_if<std::vector<VariantBase>>(&filesIt->second);
            if (!filesVec)
            {
                results.push_back(TestResult{false, "files选项不是多值类型"});
                return;
            }

            if (filesVec->size() != 3)
            {
                results.push_back(TestResult{false, "files选项值数量不正确"});
                return;
            }

            std::vector<std::string> expected = {"file1.txt", "file2.txt", "file3.txt"};
            for (size_t i = 0; i < filesVec->size(); i++)
            {
                auto file = std::get_if<String>(&(*filesVec)[i]);
                if (!file || *file != expected[i])
                {
                    results.push_back(TestResult{false, "文件值不匹配: " + expected[i]});
                    return;
                }
            }

            results.push_back(TestResult{true, ""});
        });

        char *argv[] = {"testCommand", "-f", "file1.txt", "file2.txt", "file3.txt"};
        this->parse(5, argv);

        return mergeAll(results);
    }
};

class ErrorHandlingTest : public Command, public Test
{
  public:
    ErrorHandlingTest() : Command("", new TestLogger())
    {
        this->name(id())->description("测试错误处理")->option("-r --required <value>", "必需选项");
    }
    virtual std::string id() override
    {
        return "ErrorHandlingTest";
    }
    virtual TestResult test() override
    {
        std::vector<TestResult> results;
        TestLogger *logger = static_cast<TestLogger *>(this->pLogger);

        // 测试缺少必需选项的错误
        do
        {
            bool hasError = false;
            logger->checkError = [&](const std::string &msg) {
                if (msg.find("need a value") != std::string::npos)
                    hasError = true;
            };

            char *argv[] = {"testCommand", "-r"};
            this->parse(2, argv);

            results.push_back(hasError ? TestResult{true, ""} : TestResult{false, "未正确报告缺少必需值的错误"});
        } while (false);

        // 测试未知选项的错误
        do
        {
            bool hasWarn = false;
            logger->checkWarn = [&](const std::string &msg) {
                if (msg == "unknown option: unknown-option")
                    hasWarn = true;
            };

            char *argv[] = {"testCommand", "--unknown-option"};
            this->parse(2, argv);

            results.push_back(hasWarn ? TestResult{true, ""} : TestResult{false, "未正确报告未知选项的警告"});
        } while (false);

        return mergeAll(results);
    }
};

class ComplexOptionTest : public Command, public Test
{
  public:
    ComplexOptionTest() : Command("", new TestLogger())
    {
        this->name(id())
            ->description("测试复杂选项组合")
            ->option("-d --debug", "调试模式", false)
            ->option("-v --verbose", "详细模式", false);
    }
    virtual std::string id() override
    {
        return "ComplexOptionTest";
    }
    virtual TestResult test() override
    {
        std::vector<TestResult> results;
        TestLogger *logger = static_cast<TestLogger *>(this->pLogger);

        this->action([&](Vector<Variant> args, Map<String, Variant> opts) {
            // 测试布尔选项
            auto debugIt = opts.find("debug");
            if (debugIt != opts.end())
            {
                auto debug = std::get_if<bool>(&debugIt->second);
                if (debug && *debug == true)
                    results.push_back(TestResult{true, ""});
            }

            auto verboseIt = opts.find("verbose");
            if (verboseIt != opts.end())
            {
                auto verbose = std::get_if<bool>(&verboseIt->second);
                if (verbose && *verbose == true)
                    results.push_back(TestResult{true, ""});
            }
        });

        char *argv[] = {"testCommand", "-abc", "--debug", "--verbose"};
        this->parse(4, argv);

        TestResult result({true, ""});
        for (auto res : results)
            result = result.merge(res);
        return result;
    }
};

int main(int argc, char **argv)
{
    Command("test")
        .option("-i --display-success-info", "显示信息")
        ->action([](Vector<Variant> args, Map<String, Variant> opts) {
            Test *tests[] = {new VersionTest(),          new DescriptionTest(),   new OptionTest(),
                             new ArgumentTest(),         new SubCommandTest(),    new DefaultValueTest(),
                             new MultiValueOptionTest(), new ErrorHandlingTest(), new ComplexOptionTest()};

            for (int i = 0; i < std::size(tests); i++)
            {
                TestResult res = tests[i]->test();
                if (!res.result)
                {
                    std::cout << "测试 :" << tests[i]->id() << " 失败: " << res.errMsg << std::endl;
                }
                else
                {
                    if (opts.find("display-success-info") != opts.end())
                        std::cout << "测试 :" << tests[i]->id() << " 成功" << std::endl;
                }
            }
        })
        ->parse(argc, argv);
    return 0;
}
