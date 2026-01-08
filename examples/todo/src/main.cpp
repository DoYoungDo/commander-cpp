#include <iostream>
#include <filesystem>
#include <fstream>

#include "../../../src/commander_cpp.hpp"
#include "../../third/nlohmann/json.hpp"

using namespace COMMANDER_CPP;
using json = nlohmann::json;
namespace fs = std::filesystem;
using String = std::string;

class Configer
{
  public:
    struct Setting
    {
        String table = "default";
        String repository;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Configer::Setting, table, repository)
    } setting;
    
    Configer()
    {
    }

    bool checkLocal()
    {
        String appDataDir = getAppDataDir();
        if (!fs::exists(appDataDir))
        {
            return false;
        }
        String configPath = getConfigPath();
        if (!fs::exists(configPath))
        {
            return false;
        }
        std::ifstream f(configPath);
        json data = json::parse(f);

        try
        {
            data.get_to(setting);

            if(setting.repository.empty())
            {
                return false;
            }
        }
        catch(const std::exception& e)
        {
            return false;
        }

        return true;
    }
    bool initLocal(const String& table, const String& repository, String& errMsg)
    {
        String appDataDir = getAppDataDir();
        if (!fs::exists(appDataDir) && !fs::create_directories(appDataDir))
        {
            errMsg = "创建缓存目录失败，请检查权限。";
            return false;
        }

        if (!fs::exists(repository) && !fs::create_directories(repository))
        {
            errMsg = "创建存储目录失败，请检查权限。";
            return false;
        }

        setting.repository = repository;
        setting.table = table;

        json data = setting;
        String dataText = data.dump(4);
        std::ofstream out(getConfigPath());
        out << dataText;

        return true;
    }

  public:
    inline String getAppDataLocalDir()
    {
        return "/Users/doyoung/Library/Application Support";
    }
    inline String getAppDataDir()
    {
        return getAppDataLocalDir() + "/todo";
    }
    inline String getConfigPath()
    {
        return getAppDataDir() + "/setting.json";
    }
    inline String getLogPath()
    {
        return getAppDataDir() + "/.log";
    }
};

class TodoLogger : public Logger
{
  public:
    TodoLogger()
    {
    }
    ~TodoLogger()
    {
       Configer cfg;
       if(cfg.checkLocal())
       {
           String logFilePath = cfg.getLogPath();
           std::ofstream out(logFilePath, std::ios_base::app);
           for(auto log : logs)
           {
               out << log << std::endl;
           }
           out << std::endl;
       }
    }
    virtual Logger *debug(const String &msg)
    {
        // std::cout << "DEBUG: " << msg << std::endl;
        recordLog("DEBUG: " + msg);
        return this;
    }
    virtual Logger *warn(const String &msg)
    {
        std::cout << "WARN: " << msg << std::endl;
        recordLog("WARN: " + msg);
        return this;
    }
    virtual Logger *error(const String &msg)
    {
        std::cout << "ERROR: " << msg << std::endl;
        recordLog("ERROR: " + msg);
        return this;
    }
    virtual Logger *print(const String &msg)
    {
        std::cout << msg << std::endl;
        return this;
    }

    void printHelp(Command *cmd)
    {
        char *argv[] = {(char *)"--help"};
        cmd->parse(1, argv, 0);
    }

  private:
    void recordLog(const String &msg)
    {
        logs.push_back(msg);
    }
    Vector<String> logs;
};

void doActionAdd(Command *cmd, Vector<Variant> args, Map<String, Variant> opts)
{
    Configer cfg;
    if (!cfg.checkLocal())
    {
        cmd->logger()->error("当前目录未初始化为todo仓库，请先运行todo conf init命令进行初始化。");
        return;
    }

    if(args.empty())
    {
        cmd->logger()->error("没有可用的待办事项。");
        return;
    }

    cmd->logger()->debug("开始执行添加。");
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
void doActionConfInit(Command *cmd, Vector<Variant> args, Options opts)
{
    Logger* logger = cmd->logger();
    Configer cfg;
    if (cfg.checkLocal() && !opts.hasOption("cover"))
    {
        logger->print("已经初始化过，是否重新初始化？(y/n)");
        String text;
        std::cin >> text;
        if (text != "y")
        {
            return;
        }
    }

    String repository;
    if (opts.hasOption("repository"))
    {
        repository = opts.getValue<String>("repository", "");
        if(repository.empty())
        {
            logger->error("存储仓库路径不能为空。");
            return;
        }
    }
    else
    {
        logger->print("请输入存储路径");
        std::cin >> repository;
    }

    String table;
    if (opts.hasOption("table"))
    {
        table = opts.getValue<String>("table", "");
        if (table.empty())
        {
            logger->warn("无效的表名，使用默认表名：default。");
            table = "default";
        }
    }
    else
    {
        logger->print("请输入表名");
        std::cin >> table;
    }
    String errMsg;
    if (!cfg.initLocal(table, repository, errMsg))
    {
        logger->error("初始化失败：" + errMsg);
    }
    else
    {
        logger->print("初始化成功。");
    }
}

int main(int argc, char **argv)
{
    // for(int i = 0 ;i < argc;++i)
    // {
    //     std::cout << argv[i] << std::endl;
    // }
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
    conf                            配置。

    */
    auto logger = new TodoLogger();
    Command("todo", logger)
        .version("0.0.1", "-V --version", "显示版本号。")
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
                                         ->option("-c --cover", "如果已经初始化，不再询问，直接覆盖。")
                                         ->option("-r --repository <dir>", "直接指定存储仓库位置，不再询问。")
                                         ->option("-t --table <name>", "直接指定表名，不再询问。")
                                         ->action(doActionConfInit))
                         ->action([](Command *cmd, Vector<Variant> args, Map<String, Variant> opts) {
                             static_cast<TodoLogger *>(cmd->logger())->printHelp(cmd);
                         }))
        ->action([](Command *cmd, Vector<Variant> args, Map<String, Variant> opts) {
            static_cast<TodoLogger *>(cmd->logger())->printHelp(cmd);
        })
        ->parse(argc, argv);

    delete logger;
    logger = nullptr;
    
    return 0;
}
