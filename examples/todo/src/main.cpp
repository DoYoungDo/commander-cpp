#include "../../../src/commander_cpp.hpp"
#include <iostream>

using namespace COMMANDER_CPP;

int main(int argc, char **argv)
{
    // TestLogger logger;
    // logger.stdOut = true;
    Command("todo")
        .version("0.0.1")
        ->description("待办。")
        ->addCommand((new Command("add"))
                         ->description("添加一个新的待办事项。")
                         ->argument("<todo...>", "待办事项内容。")
                         ->action([](Command *cmd, Vector<Variant> args, Map<String, Variant> opts) {
                             if (args.size() > 0)
                             {
                                 std::cout << "添加待办事项: ";
                                 for (const auto &arg : std::get<std::vector<VariantBase>>(args[0]))
                                 {
                                     std::cout << std::get<String>(arg) << " ";
                                 }
                                 std::cout << std::endl;
                             }
                             else
                             {
                                 std::cout << "没有提供待办事项内容。" << std::endl;
                             }
                         }))
        // ->command("add <todo...>", "添加一个新的待办事项。")
        // ->action([](Command *cmd, Vector<Variant> args, Map<String, Variant> opts) {
        //     // std::cout << "这是一个待办事项应用程序的示例。" << std::endl;
        //     std::cout << cmd->helpText() << std::endl;
        // })
        ->parse(argc, argv);
    return 0;
}
