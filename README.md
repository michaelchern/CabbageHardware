# TODO
- (紧急)重构DisplayManager，由于多显卡间拷贝画面导致性能问题
- (紧急)（性能瓶颈）重构queue的任务间同步、vkCmdPipelineBarrier
  - 自动记录Barrier
- (紧急)重构多线程的锁
- (紧急)[ UNASSIGNED-Threading-MultipleThreads-Write ] vkQueueSubmit(): object of type VkQueue is simultaneously used in current thread 25780 and thread 14712
- (紧急)BUG：内存泄漏(待定位)
- 程序关闭时资源释放问题
- BUG：mutiview初始化黑屏
- 支持跨device传输（Vulkan与CUDA）

# 9.17 TODO
- CoronaEngine渲染细节确认
