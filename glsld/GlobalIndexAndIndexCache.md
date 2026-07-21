# GlobalIndex、IndexCache 与 Workspace 后台索引实现说明

本文基于当前工作区中的实际代码编写，主要覆盖以下文件：

- `Sources/Base/Index/GlobalIndex.hpp`
- `Sources/Base/Index/GlobalIndex.cpp`
- `Sources/Base/Index/IndexCache.hpp`
- `Sources/Base/Index/IndexCache.cpp`
- `Sources/Server/Workspace.hpp`
- `Sources/Server/Workspace.cpp`
- `Sources/Server/Workspace.inl`
- `Sources/Server/FunctionProviders.cpp`
- `Sources/Server/LspServer.cpp`
- `Sources/Base/FileSystem/Source.hpp`
- `Sources/Base/FileSystem/Source.cpp`

本文先解释设计目标和每个函数、成员的作用，最后单独列出当前实现尚未完成的接入点与值得修正的问题。最后一节描述的是当前源码事实，不是对设计意图的否定。

## 1. 总体目标

原来的引用查找只遍历当前 `Document::bindings`，因此只能看到当前翻译单元内的绑定关系。加入全局索引后，每个已经解析的顶层文档都会向 `GlobalIndex` 提交自己的引用贡献，查询阶段再以符号定义位置为键，跨文档返回全部引用位置。

后台索引进一步解决两个问题：

1. 没有在编辑器中打开的 shader 文件也需要进入索引。
2. LSP 重启后不应每次都从零解析整个工作区。

完整数据流如下：

```text
工作区磁盘文件
    ↓ 扫描候选扩展名
后台解析 Document
    ↓ 读取 Document::bindings
Contribution 列表
    ├──→ GlobalIndex：供 references/rename 实时查询
    └──→ DiskIndexRecord：转换为可持久化位置
                ↓
          MessagePack 缓存文件
```

缓存恢复时走相反方向：

```text
MessagePack 缓存
    ↓ IndexCache::Load
DiskIndexRecord
    ↓ 检查根文件和全部依赖的 size/mtime
StoredContribution
    ↓ 通过 Workspace::InternSource 恢复 SourceLocation
Contribution
    ↓ GlobalIndex::RestoreDocument
GlobalIndex
```

## 2. 为什么索引使用 SourceLocation 作为符号身份

不同顶层文档解析同一个 include 时，会生成不同的 `SymbolInfo` 对象，因此不能用 `SymbolInfo*` 作为跨文档键。例如 `A.frag` 和 `B.frag` 都包含 `Common.glsl` 时，两个解析结果中的 `SharedValue` 符号对象地址不同，但它们的定义位置都指向 `Common.glsl` 的同一行、同一列。

`GlobalIndex` 因此使用定义的 `SourceLocation` 作为逻辑身份：

```text
definition = Common.glsl:10:5
reference  = A.frag:20:9
```

这样只要 `SourceFile` 的 URI/文件名规范化结果一致，相同物理定义就能合并到同一个全局键中。

## 3. Contribution

`Contribution` 是全局引用索引的最小输入单元：

```cpp
struct Contribution { SourceLocation definition; SourceLocation reference; };
```

字段含义：

| 字段 | 含义 |
| --- | --- |
| `definition` | 被引用符号的定义位置，也是 `GlobalIndex::references_` 的第一层键 |
| `reference` | 当前绑定发生的位置，也是第二层引用表的键 |

一条贡献表示一条有向边：`reference → definition`。虽然自然语言通常说“引用指向定义”，索引存储方向是“定义查全部引用”，因此数据结构按 `definition → reference` 组织。

同一个引用位置可能绑定到 `SymbolList`，例如重载解析或存在多个候选符号时，`CollectContributions` 会为列表中的每个候选定义分别生成一条贡献。

## 4. GlobalIndex

### 4.1 内部数据结构

#### `RefCountMap`

```cpp
using RefCountMap = ankerl::unordered_dense::map<SourceLocation, std::size_t, LocationHash>;
```

它表示某个定义下的引用位置及其贡献计数：

```text
reference location → 有多少个顶层文档贡献了这条关系
```

计数不能简单替换为集合。假设 `A.frag` 和 `B.frag` 都包含 `Common.glsl`，预处理后两个顶层文档可能都贡献同一条位于 include 内部的绑定关系。如果关闭或更新 `A.frag`，只能把计数从 2 减到 1，不能直接删除该引用，因为 `B.frag` 仍然贡献它。

#### `references_`

```cpp
ankerl::unordered_dense::map<SourceLocation, RefCountMap, LocationHash> references_;
```

这是查询方向的主索引：

```text
定义位置 → 引用位置 → 贡献计数
```

`GetReferences` 只读取这个结构，所以查询复杂度主要与目标定义的引用数量有关，而不是与工作区全部 binding 数量有关。

#### `document_contributions_`

```cpp
StringHeteroHashMap<std::vector<Contribution>> document_contributions_;
```

这是更新和删除方向的反向记录：

```text
顶层文档 URI → 该文档上一次提交的全部贡献
```

如果没有它，文档发生修改时只能添加新引用，无法知道旧版本曾经添加过哪些计数，也就无法撤销已经消失的引用。

#### `mutex_`

```cpp
mutable std::shared_mutex mutex_;
```

它保护 `references_` 和 `document_contributions_` 的一致性。

- 查询使用 `std::shared_lock`，允许多个引用查询并行执行。
- 更新、恢复、删除和清空使用独占锁。
- `ApplyContributions` 在一次独占锁内完成旧贡献撤销和新贡献安装，从 `GlobalIndex` 自身的角度看属于原子替换。

### 4.2 `GlobalIndex::CollectContributions`

作用：从已经完成符号绑定的 `Document` 中提取全部引用边，但不修改全局索引。

处理过程：

1. 按 `document.bindings.size()` 预留容量。
2. 遍历 `Document::bindings`，其中 map 的键是引用位置，值是 `SymbolReference`。
3. 如果值是单个 `const SymbolInfo*`，生成一条贡献。
4. 如果值是 `SymbolList`，对每个候选符号生成一条贡献。
5. 跳过空符号指针和没有 `SourceFile` 的无效位置。

该函数返回的列表暂时可能包含重复项。排序和去重集中放在 `ApplyContributions` 中进行，因此普通解析和缓存恢复共享相同的规范化逻辑。

它被声明为 public static，是因为后台磁盘索引需要同时获得运行时贡献和可序列化贡献：运行时贡献交给 `GlobalIndex`，同一批贡献又会被转换成 `StoredContribution` 写入 `DiskIndexRecord`。

### 4.3 `GlobalIndex::IndexDocument`

作用：把一个正常解析完成的 `Document` 写入索引。

它只负责组合两个步骤：

```text
CollectContributions(document) → ApplyContributions(uri, contributions)
```

调用者不需要知道排序、去重、引用计数和旧版本撤销等内部细节。

### 4.4 `GlobalIndex::RestoreDocument`

作用：把已经从缓存恢复成运行时 `Contribution` 的数据写入索引。

它和 `IndexDocument` 的区别只有输入来源：

- `IndexDocument` 的输入来自 `Document::bindings`。
- `RestoreDocument` 的输入来自磁盘缓存反序列化。

二者最终都调用私有的 `ApplyContributions`，所以缓存恢复不会绕开排序、去重和引用计数不变量。

### 4.5 `GlobalIndex::ApplyContributions`

这是索引更新的核心函数，保持 private 是合理的，因为外部调用者不应依赖内部更新顺序。

执行步骤如下。

#### 第一步：排序

先按 `definition` 排序，定义相同时再按 `reference` 排序。比较使用 `<=>`，每一级位置比较只执行一次。

排序的目的不是为了查询，而是为了让完全相同的贡献相邻，以便下一步线性去重。

#### 第二步：去重

`std::ranges::unique` 删除同一文档内部完全相同的 `(definition, reference)` 对。

去重非常重要：引用计数表示“多少个文档贡献了这条边”，而不是“同一个文档内部重复收集了多少次”。同一文档最多应该为同一条边贡献 1。

#### 第三步：获取独占锁

排序和去重在锁外进行，避免 CPU 工作阻塞并发查询。只有真正修改共享表时才获取 `mutex_`。

#### 第四步：撤销旧版本

通过 URI 查询 `document_contributions_`。如果文档以前已经索引过，调用 `WithdrawOldContributionLocked` 撤销旧版本贡献。

#### 第五步：安装新版本

对每条新贡献执行引用计数递增：

```text
++references_[definition][reference]
```

#### 第六步：保存文档贡献

用新列表替换 `document_contributions_[uri]`，供下一次更新或删除时撤销。

### 4.6 `GlobalIndex::WithdrawOldContributionLocked`

作用：撤销一组已经被某个文档提交过的贡献。

函数名中的 `Locked` 表示它不会自行加锁，调用者必须已经持有 `GlobalIndex::mutex_` 的独占锁。目前调用点是 `RemoveDocument` 和 `ApplyContributions`。

对每条贡献：

1. 找到对应定义桶。
2. 找到对应引用计数。
3. 计数大于 1 时递减。
4. 计数等于 1 时删除引用位置。
5. 定义下已经没有任何引用时删除整个定义桶。

代码对找不到定义或引用的情况直接跳过，这提供了一定的防御性，不会因为索引已经部分清理而崩溃。

### 4.7 `GlobalIndex::RemoveDocument`

作用：彻底删除某个顶层文档的全部索引贡献。

它先在 `document_contributions_` 中找到该 URI，调用 `WithdrawOldContributionLocked` 撤销计数，再删除文档贡献记录。

这个操作适用于文件被删除、工作区移除或者明确不再保留该磁盘文档索引的情况。编辑器关闭文件不一定等于删除索引，因为关闭后通常还应恢复该文件的磁盘版本；这也是 `Workspace::CloseDocument` 没有直接调用它的原因。

### 4.8 `GlobalIndex::GetReferences`

作用：根据定义位置返回全部唯一引用位置。

执行过程：

1. 获取共享锁。
2. 在 `references_` 中找到定义。
3. 把计数大于 0 的引用位置复制到结果 vector。
4. 释放共享锁。
5. 在锁外排序结果。

排序使用 `SourceLocation::<=>`，主要作用是让 LSP 返回结果和测试结果更加稳定。因为排序发生在锁外，它不会延长写操作的等待时间。

### 4.9 `GlobalIndex::DumpStatus`

作用：输出索引统计信息，包括：

- 定义桶数量。
- 唯一 `(definition, reference)` 关系数量。
- 所有文档贡献数量。
- 已索引顶层文档数量。

“唯一引用数量”和“文档贡献数量”不同。例如同一 include 绑定被三个顶层文档重复贡献时，前者记为 1，后者记为 3。

### 4.10 `GlobalIndex::Clear`

作用：在独占锁下清空查询索引和文档贡献表。典型用途是工作区完全切换、缓存版本整体失效或服务器准备重建全部索引。

## 5. TypeMemberIndex

虽然它和全局引用索引放在同一个文件中，但用途不同。它服务于成员补全，例如输入 `object.` 后根据类型名查找字段。

### 5.1 `type_members_`

保存 `type name → SymbolList`，返回的 `SymbolInfo*` 指向所属 `Document` 的符号对象，因此它不能直接磁盘持久化，也不能在文档销毁后继续保留。

### 5.2 `document_typenames_`

保存 `document URI → 本文档贡献的类型名`，用于文档更新或关闭时撤销 `type_members_` 中的条目。

### 5.3 `IndexDocument`

递归扫描符号作用域，发现 struct/interface 且存在内部作用域时，把内部作用域的符号作为成员列表记录下来。

### 5.4 `RemoveDocument`

读取该文档登记的类型名，删除对应成员列表，再删除文档到类型名的记录。

### 5.5 `GetMembers`

使用共享锁按类型名返回 `SymbolList` 的副本。

### 5.6 `Clear`

删除全部类型成员和文档类型名记录。

## 6. CompletionTrie

`CompletionTrie` 是按符号名字构建的前缀树。

- `Build`：清空旧树并递归收集文档全部 scope。
- `Insert`：逐字符创建 Trie 节点，在完整名字终点保存符号指针。
- `Search`：先沿 prefix 找到对应节点，再递归收集该节点下的所有符号。
- `Clear`：重新创建空根节点。
- `CollectScope`：遍历当前 scope 的符号和所有子 scope。

当前代码没有发现它的外部调用点。它还会把所有子作用域符号放在同一棵树里，未表达光标位置、作用域可见性和遮蔽关系，因此目前不适合直接替代 `Scope::GetVisibleSymbols`。

## 7. IndexCache 的磁盘数据模型

### 7.1 为什么不能直接把 SourceLocation 内存写到磁盘

`SourceLocation` 内部包含 `const SourceFile*` 和缓存哈希。指针只在当前进程中有效，LSP 重启后地址没有意义。因此磁盘格式必须把位置转换成稳定的逻辑字段：URI、行、列。

### 7.2 `StoredLocation`

| 字段 | 作用 |
| --- | --- |
| `uri` | 文件的规范化 URI，用于下次启动时重新 intern `SourceFile` |
| `line` | 解析器的一基行号 |
| `column` | 解析器的一基列号 |

加载后，`Workspace::RestoreLocation` 调用 `InternSource(uri)` 获得当前进程的 `SourceFile*`，再重新构造 `SourceLocation`，其缓存哈希也会由构造函数重新计算。

### 7.3 `StoredContribution`

磁盘版本的贡献边，包含一个定义位置和一个引用位置。它和运行时 `Contribution` 结构相似，但位置类型不同，从而明确隔离进程内指针和可持久化数据。

### 7.4 `IndexedFileStamp`

| 字段 | 作用 |
| --- | --- |
| `uri` | 被验证文件的 URI |
| `size` | 建立索引时的文件大小 |
| `write_time` | 建立索引时 `last_write_time` 的计数值 |

它用于快速判断缓存是否过期，不需要重新读取和哈希整个文件。

### 7.5 `DiskIndexRecord`

一条记录代表一个顶层文档的磁盘索引：

| 字段 | 作用 |
| --- | --- |
| `owner_uri` | 顶层文档 URI，也是 `GlobalIndex::document_contributions_` 的 owner key |
| `dependencies` | 解析该文档时遇到的 include URI，用于恢复正向/反向依赖图 |
| `stamps` | owner 文件和全部依赖文件的磁盘快照 |
| `contributions` | 该顶层文档贡献的可持久化引用边 |

缓存记录必须验证 owner 和全部 include。只验证顶层文件是不够的，因为 include 内容变化也可能改变符号绑定结果。

### 7.6 `DiskIndexSnapshot`

代表整个缓存文件：

| 字段 | 作用 |
| --- | --- |
| `schema_version` | 磁盘格式版本，不兼容格式修改时递增 |
| `cache_key` | 调用者提供的语义版本键，应覆盖解析器版本、配置、include 目录和内置数据库版本等因素 |
| `records` | 所有顶层文档记录 |

`schema_version` 解决“文件结构变了”的问题，`cache_key` 解决“结构没变但绑定语义变了”的问题。

## 8. IndexCache 函数

### 8.1 `SerializeLocation`

把 `StoredLocation` 转成 JSON object，字段名为 `uri`、`line` 和 `column`。这是内部辅助函数，不暴露到头文件。

### 8.2 `DeserializeLocation`

执行相反转换，从 JSON object 读取 URI、行和列并构造 `StoredLocation`。

### 8.3 `SerializeRecord`

把一条 `DiskIndexRecord` 转成 JSON：

1. 写入 owner URI。
2. 直接写入 dependencies 数组。
3. 逐项写入 stamps。
4. 逐项写入 definition/reference contribution。

这里的 JSON 只是中间对象，最终文件不是文本 JSON，而是 MessagePack 二进制。

### 8.4 `DeserializeRecord`

从 JSON 中恢复一条 `DiskIndexRecord`，包括依赖列表、文件 stamp 和所有贡献位置。

### 8.5 `IndexCache::Load`

执行流程：

1. 调用 `LoadBinary` 读取整个缓存文件。
2. 调用 `nlohmann::json::from_msgpack` 解码 MessagePack。
3. 读取 `schemaVersion` 和 `cacheKey`。
4. 如果 schema 或 key 不匹配，返回空值，调用者会重新扫描工作区。
5. 逐条调用 `DeserializeRecord` 恢复记录。

返回 `std::optional<DiskIndexSnapshot>`，空值表示缓存不可用而不是索引失败。缓存失效只影响启动速度，不应影响最终正确性。

### 8.6 `IndexCache::Save`

执行流程：

1. 构造顶层 JSON object。
2. 序列化全部记录。
3. 使用 `nlohmann::json::to_msgpack` 生成二进制字节。
4. 创建缓存父目录。
5. 先写入 `.temp` 临时文件。
6. 删除旧缓存文件。
7. 把临时文件重命名为正式缓存文件。

临时文件方案避免直接覆盖正式文件时留下半写入内容。即使最终替换失败，最坏结果也是下次启动重新索引。

### 8.7 `IndexCache::CaptureStamp`

根据 URI 转换出本地路径，然后读取文件大小和最后修改时间。任一文件系统操作失败都会返回空值。

### 8.8 `IndexCache::IsFresh`

逐项重新捕获 record 中每个文件的 stamp，并和缓存值比较。以下任一情况都会认为过期：

- stamps 为空。
- 文件不存在或无法读取 metadata。
- 文件大小变化。
- 最后修改时间变化。

这种策略速度快，但不属于内容级强校验：如果外部工具修改内容后刻意恢复相同大小和 mtime，缓存无法发现变化。正常编辑器保存流程一般不会出现这种情况。

## 9. Source 文件读取改动

为了让源码和缓存共享文件读取基础设施，`Source.hpp/.cpp` 增加了两个返回 `std::expected` 的函数。

### 9.1 `LoadBinary`

以二进制模式读取完整文件，成功时返回 `std::vector<std::byte>`，失败时通过 `std::unexpected<std::string>` 返回错误描述。`IndexCache::Load` 使用它读取 MessagePack。

### 9.2 `LoadSource`

先调用 `LoadBinary`，再把字节转换为 `std::string`。原来返回 `(source, error)` pair 的调用点已经改为检查 `expected::has_value()`。

这使错误路径更明确：成功结果和错误字符串不会同时存在，也不会依赖空字符串表达失败。

## 10. Workspace 中原有索引相关成员

| 成员 | 作用 |
| --- | --- |
| `documents_` | 保存编辑器当前文档的完整 `Document` 快照，保证 AST 和 `SymbolInfo*` 生命周期 |
| `source_table_` | 把文件名/URI intern 成稳定 `SourceFile`，所有 `SourceLocation` 都依赖它 |
| `include_loader_` | 解析和缓存 include 文件 |
| `include_dirs_` | include 搜索目录 |
| `forward_dependencies_` | 顶层文档 URI → 它包含的依赖 URI |
| `reverse_dependencies_` | include URI → 受它影响的顶层文档 URI 集合 |
| `dependency_mutex_` | 保护正向和反向依赖图 |
| `document_mutex_` | 保护 `documents_` 快照表 |
| `variant_mutex_` | 保护共享 variant 和每文件 variant |
| `global_index_` | 跨文档引用索引 |
| `type_member_index_` | 类型名到成员符号的内存索引 |
| `index_mutex_` | 在 Workspace 层协调引用索引和类型成员索引的联合更新 |

## 11. Workspace.hpp 新增公开函数

### 11.1 `StartBackgroundIndex`

参数：

- `roots`：需要递归扫描的工作区根目录。
- `cache_path`：MessagePack 缓存文件位置。
- `cache_key`：当前缓存语义版本。

函数先停止旧后台线程，再更新根目录和缓存配置，最后启动 `std::jthread` 执行 `BackgroundIndexLoop`。这允许工作区切换时重新启动索引器。

### 11.2 `StopBackgroundIndex`

如果后台线程存在，则请求 stop、唤醒条件变量并 join。join 保证函数返回时后台线程不再访问 `Workspace` 成员。

### 11.3 `MarkDocumentOpen`

在 LSP 文档打开、内存版本即将覆盖磁盘版本时调用。

它做三件事：

1. 把 URI 加入 `open_document_uris_`。
2. 增加该 URI 的 generation，使正在进行的旧磁盘解析结果失效。
3. 删除尚未执行的 pending path。

队列中可能还残留 URI，但后台线程弹出后找不到 pending path，就不会形成有效任务。

### 11.4 `CloseDocument`

关闭编辑器内存文档，但不立即删除全局引用索引。

执行步骤：

1. 删除该 URI 的 per-file variant。
2. 从 `TypeMemberIndex` 删除会随 Document 销毁而悬空的 `SymbolInfo*`。
3. 从 `documents_` 删除内存快照。
4. 从 `open_document_uris_` 删除 URI，并推进 generation。
5. 调度一次磁盘解析，让磁盘内容最终原子替换内存索引。

在磁盘任务完成前暂时保留内存版全局引用，避免关闭瞬间出现空索引窗口。

### 11.5 `ScheduleDiskIndex`

接受本地文件路径，规范化后生成 URI，并把任务加入后台队列。

主要逻辑：

- 打开的文档不进行磁盘索引，防止磁盘旧内容覆盖未保存内容。
- 每次调度推进 generation。
- `pending_disk_paths_` 保存 URI 对应的最新路径。
- `queued_disk_uris_` 保证队列中同一 URI 最多出现一次。
- 最后唤醒后台线程。

如果同一 URI 在排队期间再次变化，不会重复插入队列，但 path 和 generation 会更新为最新值。

### 11.6 `ScheduleDiskIndexByUri`

它是 URI 版本的便捷入口，预期行为是把 URI 转成本地路径后调用 `ScheduleDiskIndex`。

### 11.7 后台函数与普通文档函数的关系

`UpdateDocument` 处理编辑器内存文本并保留完整 `Document`；`ProcessDiskIndexTask` 处理磁盘文本，只保留引用贡献、依赖和 stamp，不保存 AST，因此后台索引不会让所有工作区文档长期占用完整 AST 内存。

## 12. Workspace.hpp 新增私有函数

### 12.1 `BackgroundIndexLoop`

后台线程主循环。启动时加载缓存并协调工作区，之后在“有任务”和“五秒周期到期”两种事件之间等待。

每次循环最多取出一个 URI 任务，解析后检查是否需要周期扫描，最后尝试刷新缓存。

### 12.2 `LoadBackgroundCache`

加载缓存快照，对每条 record 执行 freshness 检查。有效记录会：

1. 把 `StoredLocation` 恢复为当前进程的 `SourceLocation`。
2. 调用 `GlobalIndex::RestoreDocument` 恢复引用索引。
3. 调用 `UpdateDependencies` 恢复依赖图。
4. 保存到 `disk_index_records_`，供后续 freshness 检查和重新写盘。

如果该 URI 已经被标记为打开，则跳过缓存记录，防止磁盘缓存覆盖编辑器内容。

### 12.3 `ReconcileWorkspace`

负责让“磁盘上的实际候选文件”与“当前缓存记录”保持一致。

对每个候选文件：

- 生成规范化 URI。
- 如果没有 record 或 record 已过期，则调度磁盘索引。
- 如果 record 仍然新鲜，则不重复解析。

然后检查 record 中已经不存在于候选集合的 owner：

- 打开的文档暂不删除。
- 未打开且已经从磁盘候选集合消失的文档，从 `disk_index_records_`、`GlobalIndex` 和依赖图中删除。
- 标记缓存 dirty，等待重新写盘。

### 12.4 `ProcessDiskIndexTask`

这是后台单文件解析和提交函数。

详细步骤：

1. 从磁盘读取源码。
2. 创建临时 `Document`，版本固定为 0。
3. 通过 `ProcessSource` 执行 Lexer、Parser、SymbolLinker、TypeResolver 和 MacroBinder。
4. 从 bindings 收集运行时 Contribution。
5. 创建 `DiskIndexRecord`，保存 owner 和 dependencies。
6. 把运行时位置转换为 `StoredLocation`。
7. 对 owner URI 和全部 dependency URI 排序、去重并捕获 stamp。
8. 获取后台状态锁并检查 generation。
9. 如果 generation 已变化或者文档已经打开，丢弃本次结果。
10. 调用 `GlobalIndex::RestoreDocument` 提交引用贡献。
11. 更新依赖图。
12. stamps 完整时保存磁盘 record 并标记缓存 dirty。

临时 `Document` 在函数结束后释放，不会进入 `documents_`。这是后台索引内存开销低于“打开全部文件”的关键原因。

### 12.5 `FlushBackgroundCache`

只有 `background_cache_dirty_` 为 true 时才工作。

它先在锁内复制 `disk_index_records_` 形成快照，并把 dirty 清零，然后在锁外执行磁盘写入。这样 MessagePack 编码和文件 I/O 不会长时间持有后台状态锁。

保存失败时重新设置 dirty，后续循环会重试。

### 12.6 `DiscoverIndexCandidates`

递归遍历 `index_roots_`，跳过无权限目录，并排除以下大型或内部目录：

- `.git`
- `.glsld`
- `.vs`
- `node_modules`
- `vcpkg_installed`

普通文件通过 `IsIndexCandidate` 过滤后，以规范化路径加入结果。

### 12.7 `IsIndexCandidate`

当前识别 16 个扩展名，包括常规 GLSL、图形/计算 stage、mesh/task 和 ray tracing stage，以及 `.inc`。

该过滤决定哪些文件会被当作顶层翻译单元主动解析。被这些顶层文件 include 的依赖即使扩展名不在列表中，也会通过 `Document::dependencies` 参与缓存 freshness 验证。

### 12.8 `UpdateDependencies` 的 span 重载

缓存恢复阶段没有完整 `Document`，只有 `record.dependencies`，因此增加了 `std::span<const std::string>` 重载。它先注销旧依赖，再复制新的正向依赖并重建反向依赖集合。

原来的 `shared_ptr<const Document>` 重载现在只是转发 `document->dependencies`。

## 13. Workspace.hpp 新增成员变量

### 13.1 静态配置

| 成员 | 作用 |
| --- | --- |
| `index_roots_` | 后台递归扫描的工作区根目录列表 |
| `index_cache_path_` | 当前 MessagePack 缓存文件路径 |
| `index_cache_key_` | 当前缓存语义版本键 |

这些成员由 `StartBackgroundIndex` 设置，后台线程读取。

### 13.2 已提交的磁盘状态

| 成员 | 作用 |
| --- | --- |
| `disk_index_records_` | URI → 已成功建立且具有完整 stamps 的磁盘索引记录 |

它与 `GlobalIndex` 有意保持分离：`GlobalIndex` 当前可能包含未保存的编辑器内容，而 `disk_index_records_` 必须始终代表磁盘内容，才能安全写入跨进程缓存。

### 13.3 Pending 与队列去重

| 成员 | 作用 |
| --- | --- |
| `pending_disk_paths_` | URI → 下一次后台任务应读取的最新规范化路径 |
| `queued_disk_uris_` | 当前已经存在于队列中的 URI，防止重复排队 |
| `disk_index_queue_` | 等待处理的 URI FIFO 队列 |

路径和 URI 分开存储的原因是队列只需要轻量 URI，而真正执行任务时仍需本地路径读取文件。

### 13.4 并发代际控制

| 成员 | 作用 |
| --- | --- |
| `index_generations_` | URI → 当前磁盘索引任务代际 |
| `open_document_uris_` | 当前被编辑器内存内容覆盖的 URI 集合 |

generation 解决后台结果过期问题。例如：

```text
generation 10 开始解析
    ↓
文件再次变化，generation 增加到 11
    ↓
generation 10 完成
    ↓
提交前发现 10 != 11，丢弃旧结果
```

打开文档也会增加 generation，因此已经在运行的磁盘任务不能覆盖后来打开的内存文档。

### 13.5 线程同步

| 成员 | 作用 |
| --- | --- |
| `background_index_mutex_` | 保护 roots/cache 配置以外的大部分后台可变状态、队列、generation、open 集合和 record 表 |
| `background_index_condition_` | 后台线程空闲时等待新任务或周期协调时间到达 |
| `background_index_thread_` | 执行缓存加载、工作区扫描、磁盘解析和缓存刷新的 `std::jthread` |
| `background_cache_dirty_` | 表示内存中的磁盘 record 与缓存文件不同，需要重新保存 |

`std::jthread` 提供 stop token，配合显式 `StopBackgroundIndex` 可以安全结束线程。

## 14. 后台索引完整时序

### 14.1 冷启动且没有缓存

```text
StartBackgroundIndex
    ↓
BackgroundIndexLoop
    ↓
LoadBackgroundCache 返回空
    ↓
ReconcileWorkspace 扫描候选文件
    ↓
ScheduleDiskIndex 建立队列
    ↓
逐文件 ProcessDiskIndexTask
    ↓
GlobalIndex + dependency graph + DiskIndexRecord
    ↓
FlushBackgroundCache 写 MessagePack
```

### 14.2 热启动且缓存有效

```text
LoadBackgroundCache
    ↓
IsFresh 验证 owner 和 dependencies
    ↓
RestoreLocation
    ↓
GlobalIndex::RestoreDocument
    ↓
ReconcileWorkspace 只调度新增或过期文件
```

### 14.3 文档打开

预期调用顺序：

```text
MarkDocumentOpen(uri)
    ↓ generation++，阻止磁盘任务提交
UpdateDocument(uri, editor text)
    ↓
GlobalIndex::IndexDocument
```

### 14.4 文档关闭

```text
CloseDocument(uri)
    ↓ 删除 AST/TypeMemberIndex，保留临时全局引用
ScheduleDiskIndexByUri
    ↓
ProcessDiskIndexTask 读取磁盘版本
    ↓
GlobalIndex::RestoreDocument 原子替换内存版贡献
```

### 14.5 include 文件变化

每条顶层 record 的 stamps 包含全部 dependencies。五秒协调时 `IsFresh` 会发现 include stamp 变化，然后重新调度所有依赖它且已经有 record 的顶层文档。解析顶层文档会重新展开 include，并产生新的 binding 和 Contribution。

## 15. 引用查询如何使用 GlobalIndex

`FunctionProviders::GetReferences` 仍然从当前文档光标位置定位 token，并在当前 snapshot 的 `bindings` 中解析出目标 `SymbolInfo`。区别在于它不再遍历当前 snapshot 的全部 binding，而是使用：

```text
symbol->location → global_index.GetReferences
```

`LspServer::HandleReferences` 和 rename handler 都传入 `workspace_.global_index()`。因此：

- references 可以返回其他已索引文档的位置。
- rename 可以生成跨 URI 的 workspace edit。
- include 中定义的符号只要被其他顶层文档解析并贡献，就可以从定义处反查包含者中的引用。

## 16. 当前代码的实际接入状态

当前 `GlobalIndex` 已经接入普通打开文档的引用查询，但后台索引 API 尚未从 `LspServer` 调用：

- `LspServer::HandleInitialized` 当前仍为空。
- `StartBackgroundIndex` 没有外部调用点。
- `MarkDocumentOpen` 没有外部调用点。
- `CloseDocument` 没有外部调用点。
- `ScheduleDiskIndex` 和 `ScheduleDiskIndexByUri` 没有外部调用点。
- `LspServer` 也尚未保存 `rootUri` 或 `workspaceFolders`。

因此当前运行状态是：打开文档会进入 `GlobalIndex`，但新增的后台线程、工作区扫描和磁盘缓存不会启动。要启用完整流程，需要在 initialize/initialized/didOpen/didSave/didClose 中补齐这些调用。

## 17. 当前实现需要注意或修正的地方

本节按照当前源码记录实际风险，适合在正式启用后台索引前逐项处理。

### 17.1 `ScheduleDiskIndexByUri` 的转换方向错误

当前实现调用 `utils::PathToUri(uri)`，但参数已经是 URI。预期应先调用 `utils::UriToPath(uri)`，再把得到的路径传给 `ScheduleDiskIndex`。否则可能生成形如“把 file URI 当作本地路径后再次编码”的错误 URI。

正确语义应是：

```cpp
inline void Workspace::ScheduleDiskIndexByUri(std::string_view uri) { ScheduleDiskIndex(utils::UriToPath(uri)); }
```

### 17.2 `IndexCache::Save` 重命名时临时文件流仍然打开

当前 `std::ofstream stream` 的作用域覆盖到了 `remove/rename`。在 Windows 上，打开的文件句柄通常会导致 rename 失败，因此缓存可能一直保存失败并保持 dirty。

应把写流放入单独作用域，保证析构关闭后再执行替换。

### 17.3 `IndexCache::Load` 没有捕获 MessagePack/JSON 异常

`from_msgpack`、`json.at` 和 `get` 都可能因缓存损坏或字段不匹配抛异常。异常如果逃出 `BackgroundIndexLoop` 的 `std::jthread` 入口，会触发 `std::terminate`。

缓存读取应整体包在 try/catch 中，损坏缓存应被当成 cache miss，而不是服务器致命错误。

### 17.4 普通 `UpdateDocument` 仍然先删除旧索引

当前解析失败或因版本变化取消时会调用 `global_index_.RemoveDocument`，这会删除上一版仍然有效的索引。成功路径也先 Remove 再 Index，查询线程可能在两个独立内部锁之间观察到空窗口。

更合适的行为是：解析失败时保留旧版本；成功时直接调用 `IndexDocument`，让 `ApplyContributions` 在一个独占锁内撤旧换新。

### 17.5 `GetAffectedDocuments` 缺少依赖锁

后台线程会修改 `reverse_dependencies_`，而当前 `GetAffectedDocuments` 未获取 `dependency_mutex_`。启用后台线程后这会成为数据竞争。

### 17.6 `SourceTable::Intern` 在加锁前读取 map

当前 `Intern` 先无锁调用 `sources_.find`，之后才获取独占锁插入。后台解析和编辑器更新可能并发 intern，相同 map 的无锁读与写构成数据竞争。`RemoveByFilename/RemoveByUri` 也没有加锁。

启用后台线程前，应把 `Intern` 的查询和插入都放到锁内，并为 remove 操作增加独占锁。

### 17.7 配置容器与 include 目录的并发访问

`ProcessSource` 会在后台线程读取 `include_dirs_` 和 `shader_configs_`，而配置通知可能在其他线程修改它们。当前这些容器没有统一同步策略。正式并行后需要快照或共享锁，否则存在数据竞争和 span 生命周期风险。

### 17.8 `cache_key` 尚未形成真实语义版本

缓存键应至少考虑：解析器/绑定器版本、内置数据库版本、include dirs、shader config、shared variant 和 per-file variant。当前只有参数接口，没有构造和更新策略。如果这些条件改变但 key 不变，mtime/size 仍然相同的源码可能错误复用旧绑定。

### 17.9 `ReconcileWorkspace` 在后台状态锁内执行文件系统查询

它持有 `background_index_mutex_` 调用 `IndexCache::IsFresh`，后者会执行多次 `file_size` 和 `last_write_time`。依赖很多或网络文件系统较慢时，`MarkDocumentOpen` 和任务调度会被阻塞。更好的方式是在锁内复制 record，在锁外检查 freshness，再回锁验证状态。

### 17.10 候选扩展名大小写敏感

`IsIndexCandidate` 直接比较扩展名，因此 `.GLSL`、`.FRAG` 等不会进入索引。Windows 文件系统通常大小写不敏感，但字符串比较仍然区分大小写。可以先把扩展名转换为小写。

### 17.11 stamps 不完整时旧 record 的处理

`ProcessDiskIndexTask` 在无法捕获任一 stamp 时仍会更新 `GlobalIndex`，但不会替换 `disk_index_records_`。如果该 URI 原来已有旧 record，旧 record 仍可能留在表中并被周期协调反复判定为过期。需要明确选择：删除旧 record，或保存一个明确不可持久化的新状态。

### 17.12 `TypeMemberIndex` 仍依赖完整 Document 生命周期

后台临时 Document 没有写入 `TypeMemberIndex` 是正确的，因为其中保存裸 `SymbolInfo*`。这意味着未打开文件的类型成员不会通过该后台索引参与成员补全。如果未来需要全局成员补全，应为成员建立值类型 DTO，而不能直接缓存当前指针。

### 17.13 `TypeMemberIndex::IndexDocument` 的递归位置值得复核

当前对子 scope 的递归位于当前 scope 的 symbol 循环内部：当前 scope 每有一个 symbol，就会重复遍历一次所有 children；如果当前 scope 没有 symbol，则完全不会遍历 children。递归循环应与 symbol 循环同级。

### 17.14 `Load` 对首次没有缓存使用 warning

第一次启动时缓存文件不存在是正常情况。当前 `LoadBinary` 失败会输出 warning，可能造成不必要日志噪音。可以对 `no_such_file_or_directory` 静默视为 cache miss，只对确实存在但读取失败或损坏的缓存警告。

## 18. 建议的 LSP 接入位置

为了让设计真正生效，调用顺序建议如下。

### initialize

解析 `workspaceFolders`，没有时回退到 `rootUri`，保存规范化根目录。

### initialized

构造缓存路径和 cache key，调用 `workspace_.StartBackgroundIndex(...)`。

### didOpen

在把更新任务放入队列之前调用 `workspace_.MarkDocumentOpen(uri)`，确保已经运行的磁盘任务不能晚于内存文档提交。

### didSave

让普通文档更新完成后调用 `workspace_.ScheduleDiskIndexByUri(uri)`，刷新代表磁盘状态的 `DiskIndexRecord`。include 保存时，依赖它的顶层文档也需要重新解析；五秒协调可以最终发现，但主动调度响应更快。

### didClose

调用 `workspace_.CloseDocument(uri)`，不要调用会彻底删除磁盘贡献的 `RemoveDocument(uri)`。

### shutdown/exit

调用 `StopBackgroundIndex`，确保 dirty cache 刷新并等待线程退出。`Workspace` 销毁时 `std::jthread` 也会请求停止，但显式停止的生命周期更清晰。

## 19. 设计不变量总结

理解和维护这套实现时，应始终保持以下不变量：

1. `references_` 中的计数等于当前所有 owner document 对同一引用边的贡献数。
2. `document_contributions_[uri]` 必须是该 URI 当前已提交版本的完整、去重贡献集合。
3. 文档替换必须先撤销旧贡献，再安装新贡献，并对查询表现为一次原子操作。
4. 打开的内存文档优先级高于后台磁盘文档。
5. generation 不匹配的后台结果永远不能提交。
6. `disk_index_records_` 只表示磁盘版本，不能被未保存编辑器内容污染。
7. 持久化位置不能保存进程内 `SourceFile*`，必须保存 URI/行/列并在加载时重新 intern。
8. 一条缓存记录只有在 owner 和全部依赖 stamp 都有效时才能恢复。
9. 缓存不可用只应导致重新索引，不应导致 LSP 失败。
10. 保存 `SymbolInfo*` 的索引必须受完整 `Document` 生命周期约束。

满足这些不变量后，系统才能同时支持 include 反向引用、多顶层文档重复贡献、编辑器未保存覆盖、后台增量更新和跨进程缓存恢复。
