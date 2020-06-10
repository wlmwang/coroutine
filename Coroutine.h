// By: wlmwang
// Date: Jun 07 2020

#ifndef _COROUTINE_H
#define _COROUTINE_H

#if __APPLE__ && __MACH__
	#include <sys/ucontext.h>
#else 
	#include <ucontext.h>
#endif

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <memory>
#include <vector>

class Schedule;

// 协程实体
class Coroutine
{
public:
	Coroutine(Schedule* sch, CoCallback cb)
		: sch_(sch)
		, func_(cb) {}

	~Coroutine() = default;

	void saveStack(char *src, int maxsize);
	void copyStack(char *dst, int maxsize);
	void run();

private:
	friend class Schedule;

	Schedule* sch_;		// 协程调度器

	ucontext_t ctx_;				// 协程上下文
	CoCallback func_;				// 协程入口函数（用户函数）
	int status_{COROUTINE_READY};	// 协程状态
	std::vector<char> stack_;		// 协程私有栈（挂起时保存，非固定大小）
};

#endif	// _COROUTINE_H
