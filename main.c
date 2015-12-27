#include <stdio.h>
#include "coroutine.h"

typedef struct arg_t{
	int co1;
	int co2;
	int n;
}arg_t;

int co3, co4;

static void co1_func(co_schedule_t *sch, void *ud)
{
	arg_t *arg = (arg_t *)ud;

	printf("co1 start\n");

	printf("co1=%d: n=%d\n", arg->co1, arg->n);
	coroutine_resume(sch, arg->co2);

	printf("co1=%d: n=%d\n", arg->co1, arg->n);
	coroutine_resume(sch, arg->co2);

	coroutine_yield(sch);

	printf("co1 end\n");
}

static void co2_func(co_schedule_t *sch, void *ud)
{
	arg_t *arg = (arg_t *)ud;

	printf("co2 start\n");

	printf("co2=%d: n=%d\n", arg->co2, arg->n);
	arg->n++;
	coroutine_yield(sch);

	printf("co2=%d: n=%d\n", arg->co2, arg->n);
	arg->n++;
	coroutine_yield(sch);

	coroutine_resume(sch, arg->co1);
	printf("co2 end\n");
}

static void co3_func(co_schedule_t *sch, void *ud)
{
	printf("co3 start\n");
	printf("co3 step1\n");
	coroutine_yield(sch);

	coroutine_resume(sch, co4);
	coroutine_resume(sch, co4);
	coroutine_resume(sch, co4);
	coroutine_resume(sch, co4);
	coroutine_resume(sch, co4);

	printf("co3 step2\n");
	coroutine_yield(sch);

	printf("co3 step3\n");
	coroutine_yield(sch);

	printf("co3 step4\n");
	coroutine_yield(sch);

	printf("co3 end\n");
}

static void co4_func(co_schedule_t *sch, void *ud)
{
	printf("co4 start\n");
	printf("co4 step1\n");
	coroutine_yield(sch);

	printf("co4 step2\n");
	coroutine_yield(sch);

	printf("co4 step3\n");
	coroutine_yield(sch);

	printf("co4 step4\n");
	coroutine_yield(sch);

	printf("co4 end\n");
}

static void test1(co_schedule_t *sch)
{
	arg_t arg;
	arg.n = 0;
	arg.co1 = coroutine_new(sch, co1_func, &arg);
	arg.co2 = coroutine_new(sch, co2_func, &arg);

	coroutine_resume(sch, arg.co1);
	coroutine_resume(sch, arg.co2);
}

static void test2(co_schedule_t *sch)
{
	co3 = coroutine_new(sch, co3_func, NULL);
	co4 = coroutine_new(sch, co4_func, NULL);

	coroutine_resume(sch, co3);
	coroutine_resume(sch, co3);
	coroutine_resume(sch, co3);
	coroutine_resume(sch, co3);
	coroutine_resume(sch, co3);
}

int main()
{
	co_schedule_t *sch = coroutine_schedule_open(4000);
	if(sch == NULL){
		printf("coroutine schedule open error\n");
		return -1;
	}

	//test1(sch);
	test2(sch);

	coroutine_schedule_close(sch);
	return 0;
}

