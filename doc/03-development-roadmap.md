# 后续开发规划

## 一、总体架构演进方向

```
当前架构                    目标架构
┌─────────┐                ┌─────────┐
│  Source │                │  Source │
└────┬────┘                └────┬────┘
     │                          │
     ▼                          ▼
┌─────────┐                ┌─────────┐     ┌─────────┐
│ Tokenizer│               │ Tokenizer│────▶│ Token   │◀──── 单元测试
└────┬────┘                └────┬────┘     │  Test   │
     │                          │          └─────────┘
     ▼                          ▼
┌─────────┐                ┌─────────┐     ┌─────────┐
│  Parser │                │  Parser │────▶│ Parser  │◀──── 单元测试
└────┬────┘                └────┬────┘     │  Test   │
     │                          │          └─────────┘
     ▼                          ▼
┌─────────┐                ┌─────────┐
│   AST   │                │   AST   │
└────┬────┘                └────┬────┘
     │                          │
     ▼                          ▼
┌─────────┐                ┌─────────┐     ┌─────────┐
│ Codegen │                │ IR Gen  │────▶│  IR     │◀──── 独立测试
└─────────┘                └────┬────┘     │  Test   │
                                │          └─────────┘
                                ▼
                          ┌─────────┐
                          │ Optimizer│
                          └────┬────┘
                               │
                          ┌────┴────┐
                          ▼         ▼
                    ┌─────────┐ ┌─────────┐
                    │ RISC-V  │ │  x86    │◀──── 后端独立测试
                    │ Backend │ │ Backend │
                    └─────────┘ └─────────┘
```

---

## 二、前端独立开发与测试

### 2.1 前端组成

前端包含三个阶段：
1. **词法分析 (Tokenizer/Lexer)**
2. **语法分析 (Parser)**
3. **语义分析 (Semantic Analyzer)**

### 2.2 词法分析器独立测试

#### 测试策略：黑盒测试 + 快照测试

```cpp
// test/lexer_test.cpp
#include <gtest/gtest.h>
#include "tokenize.hpp"

class LexerTest : public ::testing::Test {
protected:
    std::vector<Token> tokenize(const std::string& source) {
        writeFile("tmp_test.sy", source);
        auto list = tokenizeFile("tmp_test.sy");
        return list->toVector();
    }
};

// 基本 Token 测试
TEST_F(LexerTest, Keywords) {
    auto tokens = tokenize("int void return if else while");
    
    EXPECT_EQ(tokens[0].kind, TK_KEYWORD); EXPECT_EQ(tokens[0].text, "int");
    EXPECT_EQ(tokens[1].kind, TK_KEYWORD); EXPECT_EQ(tokens[1].text, "void");
    EXPECT_EQ(tokens[2].kind, TK_KEYWORD); EXPECT_EQ(tokens[2].text, "return");
    // ...
}

TEST_F(LexerTest, Numbers) {
    auto tokens = tokenize("42 0 1234567890");
    
    EXPECT_EQ(tokens[0].kind, TK_NUM);
    EXPECT_EQ(tokens[0].getVal(), 42);
}

TEST_F(LexerTest, Comments) {
    auto tokens = tokenize(R"(
        // line comment
        /* block
           comment */
        42
    )");
    
    // 应该只有数字和一个 EOF
    EXPECT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].kind, TK_NUM);
}

TEST_F(LexerTest, ErrorRecovery) {
    // 测试非法字符处理
    auto tokens = tokenize("int @ invalid");
    
    // 应该报告错误但能继续解析
    EXPECT_TRUE(hasError());
    EXPECT_EQ(tokens[0].kind, TK_KEYWORD);
}
```

#### 自动化测试脚本

```bash
#!/bin/bash
# test/lexer_tests/run_tests.sh

for test_file in test/lexer_tests/*.sy; do
    expected="${test_file%.sy}.tokens"
    actual="tmp/${test_file##*/}.tokens"
    
    # 运行 lexer，输出 token 序列
    ./compiler --dump-tokens "$test_file" > "$actual"
    
    if diff "$expected" "$actual"; then
        echo "✓ PASS: $test_file"
    else
        echo "✗ FAIL: $test_file"
    fi
done
```

### 2.3 语法分析器独立测试

#### 测试策略：AST 结构验证

```cpp
// test/parser_test.cpp
#include "ast.hpp"
#include "parser.hpp"

class ParserTest : public ::testing::Test {
protected:
    std::unique_ptr<ASTNode> parse(const std::string& source) {
        auto tokens = lex(source);
        return Parser(tokens).parse();
    }
};

// 测试函数定义解析
TEST_F(ParserTest, FunctionDefinition) {
    auto ast = parse(R"(
        int main() {
            return 0;
        }
    )");
    
    auto* func = dynamic_cast<FuncNode*>(ast.get());
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->name, "main");
    EXPECT_EQ(func->params.size(), 0);
}

// 测试表达式解析
TEST_F(ParserTest, BinaryExpression) {
    auto ast = parseExpr("1 + 2 * 3");
    
    // 验证 AST 结构: (+ 1 (* 2 3))
    auto* bin = dynamic_cast<BinaryNode*>(ast.get());
    ASSERT_NE(bin, nullptr);
    EXPECT_EQ(bin->op, BinaryOp::ADD);
    
    auto* right = dynamic_cast<BinaryNode*>(bin->rhs);
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->op, BinaryOp::MUL);
}

// 测试错误恢复
TEST_F(ParserTest, ErrorRecovery_MissingSemicolon) {
    auto ast = parse(R"(
        int main() {
            return 0  // missing ;
            return 1;
        }
    )");
    
    // 应该报告错误但继续解析
    EXPECT_TRUE(hasError());
    ASSERT_NE(ast, nullptr);  // 仍然返回部分 AST
}

// AST 打印测试（用于调试）
TEST_F(ParserTest, ASTPrint) {
    auto ast = parse("int main() { return 1 + 2; }");
    
    std::ostringstream oss;
    ASTPrinter(oss).print(ast.get());
    
    EXPECT_EQ(oss.str(), R"(
Function: main
  Return:
    Binary: ADD
      Number: 1
      Number: 2
)");
}
```

#### 使用 AST 快照测试

```python
# test/parser_tests/update_snapshots.py
import subprocess
import os

for test_file in os.listdir("test/parser_tests"):
    if not test_file.endswith(".sy"):
        continue
    
    source = f"test/parser_tests/{test_file}"
    snapshot = f"test/parser_tests/{test_file}.ast"
    
    # 生成 AST 快照
    result = subprocess.run(
        ["./compiler", "--dump-ast", source],
        capture_output=True, text=True
    )
    
    with open(snapshot, "w") as f:
        f.write(result.stdout)
```

### 2.4 语义分析器独立测试

```cpp
// test/sema_test.cpp
class SemanticTest : public ::testing::Test {
protected:
    bool check(const std::string& source) {
        auto ast = parse(source);
        SemanticAnalyzer sema;
        return sema.analyze(ast.get());
    }
};

TEST_F(SemanticTest, UndefinedVariable) {
    EXPECT_FALSE(check(R"(
        int main() {
            return x;  // x not defined
        }
    )"));
    expectError("undefined variable 'x'");
}

TEST_F(SemanticTest, TypeMismatch) {
    EXPECT_FALSE(check(R"(
        int main() {
            int x = 3.14;  // float to int
            return x;
        }
    )"));
    expectError("type mismatch");
}

TEST_F(SemanticTest, ScopeChecking) {
    EXPECT_TRUE(check(R"(
        int main() {
            int x = 1;
            {
                int x = 2;  // shadowing allowed
            }
            return x;  // returns 1
        }
    )"));
}
```

---

## 三、IR 层设计与独立测试

### 3.1 为什么需要 IR

| 问题 | 解决方案 |
|------|----------|
| 前后端耦合 | IR 作为标准接口 |
| 难以优化 | 在 IR 层做机器无关优化 |
| 难以测试 | IR 可读、可验证、可解释执行 |
| 多后端支持 | 同一份 IR 生成不同目标代码 |

### 3.2 IR 设计建议

#### 3.2.1 选择 IR 类型

**推荐：自定义简化版 LLVM IR（三地址码 + SSA）**

理由：
- 结构清晰，易于生成和解析
- 天然支持 SSA，便于优化
- 与 LLVM 类似，学习成本低
- 足够表达 SysY 全部特性

#### 3.2.2 IR 数据结构

```cpp
// ir/ir.hpp
namespace ir {

// 值类型
class Value {
public:
    virtual ~Value() = default;
    virtual std::string toString() const = 0;
    Type* type;
};

// 常量
class ConstantInt : public Value {
public:
    int value;
    std::string toString() const override {
        return std::to_string(value);
    }
};

// 虚拟寄存器（SSA）
class VirtualReg : public Value {
public:
    int id;
    std::string toString() const override {
        return "%" + std::to_string(id);
    }
};

// 指令操作码
enum class OpCode {
    // 算术运算
    ADD, SUB, MUL, SDIV, SREM,
    FADD, FSUB, FMUL, FDIV,
    
    // 比较
    ICMP, FCMP,
    
    // 内存操作
    ALLOCA, LOAD, STORE,
    GEP,  // GetElementPtr（数组访问）
    
    // 控制流
    CALL, RET, BR, COND_BR,
    PHI,
    
    // 类型转换
    SITOF, FPTOSI,  // int <-> float
};

// 指令
class Instruction : public Value {
public:
    OpCode opcode;
    BasicBlock* parent;
    std::vector<Value*> operands;
    std::string name;  // 结果名（如 %1）
    
    std::string toString() const override;
};

// 基本块
class BasicBlock : public Value {
public:
    std::string label;
    std::list<Instruction*> instructions;
    std::vector<BasicBlock*> predecessors;
    std::vector<BasicBlock*> successors;
    
    void addInst(Instruction* inst);
    void addPred(BasicBlock* pred);
    void addSucc(BasicBlock* succ);
};

// 函数
class Function : public Value {
public:
    std::string name;
    Type* retType;
    std::vector<Argument*> args;
    std::list<BasicBlock*> blocks;
    
    BasicBlock* entryBlock() { return blocks.front(); }
};

// 模块（编译单元）
class Module {
public:
    std::string name;
    std::list<Function*> functions;
    std::list<GlobalVar*> globals;
};

} // namespace ir
```

#### 3.2.3 IR 文本格式

```llvm
; SysY IR 示例

; 全局变量
@global_array = global [10 x i32] zeroinitializer

; 函数定义
define i32 @main() {
entry:
    %1 = alloca i32, align 4        ; int a
    %2 = alloca i32, align 4        ; int b
    store i32 10, i32* %1           ; a = 10
    store i32 20, i32* %2           ; b = 20
    %3 = load i32, i32* %1          ; load a
    %4 = load i32, i32* %2          ; load b
    %5 = add i32 %3, %4             ; a + b
    ret i32 %5
}

; 带控制流的函数
define i32 @max(i32 %a, i32 %b) {
entry:
    %1 = icmp sgt i32 %a, %b        ; a > b
    br i1 %1, label %if_true, label %if_false

if_true:
    ret i32 %a

if_false:
    ret i32 %b
}

; while 循环
define void @loop_example() {
entry:
    %i = alloca i32
    store i32 0, i32* %i
    br label %loop_cond

loop_cond:
    %1 = load i32, i32* %i
    %2 = icmp slt i32 %1, 10
    br i1 %2, label %loop_body, label %loop_end

loop_body:
    ; loop body
    %3 = load i32, i32* %i
    %4 = add i32 %3, 1
    store i32 %4, i32* %i
    br label %loop_cond

loop_end:
    ret void
}
```

### 3.3 IR 独立测试策略

#### 3.3.1 IR 验证器

```cpp
// ir/verifier.hpp
class IRVerifier {
public:
    bool verify(Module* module);
    bool verify(Function* func);
    bool verify(BasicBlock* block);
    
    const std::vector<std::string>& getErrors() const;
    
private:
    std::vector<std::string> errors;
    
    // 检查项
    void checkOperands(Instruction* inst);
    void checkTerminator(BasicBlock* block);
    void checkSSAForm(Function* func);
    void checkTypes(Instruction* inst);
};

// 测试示例
TEST(IRVerifier, ValidModule) {
    auto module = parseIR(R"(
        define i32 @main() {
            ret i32 42
        }
    )");
    
    IRVerifier verifier;
    EXPECT_TRUE(verifier.verify(module.get()));
}

TEST(IRVerifier, UndefinedValue) {
    auto module = parseIR(R"(
        define i32 @main() {
            %1 = add i32 %undefined, 1
            ret i32 %1
        }
    )");
    
    IRVerifier verifier;
    EXPECT_FALSE(verifier.verify(module.get()));
    expectError("use of undefined value");
}

TEST(IRVerifier, MissingTerminator) {
    auto module = parseIR(R"(
        define i32 @main() {
            %1 = add i32 1, 2
        }
    )");
    
    IRVerifier verifier;
    EXPECT_FALSE(verifier.verify(module.get()));
    expectError("basic block lacks terminator");
}
```

#### 3.3.2 IR 解释器

```cpp
// ir/interpreter.hpp
class IRInterpreter {
public:
    // 执行 IR 函数，返回整数值
    int run(Function* func, const std::vector<int>& args);
    
    // 设置全局变量
    void setGlobal(const std::string& name, int value);
    
private:
    std::unordered_map<Value*, int> regValues;
    std::unordered_map<std::string, int> globals;
    std::unordered_map<Value*, int> stack;  // alloca 空间
    
    int executeInst(Instruction* inst);
    int getValue(Value* val);
};

// 测试：IR 解释器验证语义正确性
TEST_F(IRInterpreterTest, SimpleArithmetic) {
    auto module = generateIR(R"(
        int main() {
            return 1 + 2 * 3;
        }
    )");
    
    auto result = interpreter.run(module->getFunction("main"), {});
    EXPECT_EQ(result, 7);
}

TEST_F(IRInterpreterTest, LocalVariable) {
    auto module = generateIR(R"(
        int main() {
            int a = 10;
            int b = 20;
            return a + b;
        }
    )");
    
    auto result = interpreter.run(module->getFunction("main"), {});
    EXPECT_EQ(result, 30);
}

TEST_F(IRInterpreterTest, IfStatement) {
    auto module = generateIR(R"(
        int max(int a, int b) {
            if (a > b) return a;
            else return b;
        }
        int main() {
            return max(10, 20);
        }
    )");
    
    auto result = interpreter.run(module->getFunction("main"), {});
    EXPECT_EQ(result, 20);
}

TEST_F(IRInterpreterTest, WhileLoop) {
    auto module = generateIR(R"(
        int main() {
            int sum = 0;
            int i = 0;
            while (i < 10) {
                sum = sum + i;
                i = i + 1;
            }
            return sum;
        }
    )");
    
    auto result = interpreter.run(module->getFunction("main"), {});
    EXPECT_EQ(result, 45);  // 0+1+2+...+9
}
```

#### 3.3.3 IR 生成单元测试

```cpp
// test/irgen_test.cpp
class IRGenTest : public ::testing::Test {
protected:
    std::unique_ptr<ir::Module> generate(const std::string& source) {
        auto ast = parse(source);
        IRGenerator gen;
        return gen.generate(ast.get());
    }
};

// 测试 IR 结构
TEST_F(IRGenTest, SimpleReturn) {
    auto ir = generate(R"(
        int main() {
            return 42;
        }
    )");
    
    auto func = ir->getFunction("main");
    ASSERT_NE(func, nullptr);
    
    auto& blocks = func->blocks;
    ASSERT_EQ(blocks.size(), 1);
    
    auto& insts = blocks.front()->instructions;
    ASSERT_EQ(insts.size(), 1);
    
    auto* ret = dynamic_cast<ir::ReturnInst*>(insts.front());
    ASSERT_NE(ret, nullptr);
    
    auto* val = dynamic_cast<ir::ConstantInt*>(ret->getValue());
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(val->value, 42);
}

// 快照测试：比对生成的 IR 文本
TEST_F(IRGenTest, VariableDecl_Snapshot) {
    auto ir = generate(R"(
        int main() {
            int a = 10;
            int b = 20;
            return a + b;
        }
    )");
    
    std::ostringstream oss;
    ir::IRPrinter(oss).print(ir.get());
    
    // 与快照文件比对
    expectSnapshotMatch("variable_decl", oss.str());
}
```

### 3.4 IR 层工作流程

```
┌─────────────────────────────────────────────────────────────┐
│                      IR 开发与测试流程                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  1. 定义 IR 数据结构 (ir.hpp)                                │
│     └── 测试：创建简单 IR 对象，验证内存管理                   │
│                                                             │
│  2. 实现 IR 文本解析器 (ir_parser.cpp)                       │
│     └── 测试：解析示例 IR，比对结构                          │
│                                                             │
│  3. 实现 IR 文本生成器 (ir_printer.cpp)                      │
│     └── 测试：round-trip 测试（解析→打印→解析）              │
│                                                             │
│  4. 实现 IR 验证器 (verifier.cpp)                            │
│     └── 测试：编写非法 IR，验证报错                          │
│                                                             │
│  5. 实现 IR 解释器 (interpreter.cpp)                         │
│     └── 测试：编写 SysY 程序，解释执行验证结果               │
│                                                             │
│  6. 实现 IR 生成器 (从 AST 生成 IR)                          │
│     └── 测试：SysY → IR → 解释器执行                         │
│                                                             │
│  7. 实现优化器（可选）                                        │
│     └── 测试：优化前后语义等价性验证                          │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 四、后端独立开发与测试

### 4.1 后端架构

```
IR Module
    │
    ▼
┌─────────────────────────────────────┐
│         Instruction Selection        │  ← 将 IR 指令映射到目标指令
│         (Pattern Matching)           │
└─────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────┐
│         Register Allocation          │  ← 虚拟寄存器 → 物理寄存器
│         (Graph Coloring / Linear Scan)│
└─────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────┐
│         Assembly Generation          │  ← 生成汇编文本
└─────────────────────────────────────┘
```

### 4.2 后端独立测试策略

#### 4.2.1 指令选择测试

```cpp
// test/backend/isel_test.cpp
TEST_F(ISelTest, SimpleAdd) {
    auto ir = parseIR(R"(
        define i32 @test(i32 %a, i32 %b) {
            %1 = add i32 %a, %b
            ret i32 %1
        }
    )");
    
    RISCVBackend backend;
    auto machineFunc = backend.selectInstructions(ir->getFunction("test"));
    
    // 验证选择了正确的 RISC-V 指令
    auto& insts = machineFunc->blocks[0]->instructions;
    ASSERT_EQ(insts.size(), 1);
    
    auto* add = insts[0];
    EXPECT_EQ(add->opcode, RISCV::ADD);
    EXPECT_EQ(add->operands.size(), 3);  // rd, rs1, rs2
}
```

#### 4.2.2 寄存器分配测试

```cpp
TEST_F(RegAllocTest, SimpleAllocation) {
    auto func = createMachineFunction(R"(
        add %virt1, %virt2, %virt3
        sub %virt4, %virt1, %virt5
    )");
    
    LinearScanRegAlloc allocator({RISCV::X5, RISCV::X6, RISCV::X7});
    allocator.allocate(func.get());
    
    // 验证虚拟寄存器被分配到物理寄存器
    for (auto& inst : func->blocks[0]->instructions) {
        for (auto& operand : inst->operands) {
            if (operand->isVirtual()) {
                EXPECT_TRUE(operand->isAllocated());
            }
        }
    }
}
```

#### 4.2.3 汇编生成测试

```cpp
TEST_F(AsmGenTest, FunctionPrologue) {
    auto func = createMachineFunction(R"(
        func @main
        entry:
            alloca 16
            store 42, [%sp + 12]
            ret 42
    )");
    
    std::ostringstream oss;
    RISCVAsmPrinter(oss).print(func.get());
    
    EXPECT_EQ(oss.str(), R"(
main:
    addi sp, sp, -16
    li t0, 42
    sw t0, 12(sp)
    li a0, 42
    addi sp, sp, 16
    ret
)");
}
```

### 4.3 后端集成测试

```cpp
// 完整的后端测试
TEST_F(BackendTest, EndToEnd) {
    auto ir = parseIR(R"(
        define i32 @main() {
            %1 = add i32 10, 20
            %2 = mul i32 %1, 2
            ret i32 %2
        }
    )");
    
    // 生成汇编
    RISCVBackend backend;
    auto asmFile = backend.compile(ir.get());
    
    // 用 GNU 工具链汇编链接
    system("riscv64-unknown-elf-gcc -static -o test.out test.s");
    
    // 用 QEMU 运行
    int result = system("qemu-riscv64 ./test.out; echo $?");
    
    EXPECT_EQ(result, 60);  // (10+20)*2 = 60
}
```

---

## 五、完整测试金字塔

```
                    ┌─────────────┐
                    │   E2E 测试   │  ← 完整 SysY 程序编译运行
                    │  (test/e2e) │
                    └──────┬──────┘
                           │
           ┌───────────────┼───────────────┐
           │               │               │
           ▼               ▼               ▼
    ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
    │   IR 测试    │ │  后端测试    │ │  前端测试    │
    │ (test/ir)   │ │(test/backend)│ │(test/frontend)│
    └──────┬──────┘ └──────┬──────┘ └──────┬──────┘
           │               │               │
           ▼               ▼               ▼
    ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
    │ IR 解释器   │ │ 指令选择    │ │ Tokenizer   │
    │ 验证器      │ │ 寄存器分配  │ │ Parser      │
    │ 快照测试    │ │ 汇编打印    │ │ 语义分析    │
    └─────────────┘ └─────────────┘ └─────────────┘
```

### 5.1 测试运行脚本

```bash
#!/bin/bash
# test/run_all.sh

set -e

echo "=== Running Frontend Tests ==="
./test/frontend/run_tests.sh

echo "=== Running IR Tests ==="
./test/ir/run_tests.sh

echo "=== Running Backend Tests ==="
./test/backend/run_tests.sh

echo "=== Running E2E Tests ==="
./test/e2e/run_tests.sh

echo "=== All Tests Passed ==="
```

---

## 六、开发迭代计划

### Phase 1: 基础设施（1-2 周）

- [ ] 搭建单元测试框架（Google Test）
- [ ] 重构 TokenStream
- [ ] 为现有 Tokenizer 添加测试
- [ ] 为现有 Parser 添加测试

### Phase 2: IR 层开发（2-3 周）

- [ ] 设计 IR 数据结构
- [ ] 实现 IR Printer/Parser
- [ ] 实现 IR Verifier
- [ ] 实现 IR Interpreter
- [ ] 从 AST 生成 IR（基础功能）

### Phase 3: 功能完善（3-4 周）

- [ ] 完善表达式解析
- [ ] 实现变量定义与访问
- [ ] 实现控制流语句
- [ ] 实现函数调用
- [ ] 完善 IR 生成器

### Phase 4: 后端开发（2-3 周）

- [ ] 实现指令选择
- [ ] 实现寄存器分配
- [ ] 完善汇编生成
- [ ] 添加后端测试

### Phase 5: 优化与完善（持续）

- [ ] 实现基础优化（常量折叠、死代码消除）
- [ ] 完善错误处理
- [ ] 添加更多测试用例
- [ ] 性能优化

---

## 七、总结

**核心原则**:
1. **分层解耦**: 前端、IR、后端各自独立，通过标准接口交互
2. **测试驱动**: 每个组件都有独立的单元测试，确保可维护性
3. **快速迭代**: IR 解释器支持快速验证语义，无需等待完整代码生成

**关键收益**:
- 调试效率提升：IR 可读，解释器可单步执行
- 可靠性提升：每层都有独立验证机制
- 扩展性提升：添加新后端只需实现指令选择
- 协作友好：不同模块可由不同开发者并行开发
