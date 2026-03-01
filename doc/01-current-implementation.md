# 当前项目实现详情

## 一、项目概述

本项目是一个面向 SysY 教学语言的编译器，目标平台为 RISC-V 64 位架构。编译器采用经典的三段式结构（词法分析 → 语法分析 → 代码生成），目前处于早期开发阶段。

---

## 二、已实现功能详解

### 2.1 词法分析器 (Tokenizer)

**文件位置**: `src/tokenize.cpp`, `inc/tokenize.hpp`

#### 功能特性

| 功能 | 实现状态 | 说明 |
|------|----------|------|
| 数字常量解析 | ✅ | 支持十进制整数，`strtoul` 解析 |
| 标识符解析 | ✅ | 支持字母/下划线开头，后跟字母数字下划线 |
| 关键字识别 | ✅ | return, int, float, const, if, else, for, while, break, continue |
| 操作符识别 | ✅ | 基础标点符号（单字符） |
| 行注释跳过 | ✅ | `//` 风格注释 |
| 块注释跳过 | ✅ | `/* */` 风格注释 |
| 空白字符处理 | ✅ | 自动跳过空格、制表符、换行 |

#### 类设计

```
Token (基类)
├── DigitalTok (数字Token，带整数值)
└── 其他Token类型通过 Kind 字段区分

TokenList (Token链表管理器)
├── head/tail 指针
├── addNode() - 添加Token
├── fetchName() - 提取字符串
└── print() - 调试输出
```

#### Token 类型枚举

```cpp
typedef enum{
    TK_NUM,      // 数字常量
    TK_KEYWORD,  // 关键字
    TK_IDENT,    // 标识符
    TK_PUNCT,    // 操作符/标点
    TK_EOF,      // 文件结束
}TokenKind;
```

---

### 2.2 语法分析器 (Parser)

**文件位置**: `src/ast.cpp`, `src/astMember.cpp`, `inc/ast.hpp`

#### 当前支持的语法规则

```ebnf
CompUnit    = FuncDef
FuncDef     = FuncType Ident '(' [FuncFParams] ')' Block
FuncType    = 'void' | 'int' | 'float'
Block       = '{' { BlockItem } '}'
BlockItem   = { Stmt }
Stmt        = 'return' [Num] ';' | ';'
```

**注意**: 表达式解析（加减乘除等）已设计但未完全启用，代码被注释。

#### AST 节点类层次

```
ASTNode (基类)
├── ObjNode (对象节点：变量/函数)
│   ├── FuncNode (函数定义)
│   └── VarNode (变量定义，待完善)
├── BlockNode (代码块)
├── UnaryNode (一元表达式/语句)
├── NumNode (数字常量)
├── IFNode (if语句，定义中)
└── ForNode (for循环，定义中)
```

#### 节点类型枚举

```cpp
typedef enum{
    ND_RETURN,     // return语句
    ND_FUN,        // 函数定义
    ND_BLOCK,      // 代码块
    ND_NUM,        // 数字
    ND_ADD,        // 加法（预留）
    ND_SUB,        // 减法（预留）
    ND_MUL,        // 乘法（预留）
    ND_DIV,        // 除法（预留）
    ND_MOD,        // 取模（预留）
    ND_EXPR_STMT,  // 表达式语句（预留）
}NodeKind;
```

#### 关键实现细节

1. **递归下降解析**: 每个非终结符对应一个解析函数
2. **Token 流管理**: 通过引用传递 `Token *&Tok` 实现前进
3. **错误处理**: `skip()` 函数检查预期 Token，不匹配则报错

---

### 2.3 代码生成器 (Code Generator)

**文件位置**: `src/codegen.cpp`

#### 当前能力

| 功能 | 实现状态 | 生成代码示例 |
|------|----------|--------------|
| 函数定义头 | ✅ | `.globl main`, `.text`, `main:` |
| 整数返回 | ✅ | `li a0, <value>` |
| 函数返回指令 | ✅ | `ret` |

#### 生成流程

```cpp
void codegen(ObjNode *Obj, FILE* Out)
├── 输出函数头 (.globl, .text, label)
├── 遍历函数体语句
│   └── genStmt() - 处理每条语句
│       └── ND_RETURN: 生成 li a0, val
└── 输出 ret 指令
```

---

### 2.4 工具与基础设施

**文件位置**: `src/tools.cpp`, `inc/common.hpp`, `inc/debug.hpp`

#### 错误处理机制

| 函数/宏 | 用途 |
|---------|------|
| `error(fmt, ...)` | 打印错误并退出程序 |
| `Assert(cond, fmt, ...)` | 条件断言，失败报错 |
| `panic(fmt, ...)` | 致命错误 |
| `TODO()` | 标记未实现功能 |

#### 调试支持

通过 `inc/debug.hpp` 中的宏控制调试输出：

```cpp
// #define __Debug_Token_List__    // 词法分析调试
// #define __Debug_TokenList__     // Token列表调试
// #define __DEBUG_PARSE_STMT__    // 语句解析调试
// #define __DEBUG_BLOCKITEM__     // 块项解析调试
```

---

## 三、代码统计

```
行数统计:
├── src/main.cpp          ~84 行
├── src/tokenize.cpp      ~156 行
├── src/tokenMember.cpp   ~40 行
├── src/ast.cpp           ~165 行
├── src/astMember.cpp     ~65 行
├── src/codegen.cpp       ~46 行
├── src/tools.cpp         ~12 行
├── inc/common.hpp        ~76 行
├── inc/tokenize.hpp      ~71 行
├── inc/ast.hpp           ~182 行
└── inc/debug.hpp         ~7 行

总计: 约 900 行代码（含注释和空行）
```

---

## 四、当前限制与已知问题

### 4.1 功能限制

| 功能 | 状态 | 影响 |
|------|------|------|
| 表达式解析 | ❌ 未实现 | 无法处理 `return 1+2` |
| 变量定义 | ❌ 未实现 | 无法使用局部变量 |
| 变量赋值 | ❌ 未实现 | 无法修改变量值 |
| 函数参数 | ❌ 未实现 | 只能处理无参函数 |
| 控制流(if/while/for) | ❌ 未实现 | 无法使用分支和循环 |
| 数组 | ❌ 未实现 | 无法使用数组类型 |
| 常量定义 | ❌ 未实现 | const 关键字仅识别 |
| 类型系统 | ⚠️ 部分 | 仅识别 int/void/float，无类型检查 |

### 4.2 代码质量问题

1. **内存管理**: 使用原始指针，存在内存泄漏风险
2. **错误恢复**: 遇到错误直接退出，无恢复机制
3. **代码组织**: AST 节点类成员函数分散在多个文件
4. **测试覆盖**: 仅能通过 `00_main.sy` 测试

### 4.3 设计问题

1. **无中间表示(IR)**: 前端和后端直接耦合
2. **硬编码假设**: 如 `Stmt` 中假设 return 后紧跟数字
3. **缺乏符号表**: 变量/函数信息无统一管理

---

## 五、可成功编译的示例

### 5.1 最简单的程序

**输入** (`00_main.sy`):
```c
int main(){
    return 2;
}
```

**生成的汇编** (`tmp/00_main.S`):
```asm
 .globl main
 .text
main:
 li a0,  0    ; 注意：当前实现有bug，未正确解析数字
 ret
```

**说明**: 实际上当前解析有bug——`Stmt` 中创建 `NumNode` 时传入的是 `return` Token 而非数字 Token。

### 5.2 支持空语句

```c
int main(){
    ;;;;;;;
    return 0;
}
```

空语句 `;` 会被正确解析并跳过。

---

## 六、构建与测试

### 6.1 构建命令

```bash
# 编译项目
make

# 清理构建文件
make clean

# 统计代码行数
make count

# 统计测试用例数
make sycount
```

### 6.2 运行测试

```bash
# 运行单个测试（默认 00_main.sy）
make run

# 运行指定测试
make run tname=00_main

# 使用测试脚本
./test.sh
```

### 6.3 手动测试流程

```bash
# 1. 编译 SysY 源文件
./compiler -S -o tmp/output.s 2023test/00_main.sy

# 2. 用 RISC-V 工具链汇编
riscv64-unknown-linux-gnu-gcc -static -o build/test.out tmp/output.s

# 3. 用 QEMU 运行
qemu-riscv64 -L $RISCV/sysroot ./build/test.out

# 4. 查看返回值
echo $?
```

---

## 七、总结

当前项目完成了编译器最基础的基础设施搭建：

- ✅ **词法分析**: 功能相对完整，能正确处理注释、关键字、标识符
- ⚠️ **语法分析**: 框架已搭建，但仅能解析最简单的函数定义和 return 语句
- ⚠️ **代码生成**: 仅能生成最简单的函数框架

**下一阶段重点**:
1. 修复现有 bug（如 `Stmt` 中的 Token 传递问题）
2. 实现表达式解析（启用已注释的代码）
3. 引入中间表示(IR)解耦前后端
4. 实现符号表管理变量作用域
