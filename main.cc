// By: wlmwang
// Date: Jun 07 2020

#include "Schedule.h"
#include "Coroutine.h"

#include <iostream>

struct args
{
	int n;
};

static void foo(Schedule* sch, struct args* ud)
{
	assert(!!sch);
	struct args* arg = ud;

	for (int i = 0; i < 5; i++) {
		// 打印当前协程ID
		std::cout << "coroutine" << sch->coId() << " " << (arg->n + i) << std::endl;

		// 挂起当前协程
		sch->coYield();
	}
}

static void test(Schedule *sch)
{
	struct args arg1 = { 0 };
	struct args arg2 = { 100 };

	int co1 = sch->coNew(std::bind(&foo, sch, &arg1));
	int co2 = sch->coNew(std::bind(&foo, sch, &arg2));
	
	std::cout << "main start" << std::endl;

	// 循环调度所有协程
	while (sch->coStatus(co1) && sch->coStatus(co2)) {
		// 唤醒co1协程，直到当co1被挂起（或执行完成）时，继续执行
		sch->coResume(co1);

		// 唤醒co2协程，直到当co2被挂起（或执行完成）时，继续执行
		sch->coResume(co2);
	}

	std::cout << "main end" << std::endl;
}

int main(int argc, char* argv[])
{
	Schedule sch;
	test(&sch);

	return 0;
}

