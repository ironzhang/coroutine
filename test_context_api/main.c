#include <stdio.h>
#include <ucontext.h>

ucontext_t main_context;
ucontext_t f1_context;
ucontext_t f2_context;

char f1_stack[16384];
char f2_stack[16384];

void func1()
{
	printf("step 1\n");
	swapcontext(&f1_context, &main_context);

	printf("step 2\n");
	swapcontext(&f1_context, &main_context);

	printf("step 3\n");
	swapcontext(&f1_context, &main_context);

	printf("step 4\n");
	swapcontext(&f1_context, &main_context);

	printf("step 5\n");
	swapcontext(&f1_context, &main_context);

	printf("step 6\n");
	swapcontext(&f1_context, &main_context);
}

void func2()
{
	printf("func2: test\n");
}

int main()
{
	int ret;

	getcontext(&f1_context);
	getcontext(&f2_context);

	f1_context.uc_link = &main_context;
	f1_context.uc_stack.ss_sp = f1_stack;
	f1_context.uc_stack.ss_size = sizeof(f1_stack);
	makecontext(&f1_context, func1, 0);

	f2_context.uc_link = &main_context;
	f2_context.uc_stack.ss_sp = f2_stack;
	f2_context.uc_stack.ss_size = sizeof(f2_stack);
	makecontext(&f2_context, func2, 0);

	printf("will do step 1\n");
	ret = swapcontext(&main_context, &f1_context);

	printf("will do step 2\n");
	ret = swapcontext(&main_context, &f1_context);

	printf("will do step 3\n");
	ret = swapcontext(&main_context, &f1_context);

	printf("will do step 4\n");
	ret = swapcontext(&main_context, &f1_context);

	printf("will do step 5\n");
	ret = swapcontext(&main_context, &f1_context);

	printf("will do step 6\n");
	ret = swapcontext(&main_context, &f1_context);

	printf("will do what?\n");
	ret = swapcontext(&main_context, &f1_context);
	printf("ret=%d\n", ret);

	printf("will do what?\n");
	ret = swapcontext(&main_context, &f1_context);
	printf("ret=%d\n", ret);

	printf("will do what?\n");
	ret = swapcontext(&main_context, &f1_context);
	printf("ret=%d\n", ret);

	return 0;
}

