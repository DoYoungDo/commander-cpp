#include <iostream>
#include <filesystem>
#include <fstream>

#include "../../../src/commander_cpp.hpp"
#include "../../third/nlohmann/json.hpp"

using namespace COMMANDER_CPP;
using json = nlohmann::json;
namespace fs = std::filesystem;
using String = std::string;

struct Todo
{
    String id;
    String todo;
    String begin;
    String end;
    String desc;
    bool done;
    int priority;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Todo, id, todo, begin, end, done, priority);
};

struct Table
{
    String name;
    String date;
    Vector<Todo> list;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Table, name, date, list);
};

struct TableConnectInfo
{
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TableConnectInfo);
};
class TableConnect
{
  public:
    virtual ~TableConnect();
    virtual bool checkConnect() = 0;
    virtual bool initConnect(TableConnectInfo *info) = 0;
    virtual bool connectToTable(const String &tableName) = 0;
    virtual bool getTable(Table &table) = 0;
    virtual bool addTodo(const Todo &doto) = 0;
    virtual bool addTodoList(const Vector<Todo> &doto) = 0;
    virtual Vector<Todo> findTodoList(const String &name) = 0;
    virtual bool rmTodoById(const String &id) = 0;
    virtual bool rmTodoListById(const Vector<String> &idList) = 0;

  public:
    static void registerConnect(const String &name, std::function<TableConnect *()> creator)
    {
        connectors[name] = creator;
    }
    static bool hasConnect(const String &name)
    {
        return connectors.find(name) != connectors.end();
    }
    static std::function<TableConnect *()> getConnectCreator(const String &name)
    {
        if(!hasConnect(name))
        {
            return nullptr;
        }
        return connectors.find(name)->second;
    }
    static Vector<String> getConnectNames()
    {
        Vector<String> names;
        for (auto it : connectors)
        {
            names.push_back(it.first);
        }
        return names;
    }

  private:
    static Map<String, std::function<TableConnect *()>> connectors;
};
Map<String, std::function<TableConnect *()>> TableConnect::connectors;

class Configer
{
  public:
    struct Setting
    {
        String connect;
        String table = "default";
        TableConnectInfo connectInfo;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Configer::Setting, connect, table, connectInfo);
    } setting;

    Configer()
    {
    }
    ~Configer()
    {
        for (auto it : connectors)
        {
            delete it.second;
        }
    }

    bool check()
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

        try
        {
            std::ifstream f(configPath);
            json data = json::parse(f);
            data.get_to(setting);

            if (setting.connect.empty())
            {
                return false;
            }
        }
        catch (const std::exception &e)
        {
            return false;
        }

        return true;
    }
    bool initLocal(const String &connectName, const String &table, String &errMsg)
    {
        if (!TableConnect::hasConnect(connectName))
        {
            errMsg = "不支持的连接方式：" + connectName;
            for(auto name : TableConnect::getConnectNames())
            {
                errMsg += "，支持的连接方式：" + name;
            }
            return false;
        }

        TableConnect *connect = getTableConnect(connectName);
        TableConnectInfo info;
        bool succ = connect->initConnect(&info);
        if (!succ)
        {
            errMsg = "初始化连接 " + connectName + " 失败，请检查连接配置。";
            return false;
        }

        String appDataDir = getAppDataDir();
        if (!fs::exists(appDataDir) && !fs::create_directories(appDataDir))
        {
            errMsg = "创建缓存目录失败，请检查权限。";
            return false;
        }

        setting.connect = connectName;
        setting.table = table;
        setting.connectInfo = info;

        json data = setting;
        String dataText = data.dump(4);
        std::ofstream out(getConfigPath());
        out << dataText;

        return succ;
        // errMsg = connectName + " " + table;

    

        // if (!fs::exists(repository) && !fs::create_directories(repository))
        // {
        //     errMsg = "创建存储目录失败，请检查权限。";
        //     return false;
        // }
        // setting.repository = repository;

        // String tableDir = getTableDir();
        // if (!fs::exists(tableDir) && !fs::create_directories(tableDir))
        // {
        //     errMsg = "创建表存储目录失败，请检查权限。";
        //     return false;
        // }

        // setting.table = table;



        return false;
    }
    TableConnect *getTableConnect()
    {
        return getTableConnect(setting.connect);
    }
    TableConnect *getTableConnect(const String &connectName)
    {
        auto it = connectors.find(connectName);
        if(it != connectors.end())
        {
            return it->second;
        }

        TableConnect *connect = TableConnect::getConnectCreator(connectName)();
        if(connect)
        {
            connectors[connectName] = connect;
            return connect;
        }

        return nullptr;
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
    // inline String getTableDir()
    // {
    //     return setting.repository + "/tables";
    // }
  private:
    Map<String, TableConnect *> connectors;
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
       if(cfg.check())
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
    TableConnect* connect = nullptr;
    if (!cfg.check() || !(connect = cfg.getTableConnect()))
    {
        cmd->logger()->error("未初始化仓库,请先运行todo conf init... 命令进行初始化，更多信息请运行 todo conf --help 查看。");
        return;
    }

    // if(args.empty())
    // {
    //     cmd->logger()->error("没有可用的待办事项。");
    //     return;
    // }

    // Table table;
    // try
    // {
    //     String tableFile = cfg.getTableDir() + "/" + cfg.setting.table;
    //     std::ifstream f(tableFile);
    //     if(fs::exists(tableFile))
    //     {
    //         json data = json::parse(f);
    //         data.get_to(table);
    //     }
    // }
    // catch (const std::exception &e)
    // {
    //     cmd->logger()->warn("读取失败。");
    //     // return;
    // }

    // cmd->logger()->debug("开始执行添加。");
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
    Logger *logger = cmd->logger();
    Configer cfg;
    if (cfg.check() && !opts.hasOption("cover"))
    {
        logger->print("已经初始化过，是否重新初始化？(y/n)");
        String text;
        while (true)
        {
            std::cin >> text;
            if (text == "y")
            {
                break;
            }
            else if (text == "n")
            {
                logger->print("取消初始化。");
                return;
            }
            logger->print("无效的输入:" + text);
        }
    }

    String connectName;
    if (opts.hasOption("connectName"))
    {
        connectName = opts.getValue<String>("connectName", "");
    }

    if (connectName.empty())
    {
        logger->print("请输入connectName");
        while (true)
        {
            std::cin >> connectName;
            if (connectName.empty())
            {
                logger->print("connectName 不能为空，请输入connectName");
                continue;
            }
            break;
        }
    }

    String table;
    if (opts.hasOption("table"))
    {
        table = opts.getValue<String>("table", "");
    }

    if (table.empty())
    {
        logger->print("请输入table");
        while (true)
        {
            std::cin >> table;
            if (table.empty())
            {
                logger->print("table 不能为空，默认使用：default");
            }
            break;
        }
    }

    String err;
    if(cfg.initLocal(connectName, table, err))
    {
        logger->print("初始化成功。");
    }
    else
    {
        logger->print("初始化失败：" + err);
    }
    // String repository;
    // if (opts.hasOption("repository"))
    // {
    //     repository = opts.getValue<String>("repository", "");
    //     if(repository.empty())
    //     {
    //         logger->error("存储仓库路径不能为空。");
    //         return;
    //     }
    // }
    // else
    // {
    //     logger->print("请输入存储路径");
    //     std::cin >> repository;
    // }

    // String table;
    // if (opts.hasOption("table"))
    // {
    //     table = opts.getValue<String>("table", "");
    //     if (table.empty())
    //     {
    //         logger->warn("无效的表名，使用默认表名：default。");
    //         table = "default";
    //     }
    // }
    // else
    // {
    //     logger->print("请输入表名");
    //     std::cin >> table;
    // }
    // String errMsg;
    // if (!cfg.initLocal(table, repository, errMsg))
    // {
    //     logger->error("初始化失败：" + errMsg);
    // }
    // else
    // {
    //     logger->print("初始化成功。");
    // }
}
// void doActionConfInitWithLocal(Command *cmd, Vector<Variant> args, Options opts)
// {
//     Logger *logger = cmd->logger();
//     Configer cfg;
//     if (cfg.check() && !opts.hasOption("cover"))
//     {
//         logger->print("已经初始化过，是否重新初始化？(y/n)");
//         String text;
//         while (true)
//         {
//             std::cin >> text;
//             if (text == "y")
//             {
//                 break;
//             }
//             else if (text == "n")
//             {
//                 logger->print("取消初始化。");
//                 return;
//             }
//             logger->print("无效的输入:" + text);
//         }
//     }
// }

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
                         ->option("-D --details <descriptions>", "为待办项添加描述。")
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
                         ->argument("[range]", "显示范围\n"
                                                "起始：[start | [start-，结束缺省:max\n"
                                                "结束：end]，起始缺省：0\n"
                                                "起始-结束：[start-end] | start-end\n"
                                                "例：\n"
                                                "查看起始位置为12的：[12 或 [12-\n"
                                                "查看起始位置为12结束位置为14的：12-14 或 [12-14]\n"
                                                "查看起始位置为0结束位置为14的：14],14] = [0-14] = 0-14")
                         ->action(doActionList))
        ->addCommand((new Command("mv", logger))
                         ->description("移动待办事项。")
                         ->argument("<index>", "待办事项索引。")
                         ->argument("<distIndex>", "目标索引。")
                         ->action(doActionMv))
        ->addCommand((new Command("conf", logger))
                         ->description("配置。")
                         ->addCommand((new Command("init", logger))
                                          ->description("初始化仓库。")
                                          ->option("-c --cover", "如果已经初始化，不再询问，直接覆盖。")
                                          ->option("-t --table <tableName>", "指定表名，默认为default。")
                                          ->option("--connect <connectName>", "指定连接方式。")
                                          ->action(doActionConfInit))
                        //  ->addCommand((new Command("initWithLocal", logger))
                        //                   ->description("初始化本地仓库。")
                        //                   ->option("-c --cover", "如果已经初始化，不再询问，直接覆盖。")
                        //                   ->option("-t --table <tableName>", "指定表名，默认为default。")
                        //                   ->option("-r --repository <repositoryPath>", "指定仓库路径。")
                        //                   ->action(doActionConfInitWithLocal))
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
