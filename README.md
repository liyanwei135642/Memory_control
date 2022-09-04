# 模拟实现 google 开源项目 tcmalloc 高并发内存池 
区别 PageCache 回收的时候没有合并Pages 利用Unordered_map 保存原始指针和剩余总页数还有Span集 回收够了128页全部释放给堆；
