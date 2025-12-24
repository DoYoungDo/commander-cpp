# Commander-CPP

一个C++版本的commander.js命令行参数解析库，提供简洁易用的API来构建命令行工具。

## 特性

- 🚀 **单头文件设计** - 只需包含一个头文件即可使用
- 📝 **链式API** - 类似commander.js的流畅API设计
- 🔧 **类型安全** - 支持多种数据类型（int, double, string, bool, vector）
- 🌐 **多语言支持** - 支持中文等Unicode字符
- 📚 **子命令支持** - 支持嵌套的命令结构
- 🎯 **自动帮助生成** - 自动生成格式化的帮助信息
- 🛡️ **参数验证** - 内置参数验证和错误处理

## 快速开始

### 安装

只需将 `commander_cpp.hpp` 头文件包含到你的项目中：

```cpp
#include "commander_cpp.hpp"
using namespace COMMANDER_CPP;
```

### 基本用法

```cpp
#include "commander_cpp.hpp"
using namespace COMMANDER_CPP;

int main(int argc, char** argv) {
    Command("myapp")
        ->version("1.0.0")
        ->description("一个示例命令行工具")
        ->option("-p --port <port>", "端口号", 8080)
        ->option("-h --host <host>", "主机地址", "localhost")
        ->action([](Vector<Variant> args, Map<String, Variant> opts) {
            // 处理逻辑
            int port = std::get<int>(opts["port"]);
            String host = std::get<String>(opts["host"]);
            std::cout << "服务器运行在: " << host << ":" << port << std::endl;
        })
        ->parse(argc, argv);
    
    return 0;
}
```

### 构建

项目使用xmake构建系统：

```bash
# 构建项目
xmake

# 运行测试
xmake run commander-cpp

# 运行测试并显示成功信息
xmake run commander-cpp -i
```

## API参考

### Command类

#### 构造函数
```cpp
Command(const String& name = String(), Logger* logger = new LoggerDefaultImpl())
```

#### 主要方法

- `name(const String& name)` - 设置命令名称
- `version(const String& v, const String& flag = "", const String& desc = "")` - 设置版本信息
- `description(const String& desc)` - 设置命令描述
- `help(const String& flag = "", const String& desc = "")` - 配置帮助选项
- `option(const String& flag, const String& desc = "", const Variant& defaultValue = Variant())` - 添加选项
- `argument(const String& name, const String& desc = "", const Variant& defaultValue = Variant())` - 添加参数
- `command(const String& nameAndArg, const String& desc = "")` - 添加子命令
- `action(const Action& cb)` - 设置命令执行的回调函数
- `parse(int argc, char** argv, int index = 1)` - 解析命令行参数

### 选项语法

支持多种选项格式：

```cpp
// 短选项和长选项
->option("-p --port <port>", "端口号")

// 仅长选项
->option("--host <host>", "主机地址")

// 布尔选项（无参数）
->option("-v --verbose", "详细输出")

// 多值选项
->option("-f --files <files...>", "文件列表")

// 可选参数
->option("-c --config [config]", "配置文件")
```

### 参数语法

```cpp
// 必需参数
->argument("<input>", "输入文件")

// 可选参数  
->argument("[output]", "输出文件")

// 多值参数
->argument("<files...>", "多个文件")
```

## 数据类型

库支持以下数据类型：

- `int` - 整数
- `double` - 浮点数  
- `String` - 字符串（std::string）
- `bool` - 布尔值
- `std::vector<VariantBase>` - 多值参数

## 示例

### 文件处理工具

```cpp
Command("filetool")
    ->version("1.0.0")
    ->description("文件处理工具")
    ->option("-r --recursive", "递归处理")
    ->option("-e --ext <extensions...>", "文件扩展名", Vector<VariantBase>{"txt", "md"})
    ->argument("<directory>", "目标目录")
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        String directory = std::get<String>(args[0]);
        bool recursive = opts.find("recursive") != opts.end();
        auto extensions = std::get<std::vector<VariantBase>>(opts["ext"]);
        
        // 文件处理逻辑
        std::cout << "处理目录: " << directory << std::endl;
    })
    ->parse(argc, argv);
```

### 子命令示例

```cpp
Command("git")
    ->version("2.30.0")
    ->command("clone <repository> [directory]", "克隆仓库")
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        // clone命令逻辑
    })
    ->command("commit -m <message>", "提交更改")
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        // commit命令逻辑  
    })
    ->parse(argc, argv);
```

## 测试

项目包含完整的测试用例，验证核心功能：

- 版本信息测试
- 帮助信息测试  
- 参数解析测试
- 选项处理测试

运行测试：
```bash
xmake run commander-cpp
```

## 依赖

- C++17 或更高版本
- 标准模板库（STL）

## 许可证

MIT License

## 贡献

欢迎提交Issue和Pull Request来改进这个项目。

## 致谢

本项目灵感来源于Node.js的[commander.js](https://github.com/tj/commander.js)库。