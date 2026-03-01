# SysY Compiler Project

## Project Overview

这是一个 SysY 语言编译器项目，使用 C++ 编写。SysY 是「编译原理」课程中常用的教学语言，是 C 语言的一个子集，支持基本的数据类型（int, float）、控制流语句（if, while, for, return）、数组、函数等特性。

本编译器目标平台为 RISC-V 64 位架构，能将 SysY 源代码编译为 RISC-V 汇编代码。

## Technology Stack

- **语言**: C++ (使用 Clang++ 编译)
- **目标架构**: RISC-V 64-bit
- **构建工具**: GNU Make
- **调试工具**: GDB, QEMU (用于 RISC-V 程序模拟)
- **开发环境**: VS Code (已配置好调试和构建任务)

## Project Structure

```
├── src/                    # 源代码目录
│   ├── main.cpp           # 程序入口，命令行参数解析
│   ├── tokenize.cpp       # 词法分析器实现
│   ├── tokenMember.cpp    # Token 类成员函数实现
│   ├── ast.cpp            # 语法分析器实现（递归下降）
│   ├── astMember.cpp      # AST 节点类成员函数实现
│   ├── codegen.cpp        # 代码生成器（RISC-V 汇编）
│   └── tools.cpp          # 工具函数（错误处理等）
├── inc/                    # 头文件目录
│   ├── common.hpp         # 通用头文件，包含基础定义和宏
│   ├── tokenize.hpp       # 词法分析相关定义
│   ├── ast.hpp            # AST 节点类定义
│   └── debug.hpp          # 调试宏定义开关
├── 2023test/              # 测试用例目录
│   ├── *.sy               # SysY 测试源文件
│   └── *.out              # 期望输出（程序返回码）
├── build/                 # 编译输出目录（自动生成）
├── tmp/                   # 临时文件目录（存放生成的汇编）
├── Makefile               # 构建配置
├── test.sh                # 测试脚本
└── ast.pu                 # AST 类图（PlantUML 格式）
```

## Architecture

### 1. 词法分析器 (Tokenizer)

文件: `src/tokenize.cpp`, `inc/tokenize.hpp`

- 将源代码字符流转换为 Token 序列
- 支持的 Token 类型:
  - `TK_NUM`: 数字常量
  - `TK_KEYWORD`: 关键字（return, int, float, const, if, else, for, while, break, continue）
  - `TK_IDENT`: 标识符
  - `TK_PUNCT`: 操作符和标点符号
  - `TK_EOF`: 文件结束标记
- 支持跳过行注释 `//` 和块注释 `/* */`

### 2. 语法分析器 (Parser)

文件: `src/ast.cpp`, `inc/ast.hpp`

- 使用递归下降法解析 Token 序列，生成抽象语法树 (AST)
- 当前实现的语法规则:

```
CompUnit    = FuncDef
FuncDef     = FuncType Ident '(' [FuncFParams] ')' Block
FuncType    = 'void' | 'int' | 'float'
Block       = '{' { BlockItem } '}'
BlockItem   = { Stmt }
Stmt        = 'return' [Expr] ';' | [ Expr ] ';'
```

- AST 节点类型:
  - `ASTNode`: 基础节点类
  - `ObjNode`: 对象节点（变量或函数）
  - `FuncNode`: 函数定义节点
  - `BlockNode`: 代码块节点
  - `UnaryNode`: 一元表达式节点
  - `NumNode`: 数字常量节点
  - `IFNode`: if 语句节点（待完善）
  - `ForNode`: for 循环节点（待完善）

### 3. 代码生成器 (Code Generator)

文件: `src/codegen.cpp`

- 遍历 AST，生成 RISC-V 汇编代码
- 当前支持:
  - 函数定义和返回语句
  - 整数常量返回值
- 生成的汇编使用 RISC-V 指令集（如 `li`, `ret` 等）

## Build Commands

### 编译项目

```bash
make
# 或
make compiler
```

### 清理构建文件

```bash
make clean
```

### 统计代码行数

```bash
make count
```

### 统计测试用例数量

```bash
make sycount
```

### 运行指定测试

```bash
# 默认运行 00_main.sy
make run

# 运行指定测试
make run tname=03_branch
```

## Testing Instructions

### 测试脚本使用方法

```bash
# 运行单个测试
./test.sh

# 手动测试流程
./compiler -S -o tmp/output.s 2023test/00_main.sy
$riscv64-unknown-linux-gnu-gcc -static -o build/test.out tmp/output.s
qemu-riscv64 -L $RISCV/sysroot ./build/test.out
echo $?  # 查看返回值
```

### 测试文件格式

- 测试源文件: `2023test/<name>.sy`
- 期望输出: `2023test/<name>.out`（包含程序期望的返回码）

### 环境变量要求

测试脚本依赖以下环境变量:
- `RISCV`: RISC-V 工具链安装路径

## Code Style Guidelines

### 命名约定

- **类名**: 使用 PascalCase（如 `TokenList`, `FuncNode`）
- **函数/方法名**: 使用 camelCase（如 `getVal()`, `addBody()`）
- **成员变量**: 使用首字母大写（如 `Name`, `Kind`, `IsFunc`）
- **私有成员**: 无特殊前缀，通过访问控制符区分

### 代码组织

- 头文件放在 `inc/` 目录，使用 `#ifndef` 保护
- 实现文件放在 `src/` 目录
- 类成员函数实现分散在对应的 `.cpp` 文件中

### 调试输出

项目提供了多种调试宏（在 `inc/debug.hpp` 中定义）:

```cpp
// 取消注释以启用对应调试输出
// #define __Debug_Token_List__    // 词法分析调试
// #define __Debug_TokenList__     // Token 列表调试
// #define __DEBUG_PARSE_STMT__    // 语句解析调试
// #define __DEBUG_BLOCKITEM__     // 块项解析调试
```

### 错误处理

使用自定义的错误处理函数:
- `error(const char *Fmt, ...)`: 打印错误信息并退出
- `Assert(cond, format, ...)`: 条件断言
- `panic(format, ...)`: 触发致命错误
- `TODO()`: 标记未实现的功能

## Development Workflow

### VS Code 配置

项目已配置好 VS Code 开发环境:
- `.vscode/tasks.json`: 构建任务配置
- `.vscode/launch.json`: 调试配置（使用 GDB）
- `.vscode/settings.json`: 编辑器设置和警告选项

### 添加新功能的一般步骤

1. 在 `inc/ast.hpp` 中定义新的 AST 节点类（如需要）
2. 在 `src/astMember.cpp` 中实现节点成员函数
3. 在 `src/ast.cpp` 中添加对应的语法解析规则
4. 在 `src/codegen.cpp` 中添加代码生成逻辑
5. 添加测试用例到 `2023test/` 目录
6. 运行测试验证功能正确性

## Current Limitations

当前编译器处于早期开发阶段，已实现功能:
- ✅ 基本词法分析（数字、标识符、关键字、注释）
- ✅ 简单函数定义和返回语句
- ✅ 基本代码生成（RISC-V 汇编）

待实现功能:
- ❌ 表达式解析（加减乘除等运算符）
- ❌ 变量定义和赋值
- ❌ 控制流语句（if, while, for）
- ❌ 函数参数传递
- ❌ 数组支持
- ❌ 完整的类型系统

## Security Considerations

- 编译器假定输入源文件来自可信来源
- 未对缓冲区溢出进行严格检查（建议使用安全的文件读取函数）
- 内存管理使用原始指针，需要注意内存泄漏问题

## Dependencies

- Clang++ (或 G++)
- RISC-V GNU Toolchain (`riscv64-unknown-linux-gnu-gcc`)
- QEMU RISC-V (`qemu-riscv64`)

## References

- SysY 语言规范（参考 2023 年编译系统设计赛）
- RISC-V Instruction Set Manual
