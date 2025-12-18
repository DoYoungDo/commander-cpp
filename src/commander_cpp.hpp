#include <functional>
#include <vector>
#include <map>
#include <variant>
#include <regex>
#include <sstream>

namespace COMMANDER_CPP
{
using String = std::string;
using Variant = std::variant<std::monostate, int, double, String, bool, std::vector<std::variant<int, double, String, bool>>>;
template<typename K, typename V>
using Map = std::map<K,V>;
template<typename T>
using Vector = std::vector<T>;
using Action = std::function<void(Vector<String> args,Map<String, Variant> opts)>;

class FinialRelease
{
public:
    FinialRelease(std::function<void()> release):r(release){}
    ~FinialRelease(){ if (r) r();}
private:
    std::function<void()> r;
};

class Logger
{
    public:
        // template <typename... Args>
        virtual Logger* log(const String& msg) = 0;
        inline Logger& operator<<(const String& msg)
        {
            return *log(msg);
        }
};

class Command
{
public:
    Command(const String& name, Logger* logger = nullptr)
        : commandName(name)
        , actionCallback(nullptr)
        , pLogger(logger)
        , versionOption(nullptr)
        , helpOption(nullptr)
        , parentCommand(nullptr)
    {
        if(!pLogger)
        {
            class LoggerImpl : public Logger
            {
                public:
                    Logger* log(const String& msg) override
                    {
                        std::cout << msg;
                        return this;
                    }
            };
            pLogger = new LoggerImpl();
        }

        version("0.0.0", "-V --version", "out put version number.");
        help("-h --help");
    }
    ~Command()
    {
        if(versionOption)
        {
            delete versionOption;
            versionOption = nullptr;
        }
        if(helpOption)
        {
            delete helpOption;
            helpOption = nullptr;
        }

        for(const auto opt : options)
        {
            delete opt;
        }
        options.clear();

        for(const auto cmd : subCommands)
        {
            delete cmd;
        }
        subCommands.clear();

        for(const auto arg: arguments)
        {
            delete arg;
        }
        arguments.clear();
    }

public:
    virtual Command* version(const String& v, const String& flag = String(), const String& desc = String())
    {
        // 函数结束时调用
        FinialRelease f([this,v,desc](){
            if(versionOption){
                versionOption->defaultValue = v;
                versionOption->desc = desc;
            }
        });

        if(!flag.empty())
        {
            Option* opt = Option::create(flag,pLogger);
            if(!opt)
            {
                return this;
            }

            if(versionOption)
            {
                delete versionOption;
                versionOption = nullptr;
            }

            versionOption = opt;
        }

        return this;
    };
    virtual String version()
    {
        String* v = std::get_if<String>(&versionOption->defaultValue);
        return v ? *v : "0.0.0";
    }

    virtual Command* description(const String& desc)
    {
        commandDescription = desc;
        return this;
    };
    virtual String description(){return commandDescription;}

    virtual Command* help(const String& flag = String(), const String& desc = String())
    {
        FinialRelease r([this,desc](){
            if(helpOption)
            {
                helpOption->desc = desc;
            }
        });

        if(!flag.empty())
        {
            Option* opt = Option::create(flag,pLogger);
            if(!opt)
            {
                return this;
            }

            if(helpOption)
            {
                delete helpOption;
                helpOption = nullptr;
            }

            helpOption = opt;
        }
        return this;
    };
    virtual String helpText()
    {
        auto getHelpText = [this](){
            auto getUsageText = [this](Command* cmd){
                std::stringstream out; 
                out << cmd->commandName;
                if(cmd->options.size()) out << " [options]";
                String argumentText;
                for(const auto arg : cmd->arguments)
                {
                    argumentText += " " + (arg->isMultiValue ? "<" + arg->name + (arg->valueIsRequired ? "..." : "") + ">" : 
                                        "[" + arg->name + (arg->valueIsRequired ? "..." : "") + "]");
                }
                if(cmd->arguments.size()) out << argumentText;

                return out.str();
            };

            String usageText = getUsageText(this);
            Command* p = parentCommand;
            while(p)
            {
                usageText = p->commandName + " " + usageText;
                p = p->parentCommand;
            }

            std::stringstream out;
            out << "Usage: " << usageText << std::endl
                << std::endl << description() << std::endl;
            
            if(!arguments.empty())
            {
                out << std::endl << "Arguments:" << std::endl;
                for(const auto arg : arguments)
                {
                    out << "  " << arg->name << (arg->valueIsRequired ? "..." : "") << "  " << arg->desc << std::endl;
                }
            }

            if(!options.empty())
            {
                out << std::endl << "Options:" << std::endl;
                auto getOptionText = [&out](Option* opt){
                    out << "  " << (opt->alias.empty() ? "" : ("-" + opt->alias + ", ")) << ("--" + opt->name);
                    if(!opt->valueName.empty()) out << " " << (opt->multiValue ? "<" + opt->valueName + (opt->valueIsRequired ? "..." : "") + ">" : 
                                        "[" + opt->valueName + (opt->valueIsRequired ? "..." : "") + "]");
                    out << "  " << opt->desc;
                    if(!opt->valueName.empty())
                    {
                        String defaultValue;
                        do
                        {
                            String *sv = std::get_if<String>(&opt->defaultValue);
                            if (sv)
                            {
                                defaultValue = *sv;
                                break;
                            }

                            bool *bv = std::get_if<bool>(&opt->defaultValue);
                            if (bv)
                            {
                                defaultValue = *bv ? "true" : "false";
                                break;
                            }

                            int *iv = std::get_if<int>(&opt->defaultValue);
                            if (iv)
                            {
                                defaultValue = std::to_string(*iv);
                                break;
                            }

                            double *dv = std::get_if<double>(&opt->defaultValue);
                            if (dv)
                            {
                                defaultValue = std::to_string(*dv);
                                break;
                            }
                        } while (0);
                        if (!defaultValue.empty())
                            out << " (default: " << defaultValue << ")";
                    }

                    out<< std::endl;
                };

                getOptionText(versionOption);
                for(const auto opt : options)
                {
                    getOptionText(opt);
                }
                getOptionText(helpOption);
            }

            if(!subCommands.empty())
            {
                out << std::endl << "Commands:" << std::endl;
                for(const auto cmd : subCommands)
                {
                    out << "  " << getUsageText(cmd) << "  " << cmd->description() << std::endl;
                }
            }

            return out.str();
        };
        return !helpOption->desc.empty() ? helpOption->desc : getHelpText();
    }

    /**
        * @return new Command
        */
    virtual Command* command(const String& nameAndArg, const String& desc = String())
    {
        // std::cout << "nameAndArg: " << nameAndArg << std::endl;
        std::regex reg(R"(^\s*([a-zA-Z][a-zA-Z\d]+)\s*((?:\[[a-zA-Z][a-zA-Z\d]+(?:\.\.\.)?\])|(?:<[a-zA-Z][a-zA-Z\d]+(?:\.\.\.)?>))?\s*$)");
        std::smatch res;
        if(!std::regex_search(nameAndArg, res, reg)) return nullptr;
        // std::cout << res.str(0) << res.str(1) << res.str(2) << res.str(3) << res.str(4) << res.str(5) << std::endl;
        String name = res.str(1);
        String arg = res.str(2);
        // std::cout << "name: " << name << ", arg: " << arg << std::endl;
        Command* cmd = (new Command(name, pLogger))->description(desc)->argument(arg);

        addCommand(cmd);

        return cmd;
    };
    virtual Command* addCommand(Command* command)
    {
        if(!command)
        {
            *pLogger << "[error]:" << "add command failed, command is null";
            return this;
        }

        if(findCommand(command->commandName))
        {
            *pLogger << "[error]:" << "add command failed, command " << command->commandName.c_str() << "already exists";
            return this;
        }

        command->parentCommand = this;
        subCommands.push_back(command);
        return this;
    };

    virtual Command* argument(const String& name, const String& desc = String(), const Variant& defaultValue = Variant())
    {
        Argument* arg = Argument::create(name, pLogger);
        if(!arg)
        {
            return this;
        }

        arg->desc = desc;
        arg->defaultValue = defaultValue;
        arguments.push_back(arg);
        return this;
    };

    virtual Command* option(const String& flag, const String& desc = String(), const Variant& defaultValue = Variant())
    {
        Option* opt = Option::create(flag,pLogger);
        if(!opt)
        {
            *pLogger << "[error]:" << "option " << flag.c_str() << " create failed";
            return this;
        }

        opt->desc = desc;
        opt->defaultValue = defaultValue;
        options.push_back(opt);
        
        return this;
    };

    virtual Command* action(const Action& cb)
    {
        if(!cb)
        {
            *pLogger << "[error]:" << "action callback is null";
        }

        actionCallback = cb;
        return this;
    }

    void parse(int argc, char** argv, int index = 1)
    {
        Vector<String> args;
        Map<String, Variant> opts;

        int cur = index;
        std::regex optionAliasReg(R"(^-([a-zA-Z]+)$)");
        std::regex optionReg(R"(^(?:-([a-zA-Z])|--([a-zA-Z-]+))(?:=(.+))?$)");
        std::regex commandReg(R"(^(?!-)([a-zA-Z][a-zA-Z\d]*)$)");

        std::regex intValueReg(R"(^-?\d+$)");
        std::regex doubleValueReg(R"(^-?\d+\.\d+$)");
        std::regex boolValueReg(R"(^(?:(true)|false)$)");
        

        auto parseMuiltOptionAlias = [&](const String& alias){
            // std::cout << "alias: " << alias << std::endl;
            for(auto it = alias.begin(); it != alias.end(); it++)
            {
                char a[2];
                a[0] = *it;
                a[1] = '\0';
                Option* opt = findOptionByAlias(a);
                if(!opt)
                {
                    *pLogger << "[warn]:" << "option alias " << a << " not found\n";
                    continue;
                }
                if(opt->valueIsRequired)
                {
                    *pLogger << "[error]:" << "option: " << opt->name << " value is required\n";
                    return false;
                }
                opts[opt->name] = Variant();
                std::cout << "alias: " << *it << std::endl;
            }
            cur++;
            return true;
        };
        auto parseOptionName = [&](const String& aliasOrName, const String& value = String()){
            // std::cout << "aliasOrName: " << aliasOrName << ", value: " << value << std::endl;
            Option* opt = findOptionByAlias(aliasOrName);
            if(!opt)
            {
                opt = findOption(aliasOrName);
                if(!opt)
                {
                    *pLogger << "[warn]:" << "option " << aliasOrName << " not found\n";
                    return true;
                }
            }
            Variant v;
            if(opt->valueIsRequired)
            {
                auto getValue = [=](const String& text){
                    if(text.empty())
                    {
                        return Variant();
                    }
                    std::smatch res;
                    if(std::regex_search(text, res, intValueReg))
                    {
                        return Variant(std::stoi(text));
                    }
                    
                    if(std::regex_search(text, res, doubleValueReg))
                    {
                        return Variant(std::stod(text));
                    }
                    
                    if(std::regex_search(text, res, boolValueReg))
                    {
                        return Variant(res.str(1).empty());
                    }

                    return Variant(text);
                };             
                if(opt->multiValue)
                {

                }
                else
                {
                    String valueText = !value.empty() ? value : ++cur < argc ? argv[cur] : "";
                    std::smatch res;
                    if(valueText.empty() || std::regex_search(valueText, res, optionAliasReg) || std::regex_search(valueText, res, optionReg))
                    {
                        *pLogger << "[error]" << "option: " << opt->name << "need a value, but got empty.\n";
                        return false;   
                    }

                    v = getValue(valueText);
                }

                if(std::holds_alternative<std::monostate>(v))
                {
                    *pLogger << "[error]" << "option: "<<opt->name<<"got an invalid value";
                    return false;
                }
            }

            opts[opt->name] = v;
            cur++;
            return true;
        };
        auto parseCommand = [&](const String& name){
            cur++;
            std::cout << "command: " << name << std::endl;
            return true;
        };
        auto parseArgument = [&](const String& arg){
            cur++;
            std::cout << "arg: " << arg << std::endl;
            return true;
        };
        while(cur < argc)
        {
            // std::cout << cur<< std::endl;
            String arg = argv[cur];
            // std::cout << cur << " " << arg << std::endl;
            std::smatch res;
            if(std::regex_search(arg, res, optionAliasReg))
            {
                if(parseMuiltOptionAlias(res.str(1))) continue;
                return;
            }

            if(std::regex_search(arg, res, optionReg))
            {
                if(parseOptionName(!res.str(1).empty() ? res.str(1) : res.str(2), res.str(3))) continue;
                return;
            }

            if(std::regex_search(arg, res, commandReg))
            {
                parseCommand(res.str(1));
                // 如果解析到子命令直接就使用子命令的解析了，不再继续当前的解析了
                return;
            }

            if(parseArgument(arg)) continue;
            return;
        }

        // std::cout << "end..."<< std::endl;
        if(opts.find(versionOption->name) != opts.end())
        {
            std::cout << version() << std::endl;
            return;
        }

        if(opts.find(helpOption->name) != opts.end())
        {
            std::cout << helpText() << std::endl;
            return;
        }

        if(actionCallback)
        {
            actionCallback(args, opts);
            return;
        }
    }

private:
    Command* findCommand(const String& name)
    {
        for(const auto cmd : subCommands)
        {
            if(cmd->commandName == name)
            {
                return cmd;
            }
        }
        return nullptr;
    }

    class Option;
    Option* findOption(const String& name)
    {
        if(name == versionOption->name)
        {
            return versionOption;
        }
        if(name == helpOption->name)
        {
            return helpOption;
        }
        for(const auto opt : options)
        {
            if(opt->name == name)
            {
                return opt;
            }
        }
        return nullptr;
    }
    Option* findOptionByAlias(const String& alias)
    {
        if(alias == versionOption->alias)
        {
            return versionOption;
        }
        if(alias == helpOption->alias)
        {
            return helpOption;
        }
        for(const auto opt : options)
        {
            if(opt->alias == alias)
            {
                return opt;
            }
        }
        return nullptr;
    }

private:
    class Option{
        public:
            static Option* create(const String& flag, Logger* logger)
            {
                std::regex reg(R"(^\s*(?:(?:-([a-zA-Z])(?:(?:\s+)|(?:\s*,\s*))\-\-([a-zA-Z-]+)\s+(?:\[([a-zA-Z]+)(\.\.\.)?\]|<([a-zA-Z]+)(\.\.\.)?>))|(?:-([a-zA-Z])(?:(?:\s+)|(?:\s*,\s*))\-\-([a-zA-Z-]+))|(?:\-\-([a-zA-Z-]+)\s+(?:(?:\[([a-zA-Z]+)(\.\.\.)?\])|(?:\<([a-zA-Z]+)(\.\.\.)?\>)))|(?:\-\-([a-zA-Z-]+)))\s*$)");
                std::smatch res;
                if(!std::regex_search(flag, res, reg))
                {
                    return nullptr;
                }
                std::string alias = !res.str(1).empty() ? res.str(1) :
                                                 !res.str(7).empty() ? res.str(7) : "";
                std::string name = !res.str(2).empty() ? res.str(2) :
                                                 !res.str(8).empty() ? res.str(8) :
                                                 !res.str(9).empty() ? res.str(9) :
                                                 !res.str(14).empty() ? res.str(14) : "";
                std::string valueName = !res.str(3).empty() ? res.str(3) :
                                                 !res.str(5).empty() ? res.str(5) :
                                                 !res.str(10).empty() ? res.str(10) :
                                                 !res.str(12).empty() ? res.str(12) : "";
                                                 
                bool multiValue = !res.str(4).empty() || !res.str(6).empty() || !res.str(11).empty() || !res.str(13).empty();
                bool valueIsRequired = !res.str(5).empty() || !res.str(12).empty();
                // std::cout  << "alias: " << alias  << std::endl
                //             << " name: " << name << std::endl 
                //             << " valueName: " << valueName << std::endl
                //             << "multiValue: " << multiValue << std::endl
                //             << " valueIsRequired: " << valueIsRequired << std::endl;
                // *logger << "create option:" << flag;

                Option* opt = new Option();
                opt->name = name;
                opt->alias = alias;
                opt->valueName = valueName;
                opt->multiValue = multiValue;
                opt->valueIsRequired = valueIsRequired;
                return opt;
            }

            String name;
            String alias;
            String valueName;
            bool multiValue = false;
            bool valueIsRequired = false;

            String desc;
            Variant defaultValue;
    };
    class Argument{
        public:
            static Argument* create(const String& name, Logger* logger)
            {
                std::regex reg(R"(^\s*(?:(?:\[([a-zA-Z][a-zA-Z\d]+)(\.\.\.)?\])|(?:<([a-zA-Z][a-zA-Z\d]+)(\.\.\.)?>))\s*$)");
                std::smatch res;
                if(!std::regex_search(name, res, reg))
                {
                    return nullptr;
                }
                String argName = !res.str(1).empty() ? res.str(1) : !res.str(3).empty() ? res.str(3) : "";
                // std::cout << name << "argName: " << argName << std::endl;

                Argument* arg = new Argument;
                arg->name = argName;
                arg->isMultiValue = !res.str(2).empty() || !res.str(4).empty();
                arg->valueIsRequired = !res.str(1).empty() || !res.str(3).empty();
                return arg;
            }

            String name;
            bool isMultiValue = false;
            bool valueIsRequired = false;

            String desc;
            Variant defaultValue;
    };

    String commandName;
    String commandDescription;
    Action actionCallback;
    Option* versionOption;
    Option* helpOption;
    Command* parentCommand;

    Vector<Option*> options;
    Vector<Command*> subCommands;
    Vector<Argument*> arguments;

    Logger* pLogger;
};
}