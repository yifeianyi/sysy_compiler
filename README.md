# SysY 编译器

一个基于 C++ 实现的 SysY 语言编译器，目标平台为 RISC-V 64 位架构。

> **SysY** 是「编译原理」课程中常用的教学语言，是 C 语言的子集，支持基本数据类型（int, float）、控制流语句（if, while）、数组、函数等特性。

---

## 📊 项目状态

**当前阶段**: 阶段0 热身与修复 ✅ (4/4 测试通过)

| 阶段 | 进度 | 说明 |
|------|------|------|
| 阶段0: 热身与修复 | ✅ 100% | 词法/语法分析基础框架，return 常量 |
| 阶段1: 表达式 | ⬜ 0% | 四则运算、优先级、一元运算 (22个测试) |
| 阶段2: 变量与作用域 | ⬜ 0% | 变量定义、栈帧管理、作用域 (13个测试) |
| 阶段3: 控制流 | ⬜ 0% | if-else、while、break/continue (15个测试) |
| 阶段4: 函数调用 | ⬜ 0% | 函数定义、调用、ABI (5个测试) |
| 阶段5: IR引入 | ⬜ 0% | 复杂度危机、三地址码 (4个测试) |
| 阶段6: 复杂算法 | ⬜ 0% | 排序、图算法、字符串 (33个测试) |
| 阶段7: 高级特性 | ⬜ 0% | 数组、浮点、压力测试 (41个测试) |

**总体进度**: 4/137 (2.9%)

---

## 🚀 快速开始

### 环境要求

- **编译器**: Clang++ 14.0+ (或 G++ 11.0+)
- **目标平台**: RISC-V 64-bit
- **运行环境**: QEMU RISC-V (用于测试)
- **构建工具**: GNU Make

### 构建项目

```bash
# 克隆项目
git clone <repository-url>
cd sysy_compiler

# 编译
make

# 查看编译结果
./compiler --help
```

### 编译单个文件

```bash
# 编译 SysY 源文件到 RISC-V 汇编
./compiler -S -o output.s 2023test/stage0_warmup/00_main.sy

# 使用 RISC-V 工具链编译汇编
riscv64-unknown-linux-gnu-gcc -static -o program output.s

# 使用 QEMU 运行
qemu-riscv64 -L $RISCV/sysroot ./program
echo $?  # 查看返回值
```

### 运行测试

```bash
# 运行单个测试
make run tname=00_main

# 或使用测试脚本
./test.sh  # 运行所有测试
```

---

## 📁 项目结构

```
sysy_compiler/
├── src/                    # 源代码
│   ├── main.cpp           # 程序入口，命令行参数解析
│   ├── tokenize.cpp       # 词法分析器
│   ├── tokenMember.cpp    # Token 类成员函数
│   ├── ast.cpp            # 语法分析器（递归下降）
│   ├── astMember.cpp      # AST 节点类成员函数
│   ├── codegen.cpp        # 代码生成器（RISC-V 汇编）
│   └── tools.cpp          # 工具函数（错误处理等）
│
├── inc/                    # 头文件
│   ├── common.hpp         # 通用定义和宏
│   ├── tokenize.hpp       # 词法分析相关
│   ├── ast.hpp            # AST 节点类定义
│   └── debug.hpp          # 调试宏
│
├── 2023test/               # 测试用例（按阶段分类）
│   ├── stage0_warmup/     # 阶段0: 热身与修复 (4个)
│   ├── stage1_expression/ # 阶段1: 表达式 (22个)
│   ├── stage2_variable/   # 阶段2: 变量与作用域 (13个)
│   ├── stage3_controlflow/# 阶段3: 控制流 (15个)
│   ├── stage4_function/   # 阶段4: 函数调用 (5个)
│   ├── stage5_ir/         # 阶段5: IR引入 (4个)
│   ├── stage6_optimization/# 阶段6: 复杂算法 (33个)
│   └── stage7_advanced/   # 阶段7: 高级特性 (41个)
│
├── doc/                    # 文档
│   └── 04-learning-roadmap.md  # 学习导向开发计划
│
├── build/                  # 编译输出目录（自动生成）
├── tmp/                    # 临时文件目录（生成的汇编）
├── Makefile               # 构建配置
├── test.sh                # 测试脚本
├── TODO.md                # 开发 TODO 列表
└── README.md              # 本文件
```

---

## 🎯 功能特性

### 已实现 ✅

- **词法分析**
  - 数字常量、标识符、关键字识别
  - 支持 `//` 行注释和 `/* */` 块注释
  - 关键字: `return`, `int`, `void`, `float`, `const`, `if`, `else`, `for`, `while`, `break`, `continue`

- **语法分析**
  - 函数定义解析
  - 代码块和语句解析
  - return 语句、空语句

- **代码生成**
  - RISC-V 64 位汇编输出
  - 函数入口/出口处理
  - return 常量指令生成

### 待实现 ⬜

- 表达式（四则运算、优先级、一元运算）
- 变量定义与访问
- 控制流（if-else、while、break/continue）
- 函数调用与参数传递
- IR（三地址码）设计与实现
- 数组支持（多维数组）
- 浮点数支持
- 寄存器分配算法

---

## 📚 开发文档

- **[学习路线图](doc/04-learning-roadmap.md)** - 详细的 8 阶段学习开发计划
- **[TODO 列表](TODO.md)** - 按测试文件追踪的开发任务
- **[AGENTS.md](AGENTS.md)** - 项目背景和技术细节

### 学习理念

本项目采用**学习导向**的开发方式：

1. **阶段 0-4**: AST 直接翻译（感受复杂度）
   - 手工寄存器分配 → 痛苦
   - 手工栈帧管理 → 痛苦
   - 手工控制流 → 痛苦
   - 💡 自然理解：为什么需要 IR

2. **阶段 5-7**: 引入 IR（豁然开朗）
   - 虚拟寄存器 → 不再担心溢出
   - 符号表 → 统一管理变量
   - 优化器 → 模式匹配即可
   - 💡 深刻理解：工业编译器都用 IR 的原因

---

## 🛠️ 开发指南

### 添加新功能的一般步骤

1. 在 `inc/ast.hpp` 中定义新的 AST 节点类（如需要）
2. 在 `src/astMember.cpp` 中实现节点成员函数
3. 在 `src/ast.cpp` 中添加对应的语法解析规则
4. 在 `src/codegen.cpp` 中添加代码生成逻辑
5. 添加测试用例到 `2023test/` 对应阶段目录
6. 运行测试验证功能正确性

### 调试技巧

项目提供了多种调试宏（在 `inc/debug.hpp` 中定义）：

```cpp
// 取消注释以启用对应调试输出
#define __Debug_Token_List__    // 词法分析调试
// #define __Debug_TokenList__     // Token 列表调试
// #define __DEBUG_PARSE_STMT__    // 语句解析调试
// #define __DEBUG_BLOCKITEM__     // 块项解析调试
```

---

## 🧪 测试集

测试文件按阶段组织在 `2023test/` 目录下：

```bash
# 查看各阶段测试数量
make sycount  # 输出: 137

# 阶段0测试（当前已通过）
ls 2023test/stage0_warmup/
# 00_main.sy 00_comment2.sy 42_empty_stmt.sy 45_comment1.sy
```

每个测试文件包含：
- `.sy` - SysY 源文件
- `.out` - 期望的程序返回值
- `.in` - 输入数据（部分测试需要）

---

## 📖 参考资料

- **[SysY 语言规范](doc/)** - 2023年编译系统设计赛标准
- **[RISC-V 指令集手册](https://riscv.org/technical/specifications/)**
- **《编译原理》(龙书)** - Aho, Sethi, Ullman
- **[chibicc](https://github.com/rui314/chibicc)** - 参考实现
- **[Compiler Explorer](https://godbolt.org/)** - 在线对比编译器输出

---

## 🤝 贡献

欢迎提交 Issue 和 PR！

请确保：
1. 代码遵循项目的命名约定
2. 添加必要的注释
3. 通过相关测试
4. 更新文档（如适用）

---

## 📝 License

[MIT License](LICENSE)

---

## 🌟 致谢

本项目参考了以下优秀开源项目：
- [chibicc](https://github.com/rui314/chibicc) - Rui Ueyama 的 C 编译器
- [rvcc](https://github.com/andoShin/rvcc) - RISC-V C 编译器

感谢「编译原理」课程和 SysY 语言规范的设计者！

---

**祝你编译愉快！** 🚀
