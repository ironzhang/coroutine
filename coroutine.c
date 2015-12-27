#include "coroutine.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <ucontext.h>

//每个协程堆栈最大256K
#define MAX_CO_STACK_SIZE (256*1024)

typedef struct coroutine_t{
	int id;
	int status;
	void *ud;
	fn_coroutine_t fn_co;

	int resume_id;
	ucontext_t resume_ctx;
	ucontext_t yield_ctx;
	char stack[MAX_CO_STACK_SIZE];
}coroutine_t;

struct co_schedule_t{
	int running_id; //当前协程id
	int nco; //当前协程数量
	int max_co_num; //最大协程数量
	coroutine_t *co; //协程动态数组
};

static int init_co_schedule(co_schedule_t *sch, int max_co_num)
{
	int i;
	sch->running_id = -1;
	sch->nco = 0;
	sch->max_co_num = max_co_num;
	for(i = 0; i < max_co_num; i++)
		sch->co[i].id = -1;
	return 0;
}

co_schedule_t *coroutine_schedule_open(int max_co_num)
{
	int ret;
	size_t size;
	co_schedule_t *sch;

	assert(max_co_num > 0);

	size = sizeof(co_schedule_t) + sizeof(coroutine_t) * max_co_num;
	sch = (co_schedule_t *)malloc(size);
	if(sch == NULL)
		return NULL;
	sch->co = (coroutine_t *)(sch + 1);

	do{
		ret = init_co_schedule(sch, max_co_num);
		if(ret != 0)
			break;
		return sch;
	}while(0);

	free(sch);

	return NULL;
}

int coroutine_schedule_close(co_schedule_t *schedule)
{
	assert(schedule != NULL);
	free(schedule);
	return 0;
}

static coroutine_t *alloc_co(co_schedule_t *sch)
{
	int i, id;
	for(i = 0; i < sch->max_co_num; i++){
		id = (i + sch->nco) % sch->max_co_num;
		if(sch->co[id].id == -1){
			sch->nco++;
			sch->co[id].id = id;
			return &sch->co[id];
		}
	}
	return NULL;
}

static void free_co(co_schedule_t *sch, coroutine_t *co)
{
	co->id = -1;
	sch->nco--;
}

int coroutine_new(co_schedule_t *schedule, fn_coroutine_t fn_coroutine, void *ud)
{
	coroutine_t *co;

	assert(schedule != NULL && fn_coroutine != NULL);

	co = alloc_co(schedule);
	if(co == NULL)
		return -1;

	co->status = E_CO_STATUS_INIT;
	co->ud = ud;
	co->fn_co = fn_coroutine;
	co->resume_id = -1;

	return co->id;
}

//协程启动入口
static void co_start(uint32_t low, uint32_t high)
{
	coroutine_t *co;
	uintptr_t ptr = (uintptr_t)low | ((uintptr_t)high<<32);
	co_schedule_t *sch = (co_schedule_t *)ptr;
	co = &sch->co[sch->running_id];
	co->fn_co(sch, co->ud);
	co->status = E_CO_STATUS_DEAD;
	sch->running_id = co->resume_id;
	free_co(sch, co);
}

//启动协程
static int init_status_resume_co(co_schedule_t *sch, coroutine_t *co)
{
	int ret;
	uintptr_t ptr;

	ret = getcontext(&co->yield_ctx);
	if(ret != 0)
		return -1;

	ptr = (uintptr_t)sch;
	co->yield_ctx.uc_link = &co->resume_ctx;
	co->yield_ctx.uc_stack.ss_sp = co->stack;
	co->yield_ctx.uc_stack.ss_size = sizeof(co->stack);
	makecontext(&co->yield_ctx, (void (*)())co_start, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));

	co->resume_id = sch->running_id;
	sch->running_id = co->id;
	co->status = E_CO_STATUS_RUNNING;
	ret = swapcontext(&co->resume_ctx, &co->yield_ctx);
	if(ret != 0)
		return -1;

	return 0;
}

static int yield_status_resume_co(co_schedule_t *sch, coroutine_t *co)
{
	int ret;
	co->resume_id = sch->running_id;
	sch->running_id = co->id;
	co->status = E_CO_STATUS_RUNNING;
	ret = swapcontext(&co->resume_ctx, &co->yield_ctx);
	if(ret != 0)
		return -1;
	return 0;
}

int coroutine_resume(co_schedule_t *schedule, int id)
{
	int ret;
	coroutine_t *co;

	assert(schedule != NULL);
	assert(id >= 0 && id < schedule->max_co_num);

	co = &schedule->co[id];
	switch(co->status){
		case E_CO_STATUS_INIT:
			ret = init_status_resume_co(schedule, co);
			break;
		case E_CO_STATUS_YIELD:
			ret = yield_status_resume_co(schedule, co);
			break;
		default:
			ret = -1;
			break;
	}

	return ret;
}

int coroutine_yield(co_schedule_t *schedule)
{
	int ret;
	coroutine_t *co;

	assert(schedule != NULL);
	assert(schedule->running_id >= 0 && schedule->running_id < schedule->max_co_num);

	co = &schedule->co[schedule->running_id];
	co->status = E_CO_STATUS_YIELD;
	schedule->running_id = co->resume_id;
	ret = swapcontext(&co->yield_ctx, &co->resume_ctx);
	if(ret != 0)
		return -1;

	return 0;
}

int coroutine_status(co_schedule_t *schedule, int id)
{
	assert(schedule != NULL);
	assert(id >= 0 && id < schedule->max_co_num);
	return schedule->co[id].status;
}

int coroutine_running_id(co_schedule_t *schedule)
{
	assert(schedule != NULL);
	return schedule->running_id;
}

