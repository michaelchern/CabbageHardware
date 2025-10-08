# TODO
- (紧急)重构queue的任务间同步、vkCmdPipelineBarrier、自动记录Barrier
- (紧急)重构多线程的锁、[ UNASSIGNED-Threading-MultipleThreads-Write ]
- (紧急)重构DisplayManager，由于多显卡间拷贝画面导致性能问题（尝试VK_EXT_external_memory_dma_buf），VMA不支持VK_EXT_external_memory
- (紧急)支持跨device传输（Vulkan与CUDA）
- GTX 1660Ti 中vmaCreateImage创建crash
- 程序关闭时资源释放问题
- BUG：mutiview初始化黑屏
- 画面改变大小会崩溃
- 补全DeviceFeaturesChain中特性
