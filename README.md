
# Coroutine
* 需要 C++11 编译。代码容易阅读，几乎每行都加了注释
* 该 coroutine 实现了协程上下文切换原语
* 内含上下文切换，共享栈以及星切调度的算法实现
* 源码底层在上下文切换上使用了glibc的ucontext
* 没有任何HOOK相关代码，切换时需要用户手动调用

# 参考
* 云风的BLOG - https://blog.codingnow.com/2012/07/c_coroutine.html
