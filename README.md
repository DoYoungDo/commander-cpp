# Commander-CPP

Commander-CPP 是一个仿照 [commander.js](https://github.com/tj/commander.js) 实现的 C++ 命令行解析库。它提供了简洁的链式 API，用于定义命令、选项和参数，支持子命令、默认值、多值选项等功能。

## 特性

- 🎯 链式 API 设计，简洁易用
- 📦 单头文件，无外部依赖
- 🔧 支持选项（单值、多值、布尔值）
- 📝 支持参数（必需参数、可选参数、多值参数）
- 🌲 支持子命令和嵌套命令
- ⚙️ 支持默认值
- 📖 自动生成帮助信息
- 🔍 详细的错误处理和日志系统
- 🎨 支持选项别名和组合（如 `-abc`）

## 安装

将 `commander_cpp.hpp` 文件复制到您的项目中，并在代码中包含它：

```cpp
#include "commander_cpp.hpp"
```

## 快速开始

```cpp
#include "commander_cpp.hpp"
using namespace COMMANDER_CPP;

int main(int argc, char **argv) {
    Command("example")
        .version("1.0.0")
        ->description("一个示例命令行应用")
        ->option("-n --name <name>", "你的名字")
        ->action([](Vector<Variant> args, Map<String, Variant> opts) {
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

$ ./example --help
Usage: example [options]

一个示例命令行应用

Options:
  -V, --version       out put version number.
  -n, --name <name>   你的名字
  -h, --help
```

## 核心概念

### 选项语法

选项使用 `-` 或 `--` 前缀，支持以下格式：

- `--option` - 布尔选项
- `-o --option` - 带别名的布尔选项
- `--option <value>` - 必需值的选项
- `--option [value]` - 可选值的选项
- `--option <values...>` - 多值选项（至少一个）
- `--option [values...]` - 多值选项（可选）

### 参数语法

参数不使用前缀，支持以下格式：

- `<arg>` - 必需参数
- `[arg]` - 可选参数
- `<args...>` - 多值必需参数
- `[args...]` - 多值可选参数

## 详细用法

### 1. 定义选项

```cpp
Command("app")
    .option("-s --single <value>", "单值选项")
    ->option("-m --multi <values...>", "多值选项")
    ->option("-b --boolean", "布尔选项")
    ->option("-n --number <num>", "数字选项", 42)  // 带默认值
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        // 获取单值选项
        if (opts.find("single") != opts.end()) {
            auto value = std::get<String>(opts["single"]);
            std::cout << "Single: " << value << std::endl;
        }
        
        // 获取多值选项
        if (opts.find("multi") != opts.end()) {
            auto values = std::get<std::vector<VariantBase>>(opts["multi"]);
            for (const auto &val : values) {
                std::cout << std::get<String>(val) << " ";
            }
            std::cout << std::endl;
        }
        
        // 获取布尔选项
        if (opts.find("boolean") != opts.end()) {
            std::cout << "Boolean enabled" << std::endl;
        }
    });
```

使用示例：

```bash
$ ./app -s value1
$ ./app --single value1
$ ./app -m file1.txt file2.txt file3.txt
$ ./app -b
$ ./app -bs value1  # 组合选项
```

### 2. 定义参数

```cpp
Command("copy")
    .argument("<from>", "源文件")
    ->argument("[to]", "目标文件", "default.txt")  // 带默认值
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        auto from = std::get<String>(args[0]);
        std::cout << "From: " << from << std::endl;
        
        if (args.size() > 1) {
            auto to = std::get<String>(args[1]);
            std::cout << "To: " << to << std::endl;
        }
    });
```

使用示例：

```bash
$ ./copy source.txt
$ ./copy source.txt target.txt
```

### 3. 定义子命令

```cpp
Command("git")
    .version("1.0.0")
    ->description("Git 命令行工具")
    ->command("add <files...>", "添加文件到暂存区")
    ->option("-f --force", "强制添加")
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        auto files = std::get<std::vector<VariantBase>>(args[0]);
        std::cout << "Adding files..." << std::endl;
        for (const auto &file : files) {
            std::cout << "  " << std::get<String>(file) << std::endl;
        }
    });
    // 添加另一个子命令
    ->command("commit", "提交更改")
    ->option("-m --message <msg>", "提交信息")
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        if (opts.find("message") != opts.end()) {
            auto msg = std::get<String>(opts["message"]);
            std::cout << "Commit: " << msg << std::endl;
        }
    });
```

使用示例：

```bash
$ ./git add file1.txt file2.txt
$ ./git add -f file.txt
$ ./git commit -m "Initial commit"
```

### 4. 类型支持

Commander-CPP 自动识别并转换以下类型：

```cpp
Command("types")
    .option("-i --int <num>", "整数")
    ->option("-d --double <num>", "浮点数")
    ->option("-b --bool <val>", "布尔值")
    ->option("-s --string <text>", "字符串")
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        // 自动类型转换
        auto intVal = std::get<int>(opts["int"]);        // 42
        auto doubleVal = std::get<double>(opts["double"]); // 3.14
        auto boolVal = std::get<bool>(opts["bool"]);     // true/false
        auto strVal = std::get<String>(opts["string"]);  // "text"
    });
```

使用示例：

```bash
$ ./types -i 42 -d 3.14 -b true -s hello
```

### 5. 自定义日志

```cpp
class CustomLogger : public Logger {
  public:
    virtual Logger *error(const String &msg) override {
        std::cerr << "[ERROR] " << msg << std::endl;
        return this;
    }
    virtual Logger *warn(const String &msg) override {
        std::cerr << "[WARN] " << msg << std::endl;
        return this;
    }
    virtual Logger *debug(const String &msg) override {
        std::cout << "[DEBUG] " << msg << std::endl;
        return this;
    }
    virtual Logger *print(const String &msg) override {
        std::cout << msg << std::endl;
        return this;
    }
};

int main(int argc, char **argv) {
    CustomLogger logger;
    Command("app", &logger)
        .option("-v --verbose", "详细输出")
        ->parse(argc, argv);
    return 0;
}
```

### 6. 选项组合

支持短选项组合（类似 `tar -xzvf`）：

```cpp
Command("app")
    .option("-a --option-a", "选项 A")
    ->option("-b --option-b", "选项 B")
    ->option("-c --option-c <value>", "选项 C")
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        if (opts.find("option-a") != opts.end()) {
            std::cout << "A enabled" << std::endl;
        }
        if (opts.find("option-b") != opts.end()) {
            std::cout << "B enabled" << std::endl;
        }
    });
```

使用示例：

```bash
$ ./app -ab           # 同时启用 A 和 B
$ ./app -abc value    # 启用 A、B，并为 C 设置值
```

## 完整示例

基于 `main.cpp` 中的集成测试，这是一个完整的待办事项应用示例：

```cpp
#include "commander_cpp.hpp"
using namespace COMMANDER_CPP;

int main(int argc, char **argv) {
    Command("todo")
        .version("1.0.0")
        ->description("待办事项管理工具")
        
        // 添加子命令
        ->command("add <todos...>", "添加待办事项")
        ->option("-d --done", "标记为已完成")
        ->option("-p --priority", "设置为高优先级")
        ->action([](Vector<Variant> args, Map<String, Variant> opts) {
            auto todos = std::get<std::vector<VariantBase>>(args[0]);
            for (const auto &todo : todos) {
                std::cout << "Added: " << std::get<String>(todo);
                if (opts.find("done") != opts.end()) {
                    std::cout << " [DONE]";
                }
                if (opts.find("priority") != opts.end()) {
                    std::cout << " [HIGH]";
                }
                std::cout << std::endl;
            }
        });
    
    // 删除子命令
    Command("todo")
        .command("rm", "删除待办事项")
        ->argument("<index...>", "待办事项索引")
        ->option("-l --level <value>", "删除级别")
        ->action([](Vector<Variant> args, Map<String, Variant> opts) {
            auto indices = std::get<std::vector<VariantBase>>(args[0]);
            for (const auto &idx : indices) {
                std::cout << "Removed: " << std::get<String>(idx) << std::endl;
            }
        });
    
    Command("todo").parse(argc, argv);
    return 0;
}
```

使用示例：

```bash
$ ./todo add "学习 C++" "写代码" -p
Added: 学习 C++ [HIGH]
Added: 写代码 [HIGH]

$ ./todo rm 1 2
Removed: 1
Removed: 2

$ ./todo --help
Usage: todo [options]

待办事项管理工具

Options:
  -V, --version       out put version number.
  -h, --help

Commands:
  add [options] <todos...>  添加待办事项
  rm [options] <index...>   删除待办事项
```

## 测试

项目包含完整的测试套件（见 `src/main.cpp`），覆盖以下场景：

| 测试用例 | 描述 |
|---------|------|
| VersionTest | 验证版本信息输出 |
| DescriptionTest | 验证帮助信息生成 |
| OptionTest | 测试单值和多值选项解析 |
| ArgumentTest | 测试必需和可选参数 |
| SubCommandTest | 测试子命令功能 |
| DefaultValueTest | 测试默认值机制 |
| MultiValueOptionTest | 测试多值选项解析 |
| ErrorHandlingTest | 测试错误处理 |
| ComplexOptionTest | 测试选项组合 |
| IntegratedTest | 集成测试 |

运行测试：

```bash
$ xmake
$ ./build/macosx/arm64/debug/commander-cpp
$ ./build/macosx/arm64/debug/commander-cpp -i  # 显示详细信息
```

## 构建

项目使用 [xmake](https://xmake.io/) 构建：

```bash
# 安装 xmake
$ brew install xmake

# 构建项目
$ xmake

# 运行测试
$ xmake run commander-cpp
```

## 目录结构

```
commander-cpp/
├── src/
│   ├── commander_cpp.hpp   # 核心库（单头文件）
│   └── main.cpp            # 测试用例
├── build/                  # 构建输出
├── xmake.lua              # 构建配置
└── README.md              # 本文档
```

## 未来计划

- [ ] 支持配置文件读取

## 贡献

欢迎提交 Issue 和 Pull Request！

## 许可证

MIT License