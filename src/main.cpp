#include <iostream>

#include "commander_cpp.hpp"

using namespace COMMANDER_CPP;

void test1(int argc, char **argv)
{
    Command* cmd = (new Command("testCommand"))
    ->version("0.0.1")
    ->description("测试命令行解析。")
    ->argument("<files...>", "输入文件")
    ->option("-o --outDir [dir]", "输出目录", "./out")
    ->option("-n --names [names...]", "指定名称，和参数顺序一致")
    ->option("-s --sourceMap", "生成sourceMap", 123)
    ->option("--reuse", "重用生成文件", false)
    ->option("--generatewithVersionName <name>", "生成名称包含版本名称") // 补丁参数
    ->action([](std::vector<std::string> args, std::map<std::string, Variant> opts) {
        for(auto a : args){
            std::cout << a << std::endl;
        }
        for(auto o : opts){
            std::cout << o.first << std::endl;
        }
        std::cout << "test1 action" << std::endl;
    })
    ;
    cmd->command("sub")
        ->description("子命令")
        ->option("-s --sourceMap", "生成sourceMap")
        ->argument("<subFiles...>", "子命令输入文件")
    ;
    cmd->command("sub1 [sub1Files...]", "子命令")
        ->description("子命令1")
    ;

    cmd->parse(argc, argv)
    ;
}

int main(int argc, char **argv) {
    test1(argc, argv);
    return 0;
}
