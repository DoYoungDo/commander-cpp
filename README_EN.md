# Commander-CPP

> English | [中文](README.md)

Commander-CPP is a C++ command-line parsing library inspired by [commander.js](https://github.com/tj/commander.js). It provides a clean, chainable API for defining commands, options, and arguments, with support for subcommands, default values, multi-value options, and more.

## Features

- 🎯 Chainable API design, simple and easy to use
- 📦 Single header file, no external dependencies
- 🔧 Support for options (single-value, multi-value, boolean)
- 📝 Support for arguments (required arguments, optional arguments, multi-value arguments)
- 🌲 Support for subcommands and nested commands
- ⚙️ Support for default values
- 📖 Automatic help generation
- 🔍 Detailed error handling and logging system
- 🎨 Support for option aliases and combinations (like `-abc`)

## Installation

Copy the `commander_cpp.hpp` file to your project and include it in your code:

```cpp
#include "commander_cpp.hpp"
```

## Quick Start

```cpp
#include "commander_cpp.hpp"
using namespace COMMANDER_CPP;

int main(int argc, char **argv) {
    Command("example")
        .version("1.0.0")
        ->description("An example command-line application")
        ->option("-n --name <name>", "Your name")
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

Usage example:

```bash
$ ./example -n Alice
Hello, Alice!

$ ./example --help
Usage: example [options]

An example command-line application

Options:
  -V, --version       out put version number.
  -n, --name <name>   Your name
  -h, --help
```

## Core Concepts

### Option Syntax

Options use `-` or `--` prefixes and support the following formats:

- `--option` - Boolean option
- `-o --option` - Boolean option with alias
- `--option <value>` - Option with required value
- `--option [value]` - Option with optional value
- `--option <values...>` - Multi-value option (at least one)
- `--option [values...]` - Multi-value option (optional)

### Argument Syntax

Arguments don't use prefixes and support the following formats:

- `<arg>` - Required argument
- `[arg]` - Optional argument
- `<args...>` - Multi-value required arguments
- `[args...]` - Multi-value optional arguments

## Detailed Usage

### 1. Defining Options

```cpp
Command("app")
    .option("-s --single <value>", "Single-value option")
    ->option("-m --multi <values...>", "Multi-value option")
    ->option("-b --boolean", "Boolean option")
    ->option("-n --number <num>", "Number option", 42)  // With default value
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        // Get single-value option
        if (opts.find("single") != opts.end()) {
            auto value = std::get<String>(opts["single"]);
            std::cout << "Single: " << value << std::endl;
        }
        
        // Get multi-value option
        if (opts.find("multi") != opts.end()) {
            auto values = std::get<std::vector<VariantBase>>(opts["multi"]);
            for (const auto &val : values) {
                std::cout << std::get<String>(val) << " ";
            }
            std::cout << std::endl;
        }
        
        // Get boolean option
        if (opts.find("boolean") != opts.end()) {
            std::cout << "Boolean enabled" << std::endl;
        }
    });
```

Usage examples:

```bash
$ ./app -s value1
$ ./app --single value1
$ ./app -m file1.txt file2.txt file3.txt
$ ./app -b
$ ./app -bs value1  # Combined options
```

### 2. Defining Arguments

```cpp
Command("copy")
    .argument("<from>", "Source file")
    ->argument("[to]", "Target file", "default.txt")  // With default value
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        auto from = std::get<String>(args[0]);
        std::cout << "From: " << from << std::endl;
        
        if (args.size() > 1) {
            auto to = std::get<String>(args[1]);
            std::cout << "To: " << to << std::endl;
        }
    });
```

Usage examples:

```bash
$ ./copy source.txt
$ ./copy source.txt target.txt
```

### 3. Defining Subcommands

```cpp
Command("git")
    .version("1.0.0")
    ->description("Git command-line tool")
    ->command("add <files...>", "Add files to staging area")
    ->option("-f --force", "Force add")
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        auto files = std::get<std::vector<VariantBase>>(args[0]);
        std::cout << "Adding files..." << std::endl;
        for (const auto &file : files) {
            std::cout << "  " << std::get<String>(file) << std::endl;
        }
    });
    // Add another subcommand
    ->command("commit", "Commit changes")
    ->option("-m --message <msg>", "Commit message")
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        if (opts.find("message") != opts.end()) {
            auto msg = std::get<String>(opts["message"]);
            std::cout << "Commit: " << msg << std::endl;
        }
    });
```

Usage examples:

```bash
$ ./git add file1.txt file2.txt
$ ./git add -f file.txt
$ ./git commit -m "Initial commit"
```

### 4. Type Support

Commander-CPP automatically recognizes and converts the following types:

```cpp
Command("types")
    .option("-i --int <num>", "Integer")
    ->option("-d --double <num>", "Double")
    ->option("-b --bool <val>", "Boolean")
    ->option("-s --string <text>", "String")
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        // Automatic type conversion
        auto intVal = std::get<int>(opts["int"]);        // 42
        auto doubleVal = std::get<double>(opts["double"]); // 3.14
        auto boolVal = std::get<bool>(opts["bool"]);     // true/false
        auto strVal = std::get<String>(opts["string"]);  // "text"
    });
```

Usage examples:

```bash
$ ./types -i 42 -d 3.14 -b true -s hello
```

### 5. Custom Logging

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
        .option("-v --verbose", "Verbose output")
        ->parse(argc, argv);
    return 0;
}
```

### 6. Option Combinations

Support for short option combinations (like `tar -xzvf`):

```cpp
Command("app")
    .option("-a --option-a", "Option A")
    ->option("-b --option-b", "Option B")
    ->option("-c --option-c <value>", "Option C")
    ->action([](Vector<Variant> args, Map<String, Variant> opts) {
        if (opts.find("option-a") != opts.end()) {
            std::cout << "A enabled" << std::endl;
        }
        if (opts.find("option-b") != opts.end()) {
            std::cout << "B enabled" << std::endl;
        }
    });
```

Usage examples:

```bash
$ ./app -ab           # Enable both A and B
$ ./app -abc value    # Enable A, B, and set value for C
```

## Complete Example

Based on the integration test in `main.cpp`, here's a complete todo application example:

```cpp
#include "commander_cpp.hpp"
using namespace COMMANDER_CPP;

int main(int argc, char **argv) {
    Command("todo")
        .version("1.0.0")
        ->description("Todo management tool")
        
        // Add subcommand
        ->command("add <todos...>", "Add todo items")
        ->option("-d --done", "Mark as completed")
        ->option("-p --priority", "Set as high priority")
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
    
    // Remove subcommand
    Command("todo")
        .command("rm", "Remove todo items")
        ->argument("<index...>", "Todo item indices")
        ->option("-l --level <value>", "Removal level")
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

Usage examples:

```bash
$ ./todo add "Learn C++" "Write code" -p
Added: Learn C++ [HIGH]
Added: Write code [HIGH]

$ ./todo rm 1 2
Removed: 1
Removed: 2

$ ./todo --help
Usage: todo [options]

Todo management tool

Options:
  -V, --version       out put version number.
  -h, --help

Commands:
  add [options] <todos...>  Add todo items
  rm [options] <index...>   Remove todo items
```

## Testing

The project includes a comprehensive test suite (see `src/main.cpp`) covering the following scenarios:

| Test Case | Description |
|-----------|-------------|
| VersionTest | Verify version information output |
| DescriptionTest | Verify help information generation |
| OptionTest | Test single-value and multi-value option parsing |
| ArgumentTest | Test required and optional arguments |
| SubCommandTest | Test subcommand functionality |
| DefaultValueTest | Test default value mechanism |
| MultiValueOptionTest | Test multi-value option parsing |
| ErrorHandlingTest | Test error handling |
| ComplexOptionTest | Test option combinations |
| IntegratedTest | Integration test |

Run tests:

```bash
$ xmake
$ ./build/macosx/arm64/debug/commander-cpp
$ ./build/macosx/arm64/debug/commander-cpp -i  # Show detailed information
```

## Build

The project uses [xmake](https://xmake.io/) for building:

```bash
# Install xmake
$ brew install xmake

# Build project
$ xmake

# Run tests
$ xmake run commander-cpp
```

## Directory Structure

```
commander-cpp/
├── src/
│   ├── commander_cpp.hpp   # Core library (single header file)
│   └── main.cpp            # Test cases
├── build/                  # Build output
├── xmake.lua              # Build configuration
└── README.md              # This document
```

## Requirements

- Supports C++17 or higher

## Future Plans

- [ ] Support for configuration file reading

## Contributing

Issues and Pull Requests are welcome!

## License

MIT License
