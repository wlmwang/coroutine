// By: wlmwang
// Date: Jun 07 2020

#include "Schedule.h"
#include "Coroutine.h"

void Coroutine::saveStack(char *src, int maxsize)
{
	// 栈变量dummy，一定是被分配在共享栈上
	char dummy = 0;

	// 协程栈越界检查，栈总是向低地址推进的。`src_top-&dummy`为当前协程实际使用的栈内存
	assert(src <= (&dummy-1));

	// 分配协程私有栈内存
	// 协程总是在共享栈中运行，里面保存了全部协程栈的上下文
	if (stack_.size() < (src + maxsize - &dummy)) {
		stack_.resize(src + maxsize - &dummy);
	}

	// 拷贝共享栈内存中当前协程实际使用的栈内存到协程私有栈中
	stack_.assign(&dummy, src + maxsize);
}

void Coroutine::copyStack(char *dst, int maxsize)
{
	assert(stack_.size() <= maxsize);

	// 拷贝该协程实际使用的私有栈内存到共享栈中（栈总是向低地址推进的）
	::memcpy(dst + maxsize - stack_.size(), stack_.data(), stack_.size());
}

void Coroutine::run()
{
	assert(func_);
	func_();
}
