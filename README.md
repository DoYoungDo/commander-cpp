# Commander-CPP

Commander-CPP 是一个仿照 [commander.js](https://github.com/tj/commander.js) 实现的 C++ 命令行解析库。它提供了一个简单易用的接口，用于定义命令、选项和参数，并支持子命令、默认值、多值选项等功能。

## 特性

- 定义命令和子命令
- 支持选项（单值、多值、布尔值）
- 支持参数（必需参数、可选参数）
- 支持默认值
- 支持版本和帮助信息
- 支持复杂选项组合
- 提供详细的错误处理

## 安装

将 `commander_cpp.hpp` 文件复制到您的项目中，并在代码中包含它：

```cpp
#include "commander_cpp.hpp"
```

## 快速开始

以下是一个简单的示例：

```cpp
#include "commander_cpp.hpp"
using namespace COMMANDER_CPP;

int main(int argc, char **argv) {
    Command("example")
        .version("1.0.0")
        .description("An example command-line application.")
        .option("-n --name <name>", "Your name")
        .action([](Vector<Variant> args, Map<String, Variant> opts) {
            if (opts.find("name") != opts.end()) {
                std::cout << "Hello, " << std::get<String>(opts["name"]) << "!" << std::endl;
            } else {
                std::cout << "Hello, World!" << std::endl;
            }
        })
        .parse(argc, argv);
    return 0;
}
```

运行示例：

```bash
$ ./example -n Alice
Hello, Alice!
```

## 用法

### 定义命令

使用 `Command` 类可以定义一个命令：

```cpp
Command("myCommand")
    .description("This is a custom command.")
    .action([](Vector<Variant> args, Map<String, Variant> opts) {
        std::cout << "Command executed!" << std::endl;
    });
```

### 添加选项

选项可以是单值、多值或布尔值：

```cpp
Command("example")
    .option("-s --single <value>", "A single value option")
    .option("-m --multi <values...>", "A multi-value option")
    .option("-b --boolean", "A boolean option")
    .action([](Vector<Variant> args, Map<String, Variant> opts) {
        if (opts.find("single") != opts.end()) {
            std::cout << "Single: " << std::get<String>(opts["single"]) << std::endl;
        }
        if (opts.find("multi") != opts.end()) {
            auto values = std::get<std::vector<VariantBase>>(opts["multi"]);
            std::cout << "Multi: ";
            for (const auto &val : values) {
                std::cout << std::get<String>(val) << " ";
            }
            std::cout << std::endl;
        }
        if (opts.find("boolean") != opts.end()) {
            std::cout << "Boolean: true" << std::endl;
        }
    });
```

### 添加参数

参数可以是必需参数或可选参数：

```cpp
Command("example")
    .argument("<required>", "A required argument")
    .argument("[optional]", "An optional argument")
    .action([](Vector<Variant> args, Map<String, Variant> opts) {
        if (!args.empty()) {
            std::cout << "Required: " << std::get<String>(args[0]) << std::endl;
            if (args.size() > 1) {
                std::cout << "Optional: " << std::get<String>(args[1]) << std::endl;
            }
        }
    });
```

### 定义子命令

可以为命令添加子命令：

```cpp
Command("main")
    .command("sub <file>", "A subcommand with a required argument")
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        std::cout << "Subcommand executed with file: " << std::get<String>(args[0]) << std::endl;
    });
```

### 默认值

选项和参数可以设置默认值：

```cpp
Command("example")
    .option("-n --number <num>", "A number option", 42)
    .argument("[optional]", "An optional argument", "default_value")
    .action([](Vector<Variant> args, Map<String, Variant> opts) {
        std::cout << "Number: " << std::get<int>(opts["number"]) << std::endl;
        if (!args.empty()) {
            std::cout << "Optional: " << std::get<String>(args[0]) << std::endl;
        }
    });
```

### 错误处理

错误处理通过传入一个自定义的 `Logger` 对象来实现。所有的错误信息会输出到该 `Logger` 对象中，用户可以根据需要自定义日志的处理方式。例如：

```cpp
class CustomLogger : public Logger {
  public:
    virtual Logger *error(const String &msg) override {
        std::cerr << "[ERROR]: " << msg << std::endl;
        return this;
    }
    virtual Logger *warn(const String &msg) override {
        std::cerr << "[WARN]: " << msg << std::endl;
        return this;
    }
    virtual Logger *debug(const String &msg) override {
        std::cout << "[DEBUG]: " << msg << std::endl;
        return this;
    }
    virtual Logger *print(const String &msg) override {
        std::cout << msg << std::endl;
        return this;
    }
};

int main(int argc, char **argv) {
    CustomLogger logger;
    Command("example", &logger)
        .option("-r --required <value>", "A required option")
        .action([](Vector<Variant> args, Map<String, Variant> opts) {
            if (opts.find("required") == opts.end()) {
                std::cerr << "This should not happen!" << std::endl;
            }
        })
        .parse(argc, argv);
    return 0;
}
```

在上面的示例中，`CustomLogger` 会捕获所有的错误、警告和调试信息，并将其输出到标准错误或标准输出流。

### 复杂选项组合

支持复杂的选项组合：

```cpp
Command("example")
    .option("-d --debug", "Enable debug mode", false)
    .option("-v --verbose", "Enable verbose mode", false)
    .action([](Vector<Variant> args, Map<String, Variant> opts) {
        if (opts["debug"].index() != 0 && std::get<bool>(opts["debug"])) {
            std::cout << "Debug mode enabled" << std::endl;
        }
        if (opts["verbose"].index() != 0 && std::get<bool>(opts["verbose"])) {
            std::cout << "Verbose mode enabled" << std::endl;
        }
    });
```

## 测试用例

项目中包含多个测试用例，验证了库的核心功能。以下是测试用例的概述：

### 1. VersionTest
验证 `--version` 和 `-V` 选项是否正确输出版本号。

### 2. DescriptionTest
验证 `--help` 和 `-h` 是否正确输出命令描述。

### 3. OptionTest
测试选项解析，包括必需选项和多值选项。

### 4. ArgumentTest
测试参数解析，包括必需参数和可选参数。

### 5. SubCommandTest
测试子命令功能，包括添加和删除子命令。

### 6. DefaultValueTest
测试选项和参数的默认值功能。

### 7. MultiValueOptionTest
测试多值选项的解析。

### 8. ErrorHandlingTest
测试错误处理，包括缺少必需选项和未知选项的错误。

### 9. ComplexOptionTest
测试复杂选项组合的解析。

### 10. IntegratedTest
集成测试，验证命令行解析的整体功能。

## 目录结构

```
commander-cpp/
├── build/                  # 构建目录
├── src/                    # 源代码目录
│   ├── commander_cpp.hpp   # 主头文件
│   └── main.cpp            # 测试用例
└── xmake.lua               # 构建配置文件
```

## 构建与运行

项目使用 [xmake](https://xmake.io/) 进行构建。以下是构建和运行的步骤：

1. 安装 xmake：
   ```bash
   $ brew install xmake
   ```

2. 构建项目：
   ```bash
   $ xmake
   ```

3. 运行测试：
   ```bash
   $ ./build/macosx/arm64/debug/commander-cpp
   ```

## 贡献

欢迎提交 Issue 和 Pull Request！

## 许可证

该项目使用 MIT 许可证。详情请参阅 [LICENSE](LICENSE) 文件。