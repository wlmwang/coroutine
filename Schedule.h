// By: wlmwang
// Date: Jun 07 2020

#ifndef _SCHEDULE_H
#define _SCHEDULE_H

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

#define STACK_SIZE (8*1024*1024)
#define DEFAULT_COROUTINE 16

#define COROUTINE_DEAD 0		// 协程已销毁
#define COROUTINE_READY 1		// 协程待运行（刚创建的协程）
#define COROUTINE_RUNNING 2		// 协程正在运行（调度中）
#define COROUTINE_SUSPEND 3		// 协程被挂起

// 协程入口函数
using CoCallback = std::function<void()>;

class Coroutine;

// 协程调度器，星切调度
// 某一时刻，调度器中仅有一个协程被调度
class Schedule
{
public:
	Schedule();
	~Schedule();
	
	// 创建一个新协程
	int coNew(CoCallback cb);

	// 挂起当前协程，并切换到主协程
	void coYield();

	// 唤醒指定协程，并执行
	void coResume(int cid);

	// 指定协程状态
	int coStatus(int cid);

	int coId() { return cid_;}
	
private:
	friend void coMain(uint32_t low32, uint32_t hi32);

	ucontext_t main_;				// 主协程上下文
	int cid_;						// 正在被调度的协程ID（协程队列索引值）
	int nco_;						// 协程实际个数
	std::vector<char> stack_;		// 协程共享栈（协程最大栈长度。固定容量）
	std::vector<Coroutine*> co_;	// 协程队列
};

#endif	// _SCHEDULE_H
