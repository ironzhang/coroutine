#ifndef COROUTINE_H
#define COROUTINE_H

enum e_co_status
{
	E_CO_STATUS_INIT,
	E_CO_STATUS_DEAD,
	E_CO_STATUS_YIELD,
	E_CO_STATUS_RUNNING,
};

typedef struct co_schedule_t co_schedule_t;
typedef void (*fn_coroutine_t)(co_schedule_t *schedule, void *ud);

co_schedule_t *coroutine_schedule_open(int max_co_num);
int coroutine_schedule_close(co_schedule_t *schedule);

int coroutine_new(co_schedule_t *schedule, fn_coroutine_t fn_coroutine, void *ud);
int coroutine_resume(co_schedule_t *schedule, int id);
int coroutine_yield(co_schedule_t *schedule);

int coroutine_status(co_schedule_t *schedule, int id);
int coroutine_running_id(co_schedule_t *schedule);

#endif

