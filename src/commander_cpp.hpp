#ifndef COMMANDER_CPP_HPP
#define COMMANDER_CPP_HPP

#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <variant>
#include <vector>

namespace COMMANDER_CPP
{
using String = std::string;
using VariantBase = std::variant<std::monostate, int, double, String, bool>;
using Variant = std::variant<std::monostate, int, double, String, bool, std::vector<VariantBase>>;
template <typename K, typename V> using Map = std::map<K, V>;
template <typename T> using Vector = std::vector<T>;
using Action = std::function<void(Vector<Variant> args, Map<String, Variant> opts)>;

class FinialRelease
{
  public:
    FinialRelease(std::function<void()> release) : r(release)
    {
    }
    ~FinialRelease()
    {
        if (r)
            r();
    }

  private:
    std::function<void()> r;
};

class Logger
{
  public:
    virtual Logger *debug(const String &msg)
    {
        return this;
    };
    virtual Logger *warn(const String &msg)
    {
        return this;
    };
    virtual Logger *error(const String &msg)
    {
        return this;
    };
    virtual Logger *print(const String &msg) = 0;
};

class LoggerDefaultImpl : public Logger
{
  public:
    virtual Logger *print(const String &msg) override
    {
        std::cout << msg << std::endl;
        return this;
    };
};

class Command
{
  public:
    Command(const String &name = String(), Logger *logger = new LoggerDefaultImpl())
        : commandName(name), actionCallback(nullptr), pLogger(logger), versionOption(nullptr), helpOption(nullptr),
          parentCommand(nullptr)
    {
        version("0.0.0", "-V --version", "out put version number.");
        help("-h --help");
    }
    ~Command()
    {
        if (versionOption)
        {
            delete versionOption;
            versionOption = nullptr;
        }
        if (helpOption)
        {
            delete helpOption;
            helpOption = nullptr;
        }

        for (const auto opt : options)
        {
            delete opt;
        }
        options.clear();

        for (const auto cmd : subCommands)
        {
            delete cmd;
        }
        subCommands.clear();

        for (const auto arg : arguments)
        {
            delete arg;
        }
        arguments.clear();
    }

  public:
    virtual Command *name(const String &name)
    {
        commandName = name;
        return this;
    }
    virtual String name()
    {
        return commandName;
    }

    virtual Command *version(const String &v, const String &flag = String(), const String &desc = String())
    {
        // 函数结束时调用
        FinialRelease f([this, v, desc]() {
            if (versionOption)
            {
                versionOption->defaultValue = v;
                versionOption->desc = desc;
            }
        });

        if (!flag.empty())
        {
            Option *opt = Option::create(flag, pLogger);
            if (!opt)
            {
                return this;
            }

            if (versionOption)
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
        String *v = std::get_if<String>(&versionOption->defaultValue);
        return v ? *v : "0.0.0";
    }

    virtual Command *description(const String &desc)
    {
        commandDescription = desc;
        return this;
    };
    virtual String description()
    {
        return commandDescription;
    }

    virtual Command *help(const String &flag = String(), const String &desc = String())
    {
        FinialRelease r([this, desc]() {
            if (helpOption)
            {
                helpOption->desc = desc;
            }
        });

        if (!flag.empty())
        {
            Option *opt = Option::create(flag, pLogger);
            if (!opt)
            {
                return this;
            }

            if (helpOption)
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
        auto getHelpText = [this]() {
            auto getUsageText = [this](Command *cmd) {
                std::stringstream out;
                out << cmd->commandName;
                if (cmd->options.size())
                    out << " [options]";
                String argumentText;
                for (const auto arg : cmd->arguments)
                {
                    argumentText +=
                        " " + (arg->isMultiValue ? "<" + arg->name + (arg->valueIsRequired ? "..." : "") + ">"
                                                 : "[" + arg->name + (arg->valueIsRequired ? "..." : "") + "]");
                }
                if (cmd->arguments.size())
                    out << argumentText;

                return out.str();
            };

            String usageText = getUsageText(this);
            Command *p = parentCommand;
            while (p)
            {
                usageText = p->commandName + " " + usageText;
                p = p->parentCommand;
            }

            std::stringstream out;
            out << "Usage: " << usageText << std::endl;

            String descText = description();
            if (!descText.empty())
            {
                out << std::endl << description() << std::endl;
            }

            if (!arguments.empty())
            {
                out << std::endl << "Arguments:" << std::endl;
                for (const auto arg : arguments)
                {
                    out << "  " << arg->name << (arg->valueIsRequired ? "..." : "") << "  " << arg->desc << std::endl;
                }
            }

            do
            {
                out << std::endl << "Options:" << std::endl;
                auto getOptionText = [&out](Option *opt) {
                    out << "  " << (opt->alias.empty() ? "" : ("-" + opt->alias + ", ")) << ("--" + opt->name);
                    if (!opt->valueName.empty())
                        out << " "
                            << (opt->multiValue ? "<" + opt->valueName + (opt->valueIsRequired ? "..." : "") + ">"
                                                : "[" + opt->valueName + (opt->valueIsRequired ? "..." : "") + "]");
                    out << "  " << opt->desc;
                    if (!opt->valueName.empty())
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

                    out << std::endl;
                };

                getOptionText(versionOption);
                for (const auto opt : options)
                {
                    getOptionText(opt);
                }
                getOptionText(helpOption);
            } while (false);

            if (!subCommands.empty())
            {
                out << std::endl << "Commands:" << std::endl;
                for (const auto cmd : subCommands)
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
    virtual Command *command(const String &nameAndArg, const String &desc = String())
    {
        if (pLogger)
            pLogger->debug(String("create command: nameAndArg: ") + nameAndArg);
        std::regex reg(
            R"(^\s*([a-zA-Z][a-zA-Z\d]+)\s*((?:\[[a-zA-Z][a-zA-Z\d]+(?:\.\.\.)?\])|(?:<[a-zA-Z][a-zA-Z\d]+(?:\.\.\.)?>))?\s*$)");
        std::smatch res;
        if (!std::regex_search(nameAndArg, res, reg))
            return nullptr;
        String name = res.str(1);
        String arg = res.str(2);
        Command *cmd = (new Command(name, pLogger))->description(desc)->argument(arg);

        addCommand(cmd);

        return cmd;
    };
    virtual Command *addCommand(Command *command)
    {
        if (!command)
        {
            if (pLogger)
                pLogger->debug(String("error:") + String("add command failed, command is null"));
            return this;
        }

        if (findCommand(command->commandName))
        {
            if (pLogger)
                pLogger->debug(String("[error]:") + String("add command failed, command ") + command->commandName +
                               String(" already exists"));
            return this;
        }

        command->parentCommand = this;
        command->versionOption = versionOption;
        command->helpOption = helpOption;

        subCommands.push_back(command);
        return this;
    };

    virtual Command *argument(const String &name, const String &desc = String(),
                              const Variant &defaultValue = Variant())
    {
        Argument *arg = Argument::create(name, pLogger);
        if (!arg)
        {
            return this;
        }

        arg->desc = desc;
        arg->defaultValue = defaultValue;
        arguments.push_back(arg);
        return this;
    };

    virtual Command *option(const String &flag, const String &desc = String(), const Variant &defaultValue = Variant())
    {
        Option *opt = Option::create(flag, pLogger);
        if (!opt)
        {
            if (pLogger)
                pLogger->debug(String("[error]:") + String("option ") + flag + String(" create failed"));
            return this;
        }

        opt->desc = desc;
        opt->defaultValue = defaultValue;
        options.push_back(opt);

        return this;
    };

    /**
     * @brief 设置命令的动作回调函数
     * @param cb 动作回调函数，参数为参数列表和选项列表
     */
    virtual Command *action(const Action &cb)
    {
        if (!cb)
        {
            if (pLogger)
                pLogger->debug(String("[error]:") + String("action callback is null"));
        }

        actionCallback = cb;
        return this;
    }

    /**
     * @param argc
     * @param argv
     * @param index 开始解析的索引，默认从1开始，0为命令本身
     */
    void parse(int argc, char **argv, int index = 1)
    {
        Vector<Variant> args;
        Map<String, Variant> opts;

        int cur = index;
        std::regex optionAliasReg(R"(^-([a-zA-Z]+)$)");
        std::regex optionReg(R"(^(?:-([a-zA-Z])|--([a-zA-Z-]+))(?:=(.+))?$)");
        std::regex commandReg(R"(^(?!-)([a-zA-Z][a-zA-Z\d]*)$)");

        std::regex intValueReg(R"(^-?\d+$)");
        std::regex doubleValueReg(R"(^-?\d+\.\d+$)");
        std::regex boolValueReg(R"(^(?:(true)|false)$)");

        auto getBaseValue = [=](const String &text) {
            if (text.empty())
            {
                return VariantBase();
            }
            std::smatch res;
            if (std::regex_search(text, res, intValueReg))
            {
                return VariantBase(std::stoi(text));
            }

            if (std::regex_search(text, res, doubleValueReg))
            {
                return VariantBase(std::stod(text));
            }

            if (std::regex_search(text, res, boolValueReg))
            {
                return VariantBase(res.str(1).empty());
            }

            return VariantBase(text);
        };

        auto getValue = [=](const String &text) {
            if (text.empty())
            {
                return Variant();
            }
            std::smatch res;
            if (std::regex_search(text, res, intValueReg))
            {
                return Variant(std::stoi(text));
            }

            if (std::regex_search(text, res, doubleValueReg))
            {
                return Variant(std::stod(text));
            }

            if (std::regex_search(text, res, boolValueReg))
            {
                return Variant(res.str(1).empty());
            }

            return Variant(text);
        };
        auto parseMuiltOptionAlias = [&](const String &alias) {
            if (pLogger)
                pLogger->debug(String("parse multi option alias: ") + alias);
            for (auto it = alias.begin(); it != alias.end() - 1; it++)
            {
                char a[2];
                a[0] = *it;
                a[1] = '\0';
                Option *opt = findOptionByAlias(a);
                if (!opt)
                {
                    if (pLogger)
                        pLogger->debug(String("warn:") + String("option alias ") + a + String(" not found\n"));
                    continue;
                }
                if (opt->valueIsRequired)
                {
                    if (pLogger)
                        pLogger->error(String("option: ") + opt->name + String(" value is required"));
                    ++cur;
                    return false;
                }
                opts[opt->name] = Variant();
                if (pLogger)
                    pLogger->debug(String("parse multi option alias: ") + a + String(" success"));
            }

            char a[2];
            a[0] = alias.back();
            a[1] = '\0';
            Option *opt = findOptionByAlias(a);
            if (!opt)
            {
                if (pLogger)
                    pLogger->warn(String("warn:") + String("option alias ") + a + String(" not found\n"));
                ++cur;
                return true;
            }
            if (opt->valueIsRequired)
            {
                Variant v;
                if (opt->multiValue)
                {
                    std::vector<VariantBase> mv;

                    while (cur + 1 < argc)
                    {
                        String arg = argv[cur + 1];
                        std::smatch res;
                        if (std::regex_search(arg, res, optionAliasReg) || std::regex_search(arg, res, optionReg))
                        {
                            break;
                        }
                        auto nv = getBaseValue(arg);
                        if (std::holds_alternative<std::monostate>(nv))
                        {
                            pLogger->error(String("option: ") + opt->name + String(" got an invalid value: ") + arg);
                            return false;
                        }

                        ++cur;
                        mv.push_back(nv);
                    }

                    if (mv.size() == 0)
                    {
                        pLogger->error(String("option: ") + opt->name + String(" need a value at lest, but got zero."));
                        return false;
                    }

                    v = mv;
                }
                else
                {
                    String valueText = ++cur < argc ? argv[cur] : "";
                    std::smatch res;
                    if (valueText.empty() || std::regex_search(valueText, res, optionAliasReg) ||
                        std::regex_search(valueText, res, optionReg))
                    {
                        pLogger->error(String("option: ") + opt->name + String(" need a value, but got zero."));
                        return false;
                    }

                    v = getValue(valueText);
                }
                opts[opt->name] = v;
            }
            else
            {
                opts[opt->name] = Variant();
            }

            if (pLogger)
                pLogger->debug(String("parse multi option alias: ") + a + String(" success"));

            ++cur;
            return true;
        };
        auto parseOptionName = [&](const String &aliasOrName, const String &value = String()) {
            if (pLogger)
                pLogger->debug(String("parse option name: ") + aliasOrName + String(" value: ") + value);
            Option *opt = findOptionByAlias(aliasOrName);
            if (!opt)
            {
                opt = findOption(aliasOrName);
                if (!opt)
                {
                    if (pLogger)
                        pLogger->warn(String("unknown option: ") + aliasOrName);
                    ++cur;
                    return true;
                }
            }
            Variant v;
            if (opt->valueIsRequired)
            {
                if (opt->multiValue)
                {
                    std::vector<VariantBase> mv;
                    if (!value.empty())
                    {
                        mv.push_back(getBaseValue(value));
                    }
                    else
                    {
                        while (++cur < argc)
                        {
                            String arg = argv[cur];
                            std::smatch res;
                            if (std::regex_search(arg, res, optionAliasReg) || std::regex_search(arg, res, optionReg))
                            {
                                --cur;
                                break;
                            }
                            auto nv = getBaseValue(arg);
                            if (std::holds_alternative<std::monostate>(nv))
                                continue;

                            mv.push_back(nv);
                        }
                    }

                    if (mv.size() == 0)
                    {
                        pLogger->error(String("option: ") + opt->name + String(" need a value at lest, but got zero."));
                        ++cur;
                        return false;
                    }

                    v = mv;
                }
                else
                {
                    String valueText = !value.empty() ? value : ++cur < argc ? argv[cur] : "";
                    std::smatch res;
                    if (valueText.empty() || std::regex_search(valueText, res, optionAliasReg) ||
                        std::regex_search(valueText, res, optionReg))
                    {
                        pLogger->error(String("option: ") + opt->name + String(" need a value, but got zero."));
                        ++cur;
                        return false;
                    }

                    v = getValue(valueText);
                }

                if (std::holds_alternative<std::monostate>(v))
                {
                    pLogger->error(String("option: ") + opt->name + String("got an invalid value."));
                    return false;
                }
            }

            opts[opt->name] = v;
            cur++;
            return true;
        };
        auto parseCommand = [&](const String &name) {
            if (pLogger)
                pLogger->debug(String("parse command: ") + name);
            Command *command = findCommand(name);
            if (!command)
            {
                if (pLogger)
                    pLogger->warn(String("unknown identifier: ") + name);
                return false;
            }

            command->parse(argc, argv, ++cur);
            return true;
        };
        auto parseArgument = [&](const String &arg) {
            cur++;
            if (pLogger)
                pLogger->debug(String("parse argument: ") + arg);

            args.push_back(getValue(arg));
            return true;
        };
        while (cur < argc)
        {
            String arg = argv[cur];
            if (pLogger)
                pLogger->debug(String("parse arg: ") + arg);
            std::smatch res;

            if (std::regex_search(arg, res, commandReg))
            {
                // 如果解析到子命令直接就使用子命令的解析了，不再继续当前的解析了
                if (parseCommand(res.str(1)))
                    return;
                // 否则继续解析
            }

            if (std::regex_search(arg, res, optionAliasReg))
            {
                if (parseMuiltOptionAlias(res.str(1)))
                    continue;
                return;
            }

            if (std::regex_search(arg, res, optionReg))
            {
                if (parseOptionName(!res.str(1).empty() ? res.str(1) : res.str(2), res.str(3)))
                    continue;
                return;
            }

            if (parseArgument(arg))
                continue;
            return;
        }

        if (opts.find(versionOption->name) != opts.end())
        {
            if (pLogger)
                pLogger->print(version());
            return;
        }

        if (opts.find(helpOption->name) != opts.end())
        {
            if (pLogger)
                pLogger->print(helpText());
            return;
        }

        for (const auto arg : arguments)
        {
            if (arg->valueIsRequired)
            {
                if (args.empty())
                {
                    pLogger->error(String("argument: ") + arg->name + String(" is required, but got empty."));
                    return;
                }
                break;
            }
        }

        if (actionCallback)
        {
            actionCallback(args, opts);
            return;
        }
    }

  private:
    Command *findCommand(const String &name)
    {
        for (const auto cmd : subCommands)
        {
            if (cmd->commandName == name)
            {
                return cmd;
            }
        }
        return nullptr;
    }

    class Option;
    Option *findOption(const String &name)
    {
        Vector<Option *> opts = options;
        opts.push_back(versionOption);
        opts.push_back(helpOption);
        for (const auto opt : opts)
        {
            if (opt->name == name)
            {
                return opt;
            }
        }
        return nullptr;
    }
    Option *findOptionByAlias(const String &alias)
    {
        Vector<Option *> opts = options;
        opts.push_back(versionOption);
        opts.push_back(helpOption);
        for (const auto opt : opts)
        {
            if (opt->alias == alias)
            {
                return opt;
            }
        }
        return nullptr;
    }

  private:
    class Option
    {
      public:
        static Option *create(const String &flag, Logger *logger)
        {
            std::regex reg(
                R"(^\s*(?:(?:-([a-zA-Z])(?:(?:\s+)|(?:\s*,\s*))\-\-([a-zA-Z-]+)\s+(?:\[([a-zA-Z]+)(\.\.\.)?\]|<([a-zA-Z]+)(\.\.\.)?>))|(?:-([a-zA-Z])(?:(?:\s+)|(?:\s*,\s*))\-\-([a-zA-Z-]+))|(?:\-\-([a-zA-Z-]+)\s+(?:(?:\[([a-zA-Z]+)(\.\.\.)?\])|(?:\<([a-zA-Z]+)(\.\.\.)?\>)))|(?:\-\-([a-zA-Z-]+)))\s*$)");
            std::smatch res;
            if (!std::regex_search(flag, res, reg))
            {
                return nullptr;
            }
            std::string alias = !res.str(1).empty() ? res.str(1) : !res.str(7).empty() ? res.str(7) : "";
            std::string name = !res.str(2).empty()    ? res.str(2)
                               : !res.str(8).empty()  ? res.str(8)
                               : !res.str(9).empty()  ? res.str(9)
                               : !res.str(14).empty() ? res.str(14)
                                                      : "";
            std::string valueName = !res.str(3).empty()    ? res.str(3)
                                    : !res.str(5).empty()  ? res.str(5)
                                    : !res.str(10).empty() ? res.str(10)
                                    : !res.str(12).empty() ? res.str(12)
                                                           : "";

            bool multiValue =
                !res.str(4).empty() || !res.str(6).empty() || !res.str(11).empty() || !res.str(13).empty();
            bool valueIsRequired = !res.str(5).empty() || !res.str(12).empty();

            if (logger)
                logger->debug(String("create option: ") + String("alias: ") + alias + String(" name: ") + name +
                              String(" valueName: ") + valueName + String(" multiValue: ") +
                              std::to_string(multiValue) + String(" valueIsRequired: ") +
                              std::to_string(valueIsRequired));

            Option *opt = new Option();
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
    class Argument
    {
      public:
        static Argument *create(const String &name, Logger *logger)
        {
            std::regex reg(
                R"(^\s*(?:(?:\[([a-zA-Z][a-zA-Z\d]+)(\.\.\.)?\])|(?:<([a-zA-Z][a-zA-Z\d]+)(\.\.\.)?>))\s*$)");
            std::smatch res;
            if (!std::regex_search(name, res, reg))
            {
                return nullptr;
            }
            String argName = !res.str(1).empty() ? res.str(1) : !res.str(3).empty() ? res.str(3) : "";
            if (logger)
                logger->debug(String("create argument: ") + String("name: ") + argName + String(" isMultiValue: ") +
                              std::to_string(!res.str(2).empty() || !res.str(4).empty()) +
                              String(" valueIsRequired: ") +
                              std::to_string(!res.str(1).empty() || !res.str(3).empty()));

            Argument *arg = new Argument;
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
    Option *versionOption;
    Option *helpOption;
    Command *parentCommand;

    Vector<Option *> options;
    Vector<Argument *> arguments;
    Vector<Command *> subCommands;

  protected:
    Logger *pLogger;
};
} // namespace COMMANDER_CPP

#endif // COMMANDER_CPP_HPP