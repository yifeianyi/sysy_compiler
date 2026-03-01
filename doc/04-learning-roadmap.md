# SysY 编译器 - 学习导向开发计划

> **学习目标**: 通过动手实现一个完整的编译器，深入理解编译原理的核心概念
> **核心理念**: **先走通简单路，再修高速道** —— 先理解 AST 直接翻译的本质，再引入 IR 解耦
> **测试文件**: 2023test/ 目录下按阶段分类，共 137 个测试用例

---

## 学习哲学

```
传统学习路径                    本计划学习路径
                                              
阶段1: 前端                      阶段0-1: 前端 + 热身 + 表达式
阶段2: 设计IR                    阶段2-4: AST直译后端（核心！）
阶段3: IR生成                    阶段5: 引入IR（解耦优化）
阶段4: 后端                      阶段6-7: 现代后端 + 高级特性
阶段5: 优化                      

问题：IR抽象难以理解              优势：先看到端到端效果
      前后端解耦摸不着头脑             亲手感受复杂度爆炸
                                    自然理解为什么需要IR
```

**关键转变**：
- **先**：手工管理寄存器、硬编码指令选择 → **感受复杂度的来源**
- **后**：引入虚拟寄存器、指令选择算法、IR → **理解抽象的价值**

---

## 阶段总览

| 阶段 | 内容 | 测试数 | 关键特性 |
|------|------|--------|----------|
| 阶段0 | 热身与修复 | 4 | 注释、空语句、基础框架 |
| 阶段1 | 表达式 | 22 | 四则运算、优先级、一元运算、十六进制 |
| 阶段2 | 变量与作用域 | 13 | 变量定义、常量、块级作用域 |
| 阶段3 | 控制流 | 15 | if-else、while、break、continue |
| 阶段4 | 函数调用 | 5 | 函数定义、调用、参数传递 |
| 阶段5 | 复杂度危机与IR引入 | 4 | 短路求值、副作用、DCT |
| 阶段6 | 复杂算法综合测试 | 33 | 排序、图算法、字符串、综合测试 |
| 阶段7 | 高级特性 | 41 | 数组、浮点数、大量参数/变量 |

---

## 阶段 0: 热身与修复 (Week 1)

### 🎯 学习目标
- 彻底理解现有代码的每一行
- 修复现有 bug，建立端到端的信心
- 理解"直接翻译"的基本流程

### 📁 测试文件 (4个)
| 文件 | 说明 |
|------|------|
| `00_main.sy` | 最基础的 return 常量 |
| `00_comment2.sy` | 注释测试 |
| `42_empty_stmt.sy` | 空语句测试 |
| `45_comment1.sy` | 单行/多行注释 |

### 📚 理论学习
**必读材料**:
1. 《编译原理》(龙书) 第1章：编译器结构概览
2. [Let's Build a Compiler](https://xmonader.github.io/letsbuildacompiler-js/)

### 🔧 实践任务

#### 任务 0.1: 代码走读与注释
为现有代码添加详细中文注释，确保理解端到端流程：

```cpp
// main.cpp 执行流程
// 1. parseArgs()    - 解析命令行
// 2. tokenizeFile() - 词法分析 → TokenList
// 3. ASTBuild()     - 语法分析 → AST
// 4. codegen()      - 代码生成 → 汇编文件
```

#### 任务 0.2: 修复关键 Bug

**Bug 1: return 语句数值解析错误**

```cpp
// src/ast.cpp:89-94（修复后）
static ASTNode *Stmt(Token *&Tok){
    if(Tok->Name == "return"){
        Token* retTok = Tok;        // 保存 return Token
        Tok = Tok->Next;            // 移动到返回值
        
        UnaryNode *Nd = new UnaryNode(retTok, ND_RETURN);
        
        if (Tok->Kind == TK_NUM) {  // 有返回值
            Nd->LHS = new NumNode(Tok, ND_NUM);
        }
        
        skip(Tok, ";");
        return Nd;
    }
}
```

**Bug 2: 关键字识别失效**

```cpp
// src/tokenize.cpp:27（修复后）
int len = sizeof(keywords) / sizeof(keywords[0]);
```

### ✅ 验证标准
- [ ] 能正确编译 `return 42;` 并输出 `li a0, 42`
- [ ] 理解 Token → AST → 汇编 的完整流程

---

## 阶段 1: 表达式与手工寄存器分配 (Week 2-3) ⭐核心基础

### 🎯 学习目标
- 掌握 **AST 直接翻译** 的本质
- 理解 **手工寄存器分配** 的痛苦
- 感受 **为什么需要寄存器分配算法**

### 📁 测试文件 (22个)
| 类别 | 文件 |
|------|------|
| 四则运算 | `11_add2.sy`, `12_addc.sy`, `13_sub2.sy`, `14_subc.sy`, `15_mul.sy`, `16_mulc.sy`, `17_div.sy`, `18_divc.sy` |
| 取模 | `19_mod.sy`, `20_rem.sy` |
| 优先级 | `35_op_priority1~5.sy`, `38_op_priority4.sy` |
| 一元运算 | `40_unary_op.sy`, `41_unary_op2.sy` |
| 其他 | `29_long_line.sy`, `44_stmt_expr.sy`, `46_hex_defn.sy`, `47_hex_oct_add.sy`, `48_assign_complex_expr.sy` |

### 📚 理论学习
**必读材料**:
- 龙书 第2章：简单语法指导翻译
- 龙书 第4章：自顶向下分析（递归下降）

### 🔧 实践任务

#### 任务 1.1: 实现表达式解析

**扩展文法**:
```
Expr        = AddExpr
AddExpr     = MulExpr (('+' | '-') MulExpr)*
MulExpr     = UnaryExpr (('*' | '/' | '%') UnaryExpr)*
UnaryExpr   = ('+' | '-' | '!')* PrimaryExpr
PrimaryExpr = Num | '(' Expr ')'
```

#### 任务 1.2: 手工寄存器分配 ⭐关键体验

**不使用虚拟寄存器，直接分配物理寄存器**：

```cpp
// codegen.cpp - 手工寄存器分配版本
class CodeGen {
    // 可用寄存器池（RISC-V 临时寄存器）
    const char* regs[7] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6"};
    int regCounter = 0;
    
    // 获取一个空闲寄存器
    const char* allocReg() {
        if (regCounter >= 7) {
            // 💥 寄存器用完了！怎么办？
            error("寄存器不足！需要溢出到栈");
        }
        return regs[regCounter++];
    }
    
    void freeAllRegs() { regCounter = 0; }
    
public:
    void genExpr(ASTNode* node) {
        if (node->isNum()) {
            const char* r = allocReg();
            printLn("  li %s, %d", r, node->getVal());
            node->resultReg = r;
        }
        else if (node->isBinary()) {
            genExpr(node->lhs);  // 结果在 r1
            genExpr(node->rhs);  // 结果在 r2
            const char* rd = allocReg();
            printLn("  add %s, %s, %s", 
                rd, node->lhs->resultReg, node->rhs->resultReg);
            node->resultReg = rd;
        }
    }
};
```

### ✅ 验证标准
- [ ] 支持四则运算和取模
- [ ] 运算符优先级正确
- [ ] 支持一元运算符 (+, -, !)
- [ ] 支持十六进制常量
- [ ] 手工寄存器分配工作（简单表达式）

---

## 阶段 2: 变量与栈帧管理 (Week 4-5) ⭐核心基础

### 🎯 学习目标
- 理解 **栈帧布局**
- 掌握 **变量到栈的映射**
- 感受 **作用域的复杂性**

### 📁 测试文件 (13个)
| 类别 | 文件 |
|------|------|
| 变量定义 | `01_var_defn2.sy`, `02_var_defn3.sy`, `01_multiple_returns.sy` |
| 常量 | `06_const_var_defn2.sy`, `07_const_var_defn3.sy` |
| 作用域 | `25_scope3.sy`, `26_scope4.sy`, `27_scope5.sy`, `52_scope.sy`, `53_scope2.sy`, `54_hidden_var.sy` |
| 块级变量 | `02_ret_in_block.sy`, `43_logi_assign.sy` |

### 🔧 实践任务

#### 任务 2.1: 设计栈帧布局

```
高地址
┌─────────────────┐
│  返回地址 (ra)   │ ← fp + 8
├─────────────────┤
│  旧 fp          │ ← fp (保存)
├─────────────────┤
│  局部变量 1      │ ← fp - 4
│  局部变量 2      │ ← fp - 8
│  ...            │
│  溢出寄存器空间  │
└─────────────────┘
低地址
```

#### 任务 2.2: 实现变量定义和访问

```cpp
class CodeGen {
    // 变量名 → 栈偏移映射
    std::map<std::string, int> varOffset;
    int currentOffset = -4;
    
public:
    void genDecl(ASTNode* node) {
        // int x = expr;
        genExpr(node->init);
        varOffset[node->name] = currentOffset;
        printLn("  sw %s, %d(fp)", 
            node->init->resultReg, currentOffset);
        currentOffset -= 4;
    }
    
    void genVarRef(ASTNode* node) {
        int offset = varOffset[node->name];
        const char* r = allocReg();
        printLn("  lw %s, %d(fp)", r, offset);
        node->resultReg = r;
    }
};
```

#### 任务 2.3: 支持块级作用域

```c
int main() {
    int x = 1;      // fp-4
    {
        int x = 2;  // fp-8（遮蔽外部x）
        return x;   // 加载 fp-8
    }
    return x;       // 加载 fp-4
}
```

**需要栈结构保存每层作用域的变量**

### ✅ 验证标准
- [ ] 支持变量定义和赋值
- [ ] 支持表达式中使用变量
- [ ] 支持块级作用域和变量遮蔽
- [ ] 栈帧布局正确

---

## 阶段 3: 控制流与标签管理 (Week 6)

### 🎯 学习目标
- 理解 **标签和跳转**
- 掌握 **条件分支生成**
- 感受 **控制流翻译的细节**

### 📁 测试文件 (15个)
| 类别 | 文件 |
|------|------|
| if-else | `03_branch.sy`, `21_if_test2~5.sy`, `33_multi_branch.sy` |
| while | `26_while_test1~3.sy`, `34_multi_loop.sy` |
| while+if | `31_while_if_test1~3.sy`, `33_while_if_test3.sy` |
| break/continue | `04_break_continue.sy`, `30_continue.sy` |

### 🔧 实践任务

#### 任务 3.1: 实现 if-else

```cpp
void genIf(IfNode* node) {
    std::string elseLabel = newLabel("else");
    std::string endLabel = newLabel("end");
    
    genExpr(node->cond);
    printLn("  beqz %s, %s", node->cond->resultReg, elseLabel.c_str());
    
    genStmt(node->thenBranch);
    printLn("  j %s", endLabel.c_str());
    
    printLn("%s:", elseLabel.c_str());
    if (node->elseBranch) {
        genStmt(node->elseBranch);
    }
    printLn("%s:", endLabel.c_str());
}
```

#### 任务 3.2: 实现 while

```cpp
void genWhile(WhileNode* node) {
    std::string condLabel = newLabel("cond");
    std::string endLabel = newLabel("end");
    
    breakStack.push(endLabel);
    continueStack.push(condLabel);
    
    printLn("%s:", condLabel.c_str());
    genExpr(node->cond);
    printLn("  beqz %s, %s", node->cond->resultReg, endLabel.c_str());
    
    genStmt(node->body);
    printLn("  j %s", condLabel.c_str());
    
    printLn("%s:", endLabel.c_str());
    breakStack.pop();
    continueStack.pop();
}
```

### ✅ 验证标准
- [ ] if-else 正确执行
- [ ] while 循环正确执行
- [ ] 支持 break/continue
- [ ] 支持嵌套控制流

---

## 阶段 4: 函数调用与 ABI (Week 7)

### 🎯 学习目标
- 理解 **调用约定**
- 掌握 **寄存器保存/恢复**
- 感受 **函数调用的复杂性**

### 📁 测试文件 (5个)
| 文件 | 说明 |
|------|------|
| `05_param_name.sy` | 参数名处理 |
| `06_func_name.sy` | 函数命名 |
| `09_func_defn.sy` | 函数定义与调用基础 |
| `10_var_defn_func.sy` | 变量+函数调用 |
| `25_while_if.sy` | while+if+多函数调用 |

### 🔧 实践任务

#### 任务 4.1: 实现函数调用

```cpp
void genCall(CallNode* node) {
    // 1. 计算参数
    for (int i = 0; i < node->args.size(); i++) {
        genExpr(node->args[i]);
        // 参数放入 a0-a7
        printLn("  mv a%d, %s", i, node->args[i]->resultReg);
    }
    
    // 2. 调用
    printLn("  call %s", node->funcName.c_str());
    
    // 3. 返回值在 a0
    const char* r = allocReg();
    printLn("  mv %s, a0", r);
    node->resultReg = r;
}
```

#### 任务 4.2: 实现递归函数

```c
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
```

**需要正确处理**:
- 参数传递（a0）
- 返回地址保存（ra）
- 局部变量空间

### ✅ 验证标准
- [ ] 支持函数定义和调用
- [ ] 递归函数正确执行
- [ ] 参数传递正确

---

## 阶段 5: 复杂度危机与 IR 引入 (Week 8-9) ⭐关键转折

### 🎯 学习目标
- **回顾**：AST 直译的问题
- **引入**：IR 的价值
- **实现**：基于 IR 的编译器

### 📁 测试文件 (4个)
| 文件 | 说明 |
|------|------|
| `50_short_circuit.sy` | 逻辑短路求值 |
| `51_short_circuit3.sy` | 复杂短路求值 |
| `28_side_effect2.sy` | 副作用+短路求值 |
| `37_dct.sy` | DCT 算法（复杂计算） |

### 🔍 阶段性回顾（非常重要）

**当前架构的问题清单**：

| 问题 | 你遇到的痛苦 | 理想解决方案 |
|------|-------------|-------------|
| 寄存器分配 | 手工分配容易溢出，难跟踪生命周期 | 虚拟寄存器 + 分配算法 |
| 变量管理 | 作用域嵌套难处理，地址计算分散 | 符号表统一管理 |
| 表达式求值 | 结果寄存器传递混乱 | 三地址码，显式临时变量 |
| 优化 | 无从下手 | IR 上模式匹配优化 |
| 后端移植 | 汇编代码与 AST 紧耦合 | IR 到多后端的独立生成 |

### 🔧 实践任务

#### 任务 5.1: 设计简单 IR

```cpp
// 三地址码 IR
enum class IROp {
    ADD, SUB, MUL, DIV,     // 算术
    LOAD, STORE, ALLOCA,    // 内存
    LOAD_IMM,               // 立即数
    CMP, BR, COND_BR,       // 比较和跳转
    CALL, RET, PARAM        // 函数
};

struct IRInst {
    IROp op;
    int dst;        // 虚拟寄存器号（如 %1）
    int src1, src2; // 操作数
    int imm;        // 立即数值
};
```

### ✅ 验证标准
- [ ] 设计并实现简单 IR
- [ ] AST 能正确转为 IR
- [ ] IR 能正确转为汇编

---

## 阶段 6: 复杂算法综合测试 (Week 10-11)

### 🎯 学习目标
- 测试编译器在实际算法上的表现
- 发现和修复边界情况
- 积累调试经验

### 📁 测试文件 (33个)
| 类别 | 文件 |
|------|------|
| 图算法 | `09_BFS.sy`, `10_DFS.sy`, `18_prim.sy`, `21_union_find.sy`, `71_full_conn.sy`, `70_dijkstra.sy`, `75_max_flow.sy` |
| 树算法 | `11_BST.sy`, `13_LCA.sy` |
| 排序 | `20_sort.sy` |
| 搜索 | `19_search.sy`, `16_k_smallest.sy`, `17_maximal_clique.sy`, `76_n_queens.sy` |
| 字符串 | `23_json.sy`, `74_kmp.sy`, `77_substr.sy` |
| DP | `14_dp.sy`, `65_color.sy` |
| 其他 | `12_DSU.sy`, `15_graph_coloring.sy`, `22_matrix_multiply.sy`, `72_hanoi.sy`, `36_rotate.sy`, `38_light2d.sy`, `69_expr_eval.sy`, `64_calculator.sy`, `63_big_int_mul.sy`, `66_exgcd.sy`, `67_reverse_output.sy` |

### ✅ 验证标准
- [ ] 通过 80% 以上的测试用例
- [ ] 能正确编译并运行复杂算法

---

## 阶段 7: 高级特性 (Week 12+)

### 🎯 学习目标
- 支持数组（多维、初始化）
- 支持浮点数
- 处理大量参数/变量/嵌套

### 📁 测试文件 (41个)

#### 7.1 数组支持
| 文件 | 说明 |
|------|------|
| `03_arr_defn2.sy` | 二维数组定义 |
| `04_arr_defn3.sy` | 多维数组定义 |
| `05_arr_defn4.sy` | 数组定义 |
| `07_arr_init_nd.sy` | 多维数组初始化 |
| `08_const_array_defn.sy` | 常量数组 |
| `08_global_arr_init.sy` | 全局数组初始化 |
| `24_array_only.sy` | 纯数组操作 |
| `30_many_dimensions.sy` | 19维数组+大量参数 |
| `31_many_indirections.sy` | 多级数组访问 |
| `34_arr_expr_len.sy` | 数组长度表达式 |
| `55_sort_test1~7.sy` | 排序+数组 |

#### 7.2 浮点数支持
| 文件 | 说明 |
|------|------|
| `39_fp_params.sy` | 浮点参数（大量浮点运算） |
| `95_float.sy` | 浮点数综合测试 |
| `96_matrix_add.sy`, `97_matrix_sub.sy`, `98_matrix_mul.sy`, `99_matrix_tran.sy` | 浮点矩阵运算 |

#### 7.3 压力测试
| 文件 | 说明 |
|------|------|
| `79_var_name.sy` | 超长变量名 |
| `80_chaos_token.sy` | 混乱格式测试 |
| `81_skip_spaces.sy` | 空白字符处理 |
| `82_long_func.sy` | 超长函数 |
| `83_long_array.sy` | 大数组 |
| `84_long_array2.sy` | 大数组2 |
| `85_long_code.sy`, `86_long_code2.sy` | 超长代码 |
| `87_many_params.sy`, `88_many_params2.sy`, `32_many_params3.sy` | 大量参数 |
| `89_many_globals.sy` | 大量全局变量 |
| `90_many_locals.sy`, `91_many_locals2.sy` | 大量局部变量 |
| `92_register_alloc.sy` | 寄存器分配压力测试 |
| `93_nested_calls.sy` | 嵌套函数调用 |
| `94_nested_loops.sy` | 嵌套循环 |
| `78_side_effect.sy`, `28_side_effect2.sy` | 副作用测试 |

### ✅ 验证标准
- [ ] 支持一维/多维数组定义和访问
- [ ] 支持浮点数常量、运算、函数调用
- [ ] 通过所有压力测试

---

## 学习资源

### 必读
1. **《编译原理》(龙书)** - Aho, Sethi, Ullman
2. **《Engineering a Compiler》** - Cooper & Torczon

### 参考实现
1. [chibicc](https://github.com/rui314/chibicc) - 从 AST 直译开始，逐步引入 IR
2. [rvcc](https://github.com/andoShin/rvcc) - RISC-V 编译器，结构清晰

### 在线工具
- [Compiler Explorer](https://godbolt.org/) - 对比不同编译器输出
- [RISC-V Interpreter](https://www.cs.cornell.edu/courses/cs3410/2019sp/riscv/interpreter/)

---

## 总结：学习路径的关键转折

```
阶段0-4 (AST 直译阶段): 硬来！
    阶段0: 热身修复 ───────┐
    阶段1: 表达式 ─────────┤ 感受手工管理
    阶段2: 变量与栈帧 ─────┤ 寄存器/栈帧的
    阶段3: 控制流 ─────────┤ 痛苦和复杂性
    阶段4: 函数调用 ───────┘
    └── 💡 自然理解：为什么需要IR

阶段5-7 (IR 阶段): 豁然开朗！
    阶段5: 引入IR设计 ─────┐
    阶段6: 复杂算法测试 ───┤ 享受IR带来的
    阶段7: 高级特性 ───────┘ 便利和解耦
    └── 💡 深刻理解：工业编译器都用IR的原因
```

**核心收获**：
- 不是一上来就学习"标准答案"
- 而是先体验"为什么要这样设计"
- 通过 AST 直译的痛苦，真正理解 IR 的价值

祝你学习愉快！
