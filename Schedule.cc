// By: wlmwang
// Date: Jun 07 2020

#include "Schedule.h"
#include "Coroutine.h"

void coMain(uint32_t low32, uint32_t hi32);

Schedule::Schedule()
	: cid_(-1)
	, nco_(0)
	, stack_(STACK_SIZE)
	, co_(DEFAULT_COROUTINE, nullptr)
{
	//...
}

Schedule::~Schedule()
{
	std::vector<Coroutine*>::iterator it = co_.begin();
	for (; it != co_.end(); it++) {
		delete *it;
	}
	co_.clear();
}

int Schedule::coNew(CoCallback cb)
{
	Coroutine *co = new Coroutine(this, cb);
	
	int cap = co_.size();
	if (nco_ >= cap) {
		// 扩展2被内存给协程队列
		co_.resize(cap * 2);

		int cid = cap;
		co_[cid] = co;
		++nco_;
		return cid;
	} else {
		// 分配一个空槽给新建的协程
		for (int i = 0; i < cap; i++) {
			int cid = (i + nco_) % cap;
			if (co_[cid] == nullptr) {
				co_[cid] = co;
				++nco_;
				return cid;
			}
		}
	}

	assert(0);
	return -1;
}

// 挂起当前协程
void Schedule::coYield()
{
	// 获取当前正在被调度的协程
	int cid = cid_;
	assert(cid >=0 && cid < co_.size());
	
	// 获取需要被挂起的协程实体
	Coroutine *co = co_[cid];

	// 当前协程被挂起 ==3
	co->status_ = COROUTINE_SUSPEND;

	// 保存当前协程实际使用的共享栈内存到私有栈中
	co->saveStack(stack_.data(), stack_.size());
	
	// 清除调度器中当前正在被调度的协程ID
	cid_ = -1;

	// 切换到主协程（即该协程被唤醒的地方）。再次唤醒该协程时，将从此处执行
	::swapcontext(&co->ctx_ , &main_);
}

void Schedule::coResume(int cid)
{
	// 协程ID合法性校验
	assert(cid >=0 && cid < co_.size());

	// 理论上，调度器中当前是没有正在被调度的协程
	// 1. 协程除非主动挂起，否则CPU的控制流将一直在被调度的协程中，直到其结束
	// 2. 以上两种情形（主动挂起、调度协程结束），调度器的当前正在被调度协程ID均会被置为-1
	assert(cid_ == -1);

	// 获取需要被调度的协程实体
	Coroutine *co = co_[cid];
	
	// 协程已退出、销毁
	if (co == nullptr) {
		return;
	}

	int status = co->status_;
	switch(status) {
	case COROUTINE_READY:		// 调度待运行的协程
		{
			// 设置调度器中正在被调度的协程ID
			cid_ = cid;

			// 该协程正在被调度 ==2
			co->status_ = COROUTINE_RUNNING;

			// 设置需要被调度的协程上下文
			::getcontext(&co->ctx_);
			co->ctx_.uc_stack.ss_sp = stack_.data();	// 协程栈指针（共享栈）
			co->ctx_.uc_stack.ss_size = stack_.size();	// 协程栈大小
			co->ctx_.uc_link = &main_;					// 协程结束的后续上下文为主协程

			// 指针被转换成两个32位参数（makecontext函数限制）
			uintptr_t ptr = (uintptr_t)this;
			::makecontext(&co->ctx_, (void(*)(void))coMain, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));

			// 设置协程被挂起后，切换到主协程继续执行的点
			// 切换需要被调度的协程，执行完该协程后，返回到此处
			::swapcontext(&main_, &co->ctx_);
			break;
		}

	case COROUTINE_SUSPEND:		// 调度被挂起的协程
		{
			// 设置调度器中正在被调度的协程ID
			cid_ = cid;

			// 该协程正在被调度 ==2
			co->status_ = COROUTINE_RUNNING;
			
			// 拷贝该协程实际使用的私有栈内存到共享栈中
			co->copyStack(stack_.data(), stack_.size());

			// 设置协程被挂起后，切换到主协程继续执行的点
			// 切换需要被调度的协程，执行完该协程后，返回到此处
			::swapcontext(&main_, &co->ctx_);
			break;
		}

	default:
		assert(0);	
	}
}

int Schedule::coStatus(int cid)
{
	// 协程ID合法性校验
	assert(cid >= 0 && cid < co_.size());

	if (co_[cid] == nullptr) {
		return COROUTINE_DEAD;	// 协程已销毁 ==0
	}
	return co_[cid]->status_;
}

// 协程调度时的底层函数
void coMain(uint32_t low32, uint32_t hi32)
{
	// 获取调度器的实体
	uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
	Schedule *sch = (struct Schedule *)ptr;

	// 获取当前正在被调度的协程
	int cid = sch->cid_;
	assert(cid >= 0 && cid < sch->co_.size());

	// 获取调度协程的实体
	Coroutine *co = sch->co_[cid];

	// 运行协程的用户函数
	{
		co->run();	
		delete co;
	}

	sch->co_[cid] = nullptr;
	--sch->nco_;

	// 清除调度器中当前正在被调度的协程ID
	sch->cid_ = -1;

	// 切换到主协程（即该协程被唤醒的地方）
}
