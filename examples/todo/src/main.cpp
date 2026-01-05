#include <iostream>
#include "../../../src/commander_cpp.hpp"

using namespace COMMANDER_CPP;


class TestLogger : public Logger
{
  public:
  TestLogger(){}
    // virtual Logger *debug(const String &msg)
    // {
    //     std::cout << "DEBUG: " << msg << std::endl;
    //     return this;
    // }
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
        std::cout << msg << std::endl;
        return this;
    }
};

class Configer
{
  public:
    Configer()
    {
    }

    bool checkLocal()
    {
        return false;
    }
};

void doActionAdd(Command *cmd, Vector<Variant> args, Map<String, Variant> opts)
{
    Configer cfg;
    if (!cfg.checkLocal())
    {
        cmd->logger()->error("当前目录未初始化为todo仓库，请先运行todo conf命令进行初始化。");
        return;
    }
}
void doActionRm(Command *cmd, Vector<Variant> args, Map<String, Variant> opts)
{
}
void doActionMod(Command *cmd, Vector<Variant> args, Map<String, Variant> opts)
{
}
void doActionList(Command *cmd, Vector<Variant> args, Map<String, Variant> opts)
{
}
void doActionMv(Command *cmd, Vector<Variant> args, Map<String, Variant> opts)
{
}
void doActionConf(Command *cmd, Vector<Variant> args, Map<String, Variant> opts)
{
}
void doActionConfInit(Command *cmd, Vector<Variant> args, Map<String, Variant> opts)
{
    Configer cfg;
    if (cfg.checkLocal())
    {
        cmd->logger()->print("已经初始化过，是否重新初始化？(y/n)");
        String text;
        std::cin >> text;
        if (text != "y")
        {
            return;
        }
    }

    String text;
    std::cin >> text;
}

int main(int argc, char **argv)
{
    /*

    Usage: todo

    待办。

    Options:
    -V, --version                   out put version number.
    -h, --help  

    Commands:
    add [options] <todo...>         添加新的待办事项。
    rm <index...>                   删除待办事项。
    mod [options] <index> [todo]    修改一个待办事项。
    list [options] [range]          显示待办事项列表。
    mv <index> <distIndex>          移动待办事项。
    conf  配置。

    */
    auto logger = new TestLogger();
    Command("todo", logger)
        .version("0.0.1")
        ->description("待办。")
        ->addCommand((new Command("add", logger))
                         ->description("添加新的待办事项。")
                         ->argument("<todo...>", "待办事项内容。")
                         ->option("-d --done", "将待办事项标记为已完成。")
                         ->action(doActionAdd))
        ->addCommand((new Command("rm", logger))
                         ->description("删除待办事项。")
                         ->argument("<index...>", "待办事项索引。")
                         ->action(doActionRm))
        ->addCommand((new Command("mod", logger))
                         ->description("修改一个待办事项。")
                         ->option("-a --append", "追加内容到待办事项。")
                         ->option("-d --done", "将待办事项标记为已完成。")
                         ->argument("<index>", "待办事项索引。")
                         ->argument("[todo]", "待办事项内容。")
                         ->action(doActionMod))
        ->addCommand((new Command("list", logger))
                         ->description("显示待办事项列表。")
                         ->option("-d --done <done>", "只显示完成的或未完成的待办事项，参数为true或false。", true)
                         ->option("-c --count", "只显示待办事项数量。")
                         ->argument(
                             "[range]",
                             "显示范围\n起始：[start | [start-，结束缺省:max\n结束：end]，起始缺省：0\n起始-结束：[start-end] | "
                             "start-end\n例：\n查看起始位置为12的：[12 或 [12-\n查看起始位置为12结束位置为14的：12-14 或 "
                             "[12-14]\n查看起始位置为0结束位置为14的：14],14] = [0-14] = 0-14")
                         ->action(doActionList))
        ->addCommand((new Command("mv", logger))
                         ->description("移动待办事项。")
                         ->argument("<index>", "待办事项索引。")
                         ->argument("<distIndex>", "目标索引。")
                         ->action(doActionMv))
        ->addCommand((new Command("conf", logger))
                         ->description("配置。")
                         ->addCommand((new Command("init", logger))
                                         ->description("初始化todo仓库。")
                                         ->action(doActionConfInit))
                         ->action(doActionConf))
        ->action([](Command *cmd, Vector<Variant> args, Map<String, Variant> opts) {
            char* argv[] = {(char*)"todo", (char*)"--help"};
            cmd->parse(2, argv);
        })
        ->parse(argc, argv);
    return 0;
}
