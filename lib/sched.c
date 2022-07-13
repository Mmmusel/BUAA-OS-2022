#include <env.h>
#include <pmap.h>
#include <printf.h>

/* Overview:
 *  Implement simple round-robin scheduling.
 *
 *
 * Hints:
 *  1. The variable which is for counting should be defined as 'static'.
 *  2. Use variable 'env_sched_list', which is a pointer array.
 *  3. CANNOT use `return` statement!
 */
/*** exercise 3.15 ***/
extern struct Env_list env_sched_list[];
extern struct Env *curenv;

void sched_yield(void)
{
	/*
	static u_int cur_lasttime = 1;
	struct Env *next_env;
	cur_lasttime--;
	if (cur_lasttime == 0 || curenv == NULL)
	{
		if (curenv != NULL)
		{
			LIST_INSERT_TAIL(&env_sched_list[1], curenv, env_sched_link);
		}
		if (!LIST_EMPTY(&env_sched_list[0]))
		{
			next_env = LIST_FIRST(&env_sched_list[0]);
			LIST_REMOVE(next_env, env_sched_link);
		}
		else
		{
			next_env = LIST_FIRST(&env_sched_list[1]);
			LIST_REMOVE(next_env, env_sched_link);
		}
		cur_lasttime = next_env->env_pri;
		//printf("%x %x\n",curenv, next_env);
		env_run(next_env);
	}
	env_run(curenv);
	//printf("%x",*((int*)(TF_EPC + TIMESTACK - TF_SIZE)));
	//printf("Sched_yield End\n");
	*/
	//printf("Sched_yield happen\n");
	static int cur_lasttime = 1;
	static int cur_head_index = 0;
	struct Env *next_env;
	int now_have = 0;
	cur_lasttime--;
	if (cur_lasttime <= 0 || curenv == NULL || curenv->env_status != ENV_RUNNABLE)
	{
		now_have = 0;
		while(1)
		{
			if (LIST_EMPTY(&env_sched_list[cur_head_index]))
			{
				cur_head_index = !cur_head_index;
				break;
			}
			next_env = LIST_FIRST(&env_sched_list[cur_head_index]);
			if (next_env->env_status == ENV_RUNNABLE)
			{
				now_have = 1;
				break;
			}
			LIST_REMOVE(next_env, env_sched_link);
			LIST_INSERT_HEAD(&env_sched_list[!cur_head_index], next_env, env_sched_link);
		}
		if (!now_have)
		{
			while (1)
			{
				if (LIST_EMPTY(&env_sched_list[cur_head_index]))
				{
					panic("^^^^^^No env is RUNNABLE!^^^^^^");
				}
				next_env = LIST_FIRST(&env_sched_list[cur_head_index]);
				if (next_env->env_status == ENV_RUNNABLE)
				{
					now_have = 1;
					break;
				}
				LIST_REMOVE(next_env, env_sched_link);
				LIST_INSERT_HEAD(&env_sched_list[!cur_head_index], next_env, env_sched_link);
			}
		}
		LIST_REMOVE(next_env, env_sched_link);
		LIST_INSERT_HEAD(&env_sched_list[!cur_head_index], next_env, env_sched_link);
		cur_lasttime = next_env->env_pri;
		env_run(next_env);
		panic("^^^^^^sched yield jump faild^^^^^^");
	}
	env_run(curenv);
	panic("^^^^^^sched yield reached end^^^^^^");
}
/*
void sched_yield(void)
{
	static int count = 1;
	static int point = 0;
	struct Env *next_env;
	int now_have = 0;
count--;
	if (count <= 0 || curenv == NULL || curenv->env_status != ENV_RUNNABLE)
	{
		now_have = 0;
		while(1)
		{
			if (LIST_EMPTY(&env_sched_list[point]))
			{
				point = !point;
				break;
			}
			next_env = LIST_FIRST(&env_sched_list[point]);
			if (next_env->env_status == ENV_RUNNABLE)
			{
				now_have = 1;
				break;
			}
			LIST_REMOVE(next_env, env_sched_link);
			LIST_INSERT_HEAD(&env_sched_list[!point], next_env, env_sched_link);
		}
		if (!now_have)
		{
			while (1)
			{
				if (LIST_EMPTY(&env_sched_list[point]))
				{
					panic("^^^^^^No env is RUNNABLE!^^^^^^");
				}
				next_env = LIST_FIRST(&env_sched_list[point]);
				if (next_env->env_status == ENV_RUNNABLE)
				{
					now_have = 1;
					break;
				}
				LIST_REMOVE(next_env, env_sched_link);
				LIST_INSERT_HEAD(&env_sched_list[!point], next_env, env_sched_link);
			}
		}
		LIST_REMOVE(next_env, env_sched_link);
		LIST_INSERT_HEAD(&env_sched_list[!point], next_env, env_sched_link);
		count = next_env->env_pri;
		env_run(next_env);
		panic("^^^^^^sched yield jump faild^^^^^^");
	}
	env_run(curenv);
	panic("^^^^^^sched yield reached end^^^^^^");
}
*/

	/*
    static int count = 0; // remaining time slices of current env
    static int point = 0; // current env_sched_list index
    static struct Env *varr;
    if (curenv!=NULL){
    varr = curenv;
   // printf("sched run\n");
    if (count == 0) {
        LIST_REMOVE(varr,env_sched_link);
        LIST_INSERT_TAIL(&(env_sched_list[1-point]),varr,env_sched_link);
    }
    //printf("222\n");
    if (LIST_EMPTY(&(env_sched_list[point]))) {
        point = 1 - point;
      //  printf("333\n");
        LIST_FOREACH(varr,&(env_sched_list[point]),env_sched_link) {
            if (varr->env_status == ENV_RUNNABLE) {
                count = varr->env_pri;
                break;
            }
        }
    }
    count--;
  //  printf("env run start\n");
    }
    else {
//	    printf("this\n");
	    varr = LIST_FIRST(&(env_sched_list[point]));
	    count=varr->env_pri;
	//    printf("end this\n");
    }
    env_run(varr);
//    printf("env run end\n");

*/

//	printf("----\n");
//if(LIST_EMPTY(env_sched_list))printf("empty\n");else printf("not em\n");
//	env_run(LIST_FIRST(env_sched_list));

//	printf("8888\n");
    /*  hint:
     *  1. if (count==0), insert `e` into `env_sched_list[1-point]`
     *     using LIST_REMOVE and LIST_INSERT_TAIL.
     *  2. if (env_sched_list[point] is empty), point = 1 - point;
     *     then search through `env_sched_list[point]` for a runnable env `e`, 
     *     and set count = e->env_pri
     *  3. count--
     *  4. env_run()
     *
     *  functions or macros below may be used (not all):
     *  LIST_INSERT_TAIL, LIST_REMOVE, LIST_FIRST, LIST_EMPTY
     */

