# SysY 编译器开发 TODO 列表

> 基于 `2023test/` 测试集，按阶段追踪实现进度  
> **当前状态**: 阶段0基本完成，阶段1-7待实现

---

## 阶段 0: 热身与修复 ✅ (4/4 测试通过)

### 已实现功能
- [x] 词法分析基础框架
  - [x] 数字常量解析 (TK_NUM)
  - [x] 标识符解析 (TK_IDENT)
  - [x] 关键字识别 (return, int, void, float, const, if, else, for, while, break, continue)
  - [x] 操作符解析 (+, -, *, /, %, {, }, (, ), ; 等)
  - [x] 注释跳过 (// 行注释, /* */ 块注释)
- [x] 语法分析基础框架
  - [x] 函数定义解析 (FuncDef)
  - [x] 代码块解析 (Block)
  - [x] return 语句解析
  - [x] 空语句解析 (;)
- [x] 代码生成基础
  - [x] RISC-V 汇编输出
  - [x] 函数入口/出口处理
  - [x] return 常量指令生成 (`li a0, N` + `ret`)

### 测试文件状态
| 文件 | 状态 | 说明 |
|------|------|------|
| `00_main.sy` | ✅ | 基础 return 常量 |
| `00_comment2.sy` | ✅ | 注释测试 |
| `42_empty_stmt.sy` | ✅ | 空语句测试 |
| `45_comment1.sy` | ✅ | 单行/多行注释 |

### 已知问题/待修复
- [ ] return 语句数值解析需要修复（当前直接取 Token 值，未正确移动到数字 Token）
- [ ] 关键字识别长度计算有 bug (`sizeof(keywords)/sizeof(keywords[0])` 应该是数组长度)

---

## 阶段 1: 表达式 ⬜ (0/22 测试通过)

### 待实现功能
- [ ] **四则运算** (+, -, *, /)
  - [ ] 加法运算 (11_add2.sy, 12_addc.sy)
  - [ ] 减法运算 (13_sub2.sy, 14_subc.sy)
  - [ ] 乘法运算 (15_mul.sy, 16_mulc.sy)
  - [ ] 除法运算 (17_div.sy, 18_divc.sy)
- [ ] **取模运算** (%, rem) (19_mod.sy, 20_rem.sy)
- [ ] **运算符优先级** (35_op_priority1~5.sy, 38_op_priority4.sy)
  - [ ] 先乘除后加减
  - [ ] 括号优先级
- [ ] **一元运算** (40_unary_op.sy, 41_unary_op2.sy)
  - [ ] 正号 (+)
  - [ ] 负号 (-)
  - [ ] 逻辑非 (!)
- [ ] **十六进制常量** (0x...) (46_hex_defn.sy, 47_hex_oct_add.sy)
- [ ] **复杂表达式** (29_long_line.sy, 44_stmt_expr.sy, 48_assign_complex_expr.sy)

### 测试文件清单
```
⬜ 11_add2.sy          ⬜ 12_addc.sy          ⬜ 13_sub2.sy
⬜ 14_subc.sy          ⬜ 15_mul.sy           ⬜ 16_mulc.sy
⬜ 17_div.sy           ⬜ 18_divc.sy          ⬜ 19_mod.sy
⬜ 20_rem.sy           ⬜ 29_long_line.sy     ⬜ 35_op_priority1.sy
⬜ 36_op_priority2.sy  ⬜ 37_op_priority3.sy  ⬜ 38_op_priority4.sy
⬜ 39_op_priority5.sy  ⬜ 40_unary_op.sy      ⬜ 41_unary_op2.sy
⬜ 44_stmt_expr.sy     ⬜ 46_hex_defn.sy      ⬜ 47_hex_oct_add.sy
⬜ 48_assign_complex_expr.sy
```

---

## 阶段 2: 变量与作用域 ⬜ (0/13 测试通过)

### 待实现功能
- [ ] **变量定义**
  - [ ] 局部变量定义 (int x;) (01_var_defn2.sy, 02_var_defn3.sy)
  - [ ] 变量初始化 (int x = 1;) (01_var_defn2.sy, 02_var_defn3.sy)
  - [ ] 多变量定义 (int a, b, c;) (01_multiple_returns.sy)
- [ ] **常量定义** (const int x = 1;) (06_const_var_defn2.sy, 07_const_var_defn3.sy)
- [ ] **变量访问与赋值**
  - [ ] 变量引用 (x)
  - [ ] 变量赋值 (x = expr;)
- [ ] **栈帧管理**
  - [ ] 栈帧布局设计
  - [ ] 变量到栈偏移的映射
  - [ ] 保存/恢复 fp, ra
- [ ] **块级作用域** (25_scope3~5.sy, 52_scope.sy, 53_scope2.sy, 54_hidden_var.sy)
  - [ ] 作用域嵌套
  - [ ] 变量遮蔽
- [ ] **逻辑赋值** (43_logi_assign.sy)

### 测试文件清单
```
⬜ 01_multiple_returns.sy  ⬜ 01_var_defn2.sy      ⬜ 02_ret_in_block.sy
⬜ 02_var_defn3.sy         ⬜ 06_const_var_defn2.sy ⬜ 07_const_var_defn3.sy
⬜ 25_scope3.sy            ⬜ 26_scope4.sy         ⬜ 27_scope5.sy
⬜ 43_logi_assign.sy       ⬜ 52_scope.sy          ⬜ 53_scope2.sy
⬜ 54_hidden_var.sy
```

---

## 阶段 3: 控制流 ⬜ (0/15 测试通过)

### 待实现功能
- [ ] **if-else 语句**
  - [ ] 简单 if (03_branch.sy, 21_if_test2~5.sy)
  - [ ] if-else (03_branch.sy, 21_if_test2~5.sy)
  - [ ] 嵌套 if-else (33_multi_branch.sy)
- [ ] **while 循环**
  - [ ] 基本 while (26_while_test1~3.sy)
  - [ ] 嵌套循环 (34_multi_loop.sy)
  - [ ] while + if 组合 (31_while_if_test1~3.sy, 33_while_if_test3.sy)
- [ ] **break 语句** (04_break_continue.sy)
- [ ] **continue 语句** (04_break_continue.sy, 30_continue.sy)
- [ ] **标签管理**
  - [ ] 自动生成唯一标签
  - [ ] break/continue 目标标签栈

### 测试文件清单
```
⬜ 03_branch.sy        ⬜ 04_break_continue.sy  ⬜ 21_if_test2.sy
⬜ 22_if_test3.sy      ⬜ 23_if_test4.sy        ⬜ 24_if_test5.sy
⬜ 26_while_test1.sy   ⬜ 26_while_test2.sy     ⬜ 26_while_test3.sy
⬜ 30_continue.sy      ⬜ 31_while_if_test1.sy  ⬜ 32_while_if_test2.sy
⬜ 33_multi_branch.sy  ⬜ 33_while_if_test3.sy  ⬜ 34_multi_loop.sy
```

---

## 阶段 4: 函数调用 ⬜ (0/5 测试通过)

### 待实现功能
- [ ] **函数参数**
  - [ ] 参数定义 (int func(int a, int b))
  - [ ] 参数命名处理 (05_param_name.sy)
- [ ] **函数调用**
  - [ ] 无参调用 (09_func_defn.sy)
  - [ ] 带参调用 (10_var_defn_func.sy, 25_while_if.sy)
  - [ ] 参数传递 (a0-a7 寄存器)
- [ ] **ABI 遵守**
  - [ ] 调用者保存寄存器 (t0-t6)
  - [ ] 被调用者保存寄存器 (s0-s11)
  - [ ] 返回地址 (ra) 保存与恢复
- [ ] **多函数定义** (06_func_name.sy)
- [ ] **函数作用域** (25_while_if.sy)

### 测试文件清单
```
⬜ 05_param_name.sy   ⬜ 06_func_name.sy    ⬜ 09_func_defn.sy
⬜ 10_var_defn_func.sy ⬜ 25_while_if.sy
```

---

## 阶段 5: 复杂度危机与 IR 引入 ⬜ (0/4 测试通过)

### 待实现功能
- [ ] **短路求值实现**
  - [ ] 逻辑与 (&&) 短路 (50_short_circuit.sy, 51_short_circuit3.sy)
  - [ ] 逻辑或 (||) 短路 (50_short_circuit.sy, 51_short_circuit3.sy)
- [ ] **副作用处理** (28_side_effect2.sy)
  - [ ] 函数调用副作用
  - [ ] 自增自减副作用
- [ ] **复杂算法编译** (37_dct.sy)
  - [ ] DCT 算法
- [ ] **IR 设计**
  - [ ] 三地址码定义
  - [ ] AST 转 IR
  - [ ] IR 转汇编

### 测试文件清单
```
⬜ 28_side_effect2.sy  ⬜ 37_dct.sy  ⬜ 50_short_circuit.sy  ⬜ 51_short_circuit3.sy
```

---

## 阶段 6: 复杂算法综合测试 ⬜ (0/33 测试通过)

### 待实现功能
- [ ] **图算法**
  - [ ] BFS (09_BFS.sy)
  - [ ] DFS (10_DFS.sy)
  - [ ] Prim 最小生成树 (18_prim.sy)
  - [ ] 并查集 (12_DSU.sy, 21_union_find.sy)
  - [ ] 完全连通分量 (71_full_conn.sy)
  - [ ] Dijkstra 最短路 (70_dijkstra.sy)
  - [ ] 最大流 (75_max_flow.sy)
- [ ] **树算法**
  - [ ] 二叉搜索树 (11_BST.sy)
  - [ ] LCA 最近公共祖先 (13_LCA.sy)
- [ ] **排序算法**
  - [ ] 多种排序实现 (20_sort.sy)
- [ ] **字符串算法**
  - [ ] JSON 解析 (23_json.sy)
  - [ ] KMP 匹配 (74_kmp.sy)
  - [ ] 子串处理 (77_substr.sy)
- [ ] **动态规划** (14_dp.sy, 65_color.sy)
- [ ] **搜索算法**
  - [ ] 第K小元素 (16_k_smallest.sy)
  - [ ] 最大团 (17_maximal_clique.sy)
  - [ ] N皇后 (76_n_queens.sy)
  - [ ] 通用搜索 (19_search.sy)

### 测试文件清单
```
图算法 (7):   09_BFS.sy  10_DFS.sy  12_DSU.sy  18_prim.sy  21_union_find.sy
              70_dijkstra.sy  71_full_conn.sy  75_max_flow.sy
树算法 (2):   11_BST.sy  13_LCA.sy
排序 (1):     20_sort.sy
搜索 (4):     16_k_smallest.sy  17_maximal_clique.sy  19_search.sy  76_n_queens.sy
字符串 (3):   23_json.sy  74_kmp.sy  77_substr.sy
DP (2):       14_dp.sy  65_color.sy
其他 (14):    22_matrix_multiply.sy  36_rotate.sy  38_light2d.sy  63_big_int_mul.sy
              64_calculator.sy  66_exgcd.sy  67_reverse_output.sy  69_expr_eval.sy
              72_hanoi.sy  15_graph_coloring.sy  (还有4个未列出)
```

---

## 阶段 7: 高级特性 ⬜ (0/41 测试通过)

### 7.1 数组支持 ⬜ (0/11 测试通过)

#### 待实现功能
- [ ] **数组定义**
  - [ ] 一维数组 (int a[10];)
  - [ ] 多维数组 (int a[10][10];) (03_arr_defn2.sy, 04_arr_defn3.sy, 05_arr_defn4.sy)
  - [ ] 常量数组 (const int a[10];) (08_const_array_defn.sy)
  - [ ] 全局数组初始化 (08_global_arr_init.sy)
- [ ] **数组初始化**
  - [ ] 多维数组初始化 (07_arr_init_nd.sy)
- [ ] **数组访问**
  - [ ] 一维数组访问 (a[i])
  - [ ] 多维数组访问 (a[i][j]) (30_many_dimensions.sy)
  - [ ] 多级数组访问 (31_many_indirections.sy)
- [ ] **数组作为参数**
  - [ ] 数组传参 (int func(int a[]))
  - [ ] 多维数组传参

#### 测试文件
```
⬜ 03_arr_defn2.sy  ⬜ 04_arr_defn3.sy  ⬜ 05_arr_defn4.sy  ⬜ 07_arr_init_nd.sy
⬜ 08_const_array_defn.sy  ⬜ 08_global_arr_init.sy  ⬜ 24_array_only.sy
⬜ 30_many_dimensions.sy  ⬜ 31_many_indirections.sy  ⬜ 34_arr_expr_len.sy
⬜ 55_sort_test1~7.sy
```

### 7.2 浮点数支持 ⬜ (0/6 测试通过)

#### 待实现功能
- [ ] **浮点类型**
  - [ ] float 关键字识别
  - [ ] 浮点常量解析 (3.14, .5, 1e-6) (95_float.sy)
  - [ ] 十六进制浮点常量 (0x1.921fb6p+1)
- [ ] **浮点运算**
  - [ ] 基本四则运算
  - [ ] 浮点比较
- [ ] **浮点函数**
  - [ ] 浮点参数传递 (fa0-fa7) (39_fp_params.sy)
  - [ ] 浮点返回值
- [ ] **矩阵运算** (96_matrix_add.sy, 97_matrix_sub.sy, 98_matrix_mul.sy, 99_matrix_tran.sy)

#### 测试文件
```
⬜ 39_fp_params.sy  ⬜ 95_float.sy  ⬜ 96_matrix_add.sy  ⬜ 97_matrix_sub.sy
⬜ 98_matrix_mul.sy  ⬜ 99_matrix_tran.sy
```

### 7.3 压力测试 ⬜ (0/24 测试通过)

#### 待实现功能
- [ ] **超长变量名** (79_var_name.sy)
- [ ] **混乱格式处理** (80_chaos_token.sy, 81_skip_spaces.sy)
- [ ] **超长代码/函数/数组** (82_long_func.sy, 83_long_array.sy, 84_long_array2.sy, 85_long_code.sy, 86_long_code2.sy)
- [ ] **大量参数** (87_many_params.sy, 88_many_params2.sy, 32_many_params3.sy)
- [ ] **大量全局变量** (89_many_globals.sy)
- [ ] **大量局部变量** (90_many_locals.sy, 91_many_locals2.sy)
- [ ] **寄存器分配压力测试** (92_register_alloc.sy)
- [ ] **嵌套调用** (93_nested_calls.sy)
- [ ] **嵌套循环** (94_nested_loops.sy)
- [ ] **副作用测试** (78_side_effect.sy, 28_side_effect2.sy)

---

## 总体进度统计

| 阶段 | 测试总数 | 通过数 | 进度 |
|------|----------|--------|------|
| 阶段0: 热身与修复 | 4 | 4 | ✅ 100% |
| 阶段1: 表达式 | 22 | 0 | ⬜ 0% |
| 阶段2: 变量与作用域 | 13 | 0 | ⬜ 0% |
| 阶段3: 控制流 | 15 | 0 | ⬜ 0% |
| 阶段4: 函数调用 | 5 | 0 | ⬜ 0% |
| 阶段5: IR引入 | 4 | 0 | ⬜ 0% |
| 阶段6: 复杂算法 | 33 | 0 | ⬜ 0% |
| 阶段7: 高级特性 | 41 | 0 | ⬜ 0% |
| **总计** | **137** | **4** | **2.9%** |

---

## 下一步工作重点

### 🔥 优先级 1: 阶段1 表达式 (基础功能)
1. 实现四则运算语法分析
2. 实现表达式代码生成（寄存器分配）
3. 支持运算符优先级
4. 支持一元运算

### 📌 优先级 2: 阶段0 已知问题修复
1. 修复 return 语句数值解析
2. 修复关键字识别长度计算

### 📝 优先级 3: 阶段2 变量支持
1. 实现变量定义和访问
2. 设计栈帧布局
3. 支持块级作用域

---

*最后更新: 2026-03-01*
