# 当前项目架构可优化点

## 一、架构现状分析

当前编译器采用**直接翻译**模式：源码 → Token → AST → 汇编。这种设计在初期简单直接，但随着功能增加会面临扩展性瓶颈。

```
当前数据流:
SysY 源码 → [Tokenizer] → TokenList → [Parser] → AST → [Codegen] → RISC-V 汇编
                                      ↓
                                 (直接遍历生成)
```

---

## 二、高优先级优化点

### 2.1 引入中间表示（IR）层 ⭐⭐⭐

**现状问题**:
- AST 直接用于代码生成，语义信息与目标代码混杂
- 前端优化（如常量折叠）难以实施
- 后端无法独立测试，必须依赖完整前端

**优化方案**:

```
优化后数据流:
SysY 源码 → Tokenizer → Parser → AST → IR生成器 → IR 
                                               ↓
                                          [优化器]
                                               ↓
                                         代码生成器 → RISC-V 汇编
```

**IR 设计建议**:

```cpp
// 建议采用类 LLVM IR 的三地址码形式
enum class IROp {
    ADD, SUB, MUL, DIV, MOD,    // 算术运算
    LOAD, STORE,                // 内存访问
    ALLOCA,                     // 栈分配
    CALL, RET,                  // 函数调用/返回
    LABEL, BR, COND_BR,         // 控制流
    ICMP, FCMP,                 // 比较
    PHI,                        // SSA 合并
};

struct IRInst {
    IROp op;
    IRValue* dst;           // 结果值
    std::vector<IRValue*> operands;  // 操作数
};

struct IRFunction {
    std::string name;
    std::vector<IRValue*> params;
    std::list<IRBasicBlock*> blocks;
    std::list<IRInst*> instructions;
};
```

**收益**:
- 前后端解耦，可独立开发和测试
- 支持优化器独立运作
- 便于添加新的后端（如 x86、ARM）

---

### 2.2 重构 AST 节点类层次 ⭐⭐⭐

**现状问题**:

```cpp
// 当前设计：使用虚函数和继承，但存在缺陷
class ASTNode {
    virtual void addParams(Token *&Tok) { error(...); }
    virtual void addBody(ASTNode* block) { error(...); }
    virtual ASTNode *getBody() { error(...); return NULL; }
    virtual ASTNode *getRHS() { error(...); return nullptr; }
    virtual ASTNode *getLHS() { error(...); return nullptr; }
};
```

**问题分析**:
1. **接口污染**: 基类包含所有子类可能需要的方法，违反接口隔离原则
2. **运行时错误**: 方法在不适用的子类中调用时才报错，而非编译期
3. **扩展困难**: 新增节点类型需要修改基类

**优化方案 - Visitor 模式 + 类型区分**:

```cpp
// 方案1: 使用 std::variant (C++17) 实现类型安全
enum class NodeKind {
    Function, Block, Return, Binary, Unary, 
    Number, Variable, If, While, For, Call
};

class ASTNode {
public:
    NodeKind kind;
    SourceLocation loc;  // 源码位置（错误报告）
    virtual ~ASTNode() = default;
};

// 各类节点只包含自己的数据
class FuncNode : public ASTNode {
public:
    std::string name;
    Type* retType;
    std::vector<VarNode*> params;
    BlockNode* body;
};

class BinaryNode : public ASTNode {
public:
    BinaryOp op;
    ASTNode* lhs;
    ASTNode* rhs;
};

// Visitor 模式遍历
class ASTVisitor {
public:
    virtual void visit(FuncNode* node) = 0;
    virtual void visit(BlockNode* node) = 0;
    virtual void visit(ReturnNode* node) = 0;
    virtual void visit(BinaryNode* node) = 0;
    // ...
};
```

**收益**:
- 编译期类型安全
- 新增节点类型无需修改现有代码（开闭原则）
- 遍历逻辑与节点数据分离

---

### 2.3 符号表与作用域管理 ⭐⭐⭐

**现状问题**:
- 无符号表，变量信息无处存储
- 无法处理作用域嵌套（如块级作用域）
- 同名变量无法区分

**优化方案**:

```cpp
// 符号表条目
struct Symbol {
    std::string name;
    Type* type;
    enum { VAR, FUNC, CONST } kind;
    union {
        struct { int offset; bool isGlobal; } var;  // 变量信息
        struct { std::vector<Type*> params; } func; // 函数信息
    };
    ASTNode* decl;  // 指向声明节点（用于错误报告）
};

// 作用域（支持嵌套）
class Scope {
    Scope* parent;
    std::unordered_map<std::string, Symbol*> symbols;
public:
    Symbol* lookup(const std::string& name);      // 递归查找
    Symbol* lookupCurrent(const std::string& name); // 仅当前作用域
    void define(Symbol* sym);
};

// 符号表管理器
class SymbolTable {
    Scope* current;
    std::vector<std::unique_ptr<Scope>> allScopes;
public:
    void enterScope();
    void exitScope();
    Symbol* lookup(const std::string& name);
    void define(const std::string& name, Type* type, ...);
};
```

**使用示例**:

```cpp
// 解析时维护符号表
ASTNode* Parser::parseBlock() {
    symTable.enterScope();  // 进入新作用域
    
    while (not at("}")) {
        if (match("int")) {
            auto var = parseVarDecl();
            symTable.define(var->name, var->type);  // 注册符号
        }
        // ...
    }
    
    symTable.exitScope();   // 退出作用域
}
```

---

### 2.4 错误处理机制改进 ⭐⭐

**现状问题**:
- `error()` 直接 `exit(1)`，无法恢复
- 无源码位置信息
- 错误信息格式单一

**优化方案**:

```cpp
// 错误级别
enum class ErrorLevel { WARNING, ERROR, FATAL };

// 源码位置
struct SourceLocation {
    std::string filename;
    int line;
    int column;
    std::string sourceLine;  // 源码内容（用于高亮）
};

// 错误信息
struct CompileError {
    ErrorLevel level;
    SourceLocation loc;
    std::string message;
    std::string hint;  // 修复建议
};

// 错误报告器
class ErrorReporter {
    std::vector<CompileError> errors;
    int maxErrors = 10;
    
public:
    void report(ErrorLevel level, const SourceLocation& loc, 
                const std::string& msg, const std::string& hint = "");
    
    bool hasError() const { return !errors.empty(); }
    void printAll();
    
    // 格式化错误输出（类似 Rust 风格）
    void printPretty(const CompileError& err);
};

// 使用示例
try {
    auto ast = parser.parse();
} catch (const ParseError& e) {
    reporter.report(ErrorLevel::ERROR, e.loc, e.what(), "expected ';' here");
    if (reporter.errorCount() >= maxErrors) {
        reporter.printAll();
        exit(1);
    }
}
```

**输出示例**:
```
error: expected ';' after return statement
  --> 2023test/01_var_defn2.sy:3:15
   |
 3 |     a = b + c
   |               ^ help: add ';' here
```

---

### 2.5 Token 流管理优化 ⭐⭐

**现状问题**:

```cpp
// 当前设计：通过引用传递修改指针位置，容易出错
static ASTNode *FuncDef(Token *&Tok) {
    declspec(Tok);        // Tok 前进
    FuncNode *Nd = new FuncNode(Tok, ND_FUN);
    Nd->addParams(Tok);   // Tok 前进
    Nd->addBody(Block(Tok)); // Tok 前进
    return Nd;
}
```

**问题**:
1. 副作用不透明：调用函数会修改 Tok
2. 回溯困难：一旦前进无法回退
3. 没有 lookahead 能力

**优化方案**:

```cpp
// 封装 Token 流
class TokenStream {
    std::vector<Token*> tokens;
    size_t pos = 0;
    
public:
    TokenStream(TokenList* list);
    
    // 查看当前/后续 Token（不前进）
    Token* peek(size_t offset = 0) const;
    
    // 消费当前 Token（前进）
    Token* consume();
    
    // 尝试消费特定 Token
    bool consume(const std::string& expect);
    
    // 必须消费特定 Token，否则报错
    Token* expect(const std::string& expect);
    
    // 检查当前 Token 类型
    bool check(TokenKind kind) const;
    bool check(const std::string& name) const;
    
    // 保存/恢复位置（支持回溯）
    size_t save() const { return pos; }
    void restore(size_t p) { pos = p; }
    
    bool eof() const;
};
```

**使用示例**:

```cpp
ASTNode* Parser::parseFuncDef() {
    auto retType = parseType();
    auto name = tokens.expectIdentifier();
    tokens.expect("(");
    
    auto params = parseParams();  // 内部使用 tokens
    
    tokens.expect(")");
    auto body = parseBlock();
    
    return new FuncNode(retType, name, params, body);
}
```

---

## 三、中优先级优化点

### 3.1 内存管理现代化

**现状**: 大量使用裸指针 `new/delete`

**优化方案**:

```cpp
// 使用智能指针和自定义分配器
using NodePtr = std::unique_ptr<ASTNode>;
using SymbolPtr = std::unique_ptr<Symbol>;

class ASTAllocator {
    std::vector<std::unique_ptr<char[]>> blocks;
    static constexpr size_t BLOCK_SIZE = 64 * 1024;  // 64KB
    
public:
    template<typename T, typename... Args>
    T* alloc(Args&&... args) {
        // 在内存池中分配，提高性能
    }
};

// 节点创建使用工厂方法
template<typename T, typename... Args>
T* makeNode(Args&&... args) {
    return allocator.alloc<T>(std::forward<Args>(args)...);
}
```

---

### 3.2 类型系统重构

**现状**: `Type` 类几乎为空，无实际类型检查

**优化方案**:

```cpp
enum class TypeKind {
    VOID, INT, FLOAT, BOOL,
    ARRAY, POINTER, FUNCTION
};

class Type {
public:
    TypeKind kind;
    int size;           // 类型大小（字节）
    int align;          // 对齐要求
    
    // 数组类型
    Type* base;         // 元素类型
    int arrayLen;       // 数组长度
    
    // 函数类型
    Type* retType;
    std::vector<Type*> paramTypes;
};

// 类型检查器
class TypeChecker : public ASTVisitor {
public:
    void visit(BinaryNode* node) override {
        auto lhsType = check(node->lhs);
        auto rhsType = check(node->rhs);
        
        if (!lhsType->isCompatible(rhsType)) {
            error(node->loc, "type mismatch in binary expression");
        }
        
        node->type = lhsType;  // 设置表达式类型
    }
};
```

---

### 3.3 源码位置追踪

**现状**: Token 和 AST 节点无位置信息，错误难以定位

**优化方案**:

```cpp
// Token 增加位置
class Token {
    TokenKind kind;
    std::string text;
    int line, column;
    std::string* sourceFile;  // 指向源文件（支持多文件编译）
};

// AST 节点继承位置
class ASTNode {
    SourceLocation loc;
public:
    explicit ASTNode(const SourceLocation& l) : loc(l) {}
    const SourceLocation& location() const { return loc; }
};

// 创建节点时自动传递位置
auto node = makeNode<NumNode>(tok.loc, value);
```

---

## 四、低优先级优化点

### 4.1 编译器驱动重构

```cpp
// 封装编译流程
class CompilerDriver {
    CompileOptions options;
    ErrorReporter reporter;
    
public:
    bool compile(const std::string& sourceFile) {
        // 1. 词法分析
        auto tokens = Lexer(sourceFile).tokenize();
        if (reporter.hasError()) return false;
        
        // 2. 语法分析
        auto ast = Parser(tokens, reporter).parse();
        if (reporter.hasError()) return false;
        
        // 3. 语义分析
        SemanticAnalyzer(reporter).analyze(ast);
        if (reporter.hasError()) return false;
        
        // 4. IR 生成
        auto ir = IRGenerator().generate(ast);
        
        // 5. 优化
        if (options.optimize) {
            Optimizer().run(ir);
        }
        
        // 6. 代码生成
        CodeGenerator(options.target).generate(ir, outputFile);
        
        return true;
    }
};
```

### 4.2 测试框架

```cpp
// 单元测试支持
class CompilerTest : public ::testing::Test {
protected:
    std::string compileAndRun(const std::string& source) {
        // 编译源码
        // 用 QEMU 运行
        // 返回输出
    }
};

TEST_F(CompilerTest, SimpleReturn) {
    auto source = R"(
        int main() {
            return 42;
        }
    )";
    EXPECT_EQ(compileAndRun(source), 42);
}
```

---

## 五、优化路线图

| 阶段 | 优化项 | 优先级 | 预估工作量 |
|------|--------|--------|-----------|
| 1 | IR 层设计 | ⭐⭐⭐ | 3-4 天 |
| 2 | 符号表实现 | ⭐⭐⭐ | 2-3 天 |
| 3 | AST Visitor 重构 | ⭐⭐⭐ | 2-3 天 |
| 4 | TokenStream 封装 | ⭐⭐ | 1-2 天 |
| 5 | 错误处理改进 | ⭐⭐ | 1-2 天 |
| 6 | 内存管理优化 | ⭐⭐ | 1-2 天 |
| 7 | 类型系统完善 | ⭐⭐ | 2-3 天 |
| 8 | 测试框架搭建 | ⭐ | 1-2 天 |

**建议实施顺序**: 
1. 先完成 TokenStream 封装（立即收益）
2. 并行开发符号表和 IR 设计
3. 然后重构 AST 和错误处理
4. 最后完善类型系统和测试框架
