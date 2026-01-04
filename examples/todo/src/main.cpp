#include "../../../src/commander_cpp.hpp"
#include <iostream>

using namespace COMMANDER_CPP;


class TestLogger : public Logger
{
  public:
  TestLogger(){}
    virtual Logger *debug(const String &msg)
    {
        std::cout << "DEBUG: " << msg << std::endl;
        return this;
    }
    virtual Logger *warn(const String &msg)
    {
        std::cout << "WARN: " << msg << std::endl;
        return this;
    }
    virtual Logger *error(const String &msg)
    {
        std::cout << "ERROR: " << msg << std::endl;
        return this;
    }
    virtual Logger *print(const String &msg)
    {
        std::cout << "PRINT: " << msg << std::endl;
        return this;
    }
};
int main(int argc, char **argv)
{
    // logger.stdOut = true;
    Command("todo", new TestLogger())
        .version("0.0.1")
        ->description("待办。")
        ->addCommand((new Command("add", nullptr))
                         ->description("添加一个新的待办事项。")
                         ->argument("<todo...>", "待办事项内容。")
                         ->action([](Command *cmd, Vector<Variant> args, Map<String, Variant> opts) {
                             std::cout << "这是一个待办事项应用程序的示例。" << std::endl;
                         }))
        // ->command("add <todo...>", "添加一个新的待办事项。")
        // ->action([](Command *cmd, Vector<Variant> args, Map<String, Variant> opts) {
        //     std::cout << cmd->helpText() << std::endl;
        // })
        ->parse(argc, argv);
    return 0;
}
