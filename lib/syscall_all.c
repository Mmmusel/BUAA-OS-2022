#include "../drivers/gxconsole/dev_cons.h"
#include <mmu.h>
#include <env.h>
#include <printf.h>
#include <pmap.h>
#include <sched.h>
#define E_ENV_VAR_NOT_FOUND 13
#define E_ENV_VAR_READONLY 14

#define MAXERROR 14
extern char *KERNEL_SP;
extern struct Env *curenv;
/* Overview:
 * 	This function is used to print a character on screen.
 *
 * Pre-Condition:
 * 	`c` is the character you want to print.
 */
void sys_putchar(int sysno, int c, int a2, int a3, int a4, int a5)
{
	printcharc((char) c);
	return ;
}
/* Overview:
 * 	This function enables you to copy content of `srcaddr` to `destaddr`.
 *
 * Pre-Condition:
 * 	`destaddr` and `srcaddr` can't be NULL. Also, the `srcaddr` area
 * 	shouldn't overlap the `destaddr`, otherwise the behavior of this
 * 	function is undefined.
 *
 * Post-Condition:
 * 	the content of `destaddr` area(from `destaddr` to `destaddr`+`len`) will
 * be same as that of `srcaddr` area.
 */
void *memcpy(void *destaddr, void const *srcaddr, u_int len)
{
	char *dest = destaddr;
	char const *src = srcaddr;
	while (len-- > 0) {
		*dest++ = *src++;
	}
	return destaddr;
}
/* Overview:
 *	This function provides the environment id of current process.
 *
 * Post-Condition:
 * 	return the current environment id
 */
u_int sys_getenvid(void)
{
	return curenv->env_id;
}
/* Overview:
 *	This function enables the current process to give up CPU.
 *
 * Post-Condition:
 * 	Deschedule current environment. This function will never return.
 */
/*** exercise 4.6 ***/
void sys_yield(void)
{
    bcopy((void *)KERNEL_SP - sizeof(struct Trapframe),
     (void *)TIMESTACK - sizeof(struct Trapframe), 
     sizeof(struct Trapframe));
    sched_yield();
	
}
/* Overview:
 * 	This function is used to destroy the current environment.
 *
 * Pre-Condition:
 * 	The parameter `envid` must be the environment id of a
 * process, which is either a child of the caller of this function
 * or the caller itself.
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 when error occurs.
 */
int sys_env_destroy(int sysno, u_int envid)
{
	int r;
	struct Env *e;
	if ((r = envid2env(envid, &e, 1)) < 0) {
		return r;
	}
//	printf("[%08x] destroying %08x\n", curenv->env_id, e->env_id);
	env_destroy(e);
	return 0;
}
/* Overview:
 * 	Set envid's pagefault handler entry point and exception stack.
 *
 * Pre-Condition:
 * 	xstacktop points one byte past exception stack.
 *
 * Post-Condition:
 * 	The envid's pagefault handler will be set to `func` and its
 * 	exception stack will be set to `xstacktop`.
 * 	Returns 0 on success, < 0 on error.
 */
/*** exercise 4.12 ***/
int sys_set_pgfault_handler(int sysno, u_int envid, u_int func, u_int xstacktop)
{
	struct Env *env;
	int r;
    if ((r = envid2env(envid, &env, 1)) < 0) return r;
    env-> env_pgfault_handler = func;
    env->env_xstacktop=xstacktop;
	return 0;
	//	panic("sys_set_pgfault_handler not implemented");
}
/* Overview:
 * 	Allocate a page of memory and map it at 'va' with permission
 * 'perm' in the address space of 'envid'.
 *
 * 	If a page is already mapped at 'va', that page is unmapped as a
 * side-effect.
 *
 * Pre-Condition:
 * perm -- PTE_V is required,
 *         PTE_COW is not allowed(return -E_INVAL),
 *         other bits are optional.
 *
 * Post-Condition:
 * Return 0 on success, < 0 on error
 *	- va must be < UTOP
 *	- env may modify its own address space or the address space of its children
 */
/*** exercise 4.3 ***/
int sys_mem_alloc(int sysno, u_int envid, u_int va, u_int perm)
{
	struct Env *env;
	struct Page *ppage;
	int r;

    if((va>=UTOP) || ((perm & PTE_COW)==PTE_COW)||((perm&PTE_V)!=PTE_V)) return -E_INVAL;
    if((r = envid2env(envid,&env,1))<0) return r;
    if((r = page_alloc(&ppage)) < 0) return r;

    if((r = page_insert(env->env_pgdir,ppage,va,perm)) < 0) return r;

    return 0;
}
/* Overview:
 * 	Map the page of memory at 'srcva' in srcid's address space
 * at 'dstva' in dstid's address space with permission 'perm'.
 * Perm has the same restrictions as in sys_mem_alloc.
 * (Probably we should add a restriction that you can't go from
 * non-writable to writable?)
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 on error.
 *
 * Note:
 * 	Cannot access pages above UTOP.
 */
/*** exercise 4.4 ***/
int sys_mem_map(int sysno, u_int srcid, u_int srcva, u_int dstid, u_int dstva,
				u_int perm)
{
	int r;
	u_int round_srcva, round_dstva;
	struct Env *srcenv;
	struct Env *dstenv;
	struct Page *ppage;
	Pte *ppte;
	ppage = NULL;
	//r = 0;
	round_srcva = ROUNDDOWN(srcva, BY2PG);
	round_dstva = ROUNDDOWN(dstva, BY2PG);

    if (srcva >= UTOP || dstva >= UTOP) return -E_INVAL;
    if( ((perm & PTE_V)!=PTE_V)) return -E_INVAL;
    
    if((r = envid2env(srcid,&srcenv,0))<0) return r;
    if((r = envid2env(dstid,&dstenv,0))<0) return r;

    if((ppage = page_lookup(srcenv->env_pgdir,round_srcva,&ppte)) == NULL) {
	return -E_INVAL;
    }

    //Hint: can't go from non-writable to writable
   // u_int srcperm=(*ppte)&0xFFF;
   // if(((srcperm&PTE_V)!=PTE_V))return -E_INVAL;
    if (((perm & PTE_R) == 0) && ((perm & PTE_R) == 1)) return -E_INVAL;

   // ppage = pa2page(PTE_ADDR(*ppte));
    //Redundant: ppage = pa2page(PTE_ADDR(*ppte));
    if((r = page_insert(dstenv->env_pgdir,ppage,round_dstva,perm)) < 0) return r;

	return 0;
}
/* Overview:
 * 	Unmap the page of memory at 'va' in the address space of 'envid'
 * (if no page is mapped, the function silently succeeds)
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 on error.
 *
 * Cannot unmap pages above UTOP.
 */
/*** exercise 4.5 ***/
int sys_mem_unmap(int sysno, u_int envid, u_int va)
{
	int r;
	struct Env *env;

    if (va >= UTOP) return -E_INVAL;
    if((r = envid2env(envid,&env,0)) < 0) return r;
    page_remove(env->env_pgdir,va);

	return r;
	//	panic("sys_mem_unmap not implemented");
}
/* Overview:
 * 	Allocate a new environment.
 *
 * Pre-Condition:
 * The new child is left as env_alloc created it, except that
 * status is set to ENV_NOT_RUNNABLE and the register set is copied
 * from the current environment.
 *
 * Post-Condition:
 * 	In the child, the register set is tweaked so sys_env_alloc returns 0.
 * 	Returns envid of new environment, or < 0 on error.
 */
/*** exercise 4.8 ***/
int sys_env_alloc(void)
{
	int r;
	struct Env *e;
    if((r = env_alloc(&e,curenv->env_id)) < 0) return r;

	//Error: 不能从&(curenv->env_tf)中复制恢复现场：父进程处于中断，协寄存器
    bcopy((void *)KERNEL_SP - sizeof(struct Trapframe), (void *)&(e->env_tf), sizeof(struct Trapframe));
    
    //fork()'s child update return value
    e->env_tf.regs[2]=0;
    //fork()'s child block to wait for info
    e->env_status=ENV_NOT_RUNNABLE;

    //set initial value outside env_alloc(which is env_run & creatr_pri) should set there
    e->env_pri = curenv->env_pri;
    //pc: child from now PC run
    e->env_tf.pc = e->env_tf.cp0_epc;
    
	return e->env_id;
	//	panic("sys_env_alloc not implemented");
}
/* Overview:
 * 	Set envid's env_status to status.
 *
 * Pre-Condition:
 * 	status should be one of `ENV_RUNNABLE`, `ENV_NOT_RUNNABLE` and
 * `ENV_FREE`. Otherwise return -E_INVAL.
 *
 * Post-Condition:
 * 	Returns 0 on success, < 0 on error.
 * 	Return -E_INVAL if status is not a valid status for an environment.
 * 	The status of environment will be set to `status` on success.
 */
/*** exercise 4.14 ***/
int sys_set_env_status(int sysno, u_int envid, u_int status)
{
	// Your code here.
	struct Env *env;
	int ret;

	if (status != ENV_RUNNABLE && status != ENV_NOT_RUNNABLE && status != ENV_FREE) return -E_INVAL;
	ret = envid2env(envid, &env, 0);
	if (ret) return ret;
	if (env->env_status != ENV_RUNNABLE && status == ENV_RUNNABLE) LIST_INSERT_HEAD(&env_sched_list[0], env, env_sched_link);
	if (env->env_status == ENV_RUNNABLE && status != ENV_RUNNABLE) LIST_REMOVE(env, env_sched_link);
	env->env_status = status;

	return 0;
}
/* Overview:
 * 	Set envid's trap frame to tf.
 *
 * Pre-Condition:
 * 	`tf` should be valid.
 *
 * Post-Condition:
 * 	Returns 0 on success, < 0 on error.
 * 	Return -E_INVAL if the environment cannot be manipulated.
 *
 * Note: This hasn't be used now?
 */
int sys_set_trapframe(int sysno, u_int envid, struct Trapframe *tf)
{
	return 0;
}
/* Overview:
 * 	Kernel panic with message `msg`.
 *
 * Pre-Condition:
 * 	msg can't be NULL
 *
 * Post-Condition:
 * 	This function will make the whole system stop.
 */
void sys_panic(int sysno, char *msg)
{
	// no page_fault_mode -- we are trying to panic!
	panic("%s", TRUP(msg));
}
/* Overview:
 * 	This function enables caller to receive message from
 * other process. To be more specific, it will flag
 * the current process so that other process could send
 * message to it.
 *
 * Pre-Condition:
 * 	`dstva` is valid (Note: NULL is also a valid value for `dstva`).
 *
 * Post-Condition:
 * 	This syscall will set the current process's status to
 * ENV_NOT_RUNNABLE, giving up cpu.
 */
/*** exercise 4.7 ***/
void sys_ipc_recv(int sysno, u_int dstva)
{
    if (dstva >= UTOP) return;
    curenv->env_ipc_recving = 1;
    curenv->env_ipc_dstva = dstva;
    curenv->env_status = ENV_NOT_RUNNABLE;
    sys_yield();
}
/* Overview:
 * 	Try to send 'value' to the target env 'envid'.
 *
 * 	The send fails with a return value of -E_IPC_NOT_RECV if the
 * target has not requested IPC with sys_ipc_recv.
 * 	Otherwise, the send succeeds, and the target's ipc fields are
 * updated as follows:
 *    env_ipc_recving is set to 0 to block future sends
 *    env_ipc_from is set to the sending envid
 *    env_ipc_value is set to the 'value' parameter
 * 	The target environment is marked runnable again.
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 on error.
 *
 * Hint: the only function you need to call is envid2env.
 */
/*** exercise 4.7 ***/
int sys_ipc_can_send(int sysno, u_int envid, u_int value, u_int srcva,
					 u_int perm)
{
	int r;
	struct Env *env;
	struct Page *p;
    
    if (srcva >= UTOP) return -E_INVAL;
    if((r = envid2env(envid,&env,0))<0) return r;
    if((env->env_ipc_recving) == 0)return -E_IPC_NOT_RECV;

    env->env_ipc_recving = 0;
    env->env_ipc_value = value;
    env->env_ipc_from = curenv->env_id;
    env->env_ipc_perm = perm;
    env->env_status = ENV_RUNNABLE;

    Pte *pte;
    if(srcva != 0) {
        p = page_lookup(curenv->env_pgdir, srcva, &pte);
		if (p == NULL) return -E_INVAL;
		page_insert(env->env_pgdir, p, env->env_ipc_dstva, perm);
    }
	return 0;
}
/* Overview:
 * 	This function is used to write data to device, which is
 * 	represented by its mapped physical address.
 *	Remember to check the validity of device address (see Hint below);
 * 
 * Pre-Condition:
 *      'va' is the starting address of source data, 'len' is the
 *      length of data (in bytes), 'dev' is the physical address of
 *      the device
 * 	
 * Post-Condition:
 *      copy data from 'va' to 'dev' with length 'len'
 *      Return 0 on success.
 *	Return -E_INVAL on address error.
 *      
 * Hint: Use ummapped segment in kernel address space to perform MMIO.
 *	 Physical device address:
 *	* ---------------------------------*
 *	|   device   | start addr | length |
 *	* -----------+------------+--------*
 *	|  console   | 0x10000000 | 0x20   |
 *	|    IDE     | 0x13000000 | 0x4200 |
 *	|    rtc     | 0x15000000 | 0x200  |
 *	* ---------------------------------*
 */
 /*** exercise 5.1 ***/
int PHYSADDR_OFFSET = 0xA0000000;
int sys_read_dev(int sysno, u_int va, u_int dev, u_int len)
{
if (len < 0) {
        return -E_INVAL;
    }
    if ((0x10000000 <= dev && (dev + len) <= (0x10000000 + 0x20)) || (0x13000000 <= dev && (dev + len) <= (0x13000000 + 0x4200)) || (0x15000000 <= dev && (dev + len) <= (0x15000000 + 0x200))) 
    {
        bcopy((void *)(0xA0000000 + dev), (void *)va, len);
//         printf("sys_read_dev succeed.\n");
        return 0;
    }
    return -E_INVAL;
/*

	if ((dev >= 0x10000000) && (dev < 0x10000020)) {
        if ((dev + len) >= 0x10000020) return -E_INVAL;
    } else if ((dev >= 0x13000000) && (dev < 0x13004200)) {
        if ((dev + len) >= 0x13004200) return -E_INVAL;
    } else if ((dev >= 0x15000000) && (dev < 0x15000200)) {
        if ((dev + len) >= 0x15000200) return -E_INVAL;
    } else {
        return -E_INVAL;
    }

    bcopy(va,(dev+PHYSADDR_OFFSET),len);
*/
}

/* Overview:
 * 	This function is used to read data from device, which is
 * 	represented by its mapped physical address.
 *	Remember to check the validity of device address (same as sys_write_dev)
 * 
 * Pre-Condition:
 *      'va' is the starting address of data buffer, 'len' is the
 *      length of data (in bytes), 'dev' is the physical address of
 *      the device
 * 
 * Post-Condition:
 *      copy data from 'dev' to 'va' with length 'len'
 *      Return 0 on success, < 0 on error
 *      
 * Hint: Use ummapped segment in kernel address space to perform MMIO.
 */
 /*** exercise 5.1 ***/
int sys_write_dev(int sysno, u_int va, u_int dev, u_int len)
{
	if (len < 0) {
        return -E_INVAL;
    }
    if ((0x10000000 <= dev && (dev + len) <= (0x10000000 + 0x20)) || (0x13000000 <= dev && (dev + len) <= (0x13000000 + 0x4200)) || (0x15000000 <= dev && (dev + len) <= (0x15000000 + 0x200)))
    {
        bcopy((void *)va, (void *)(0xA0000000 + dev), len);
//         printf("sys_write_dev succeed.\n");
        return 0;
    }
    return -E_INVAL;
	/*
	if ((dev>= 0x10000000) && (dev < 0x10000020)) {
        if ((dev + len) >= 0x10000020) return -E_INVAL;
    } else if ((dev >= 0x13000000) && (dev < 0x13004200)) {
        if ((dev + len) >= 0x13004200) return -E_INVAL;
    } else if ((dev >= 0x15000000) && (dev < 0x15000200)) {
        if ((dev + len) >= 0x15000200) return -E_INVAL;
    } else {
        return -E_INVAL;
    }

    bcopy((dev+PHYSADDR_OFFSET),va,len);
*/
    }

// 0 - create
// 1 - get
// 2 - set
// 3 - unset
// 4 - get list
// 5 - create readonly
int sys_env_var(int sysno, char *name, char *value, u_int op) {
    const int MOD = 1 << 8;
    static char name_table[128][64];
    static char value_table[128][256];
    static int mode[128];
static int point=0;
static char tmp_name[64][128][64];
static char tmp_value[64][128][256];

static int tmp_mode[64][128];
static int tmp_point[64]={0};

//mode
//0 unset
//1 set

//3 readonly

/*
    if (op == 4) {
        char **name_list = name;
        char **value_list = value;
            if (name_table[i][0]) {
		    printf("find %d\n",i);
strcpy((name+pos*2),name_table[i]);
		     strcpy((value+pos*5), value_table[i]);
                pos+=4;
            }
	return 0;
    }
    */
/*
        if (op == 4) {
        char **name_list = name;
        char **value_list = value;
        int pos = 0, i;
        for (i = 0; i < MOD; ++i)
            if (name_table[i][0]) {
                name_list[pos] = name_table[i];
                value_list[pos] = value_table[i];
                ++pos;
            }
        name_list[pos] = 0;
    }

    */



/*
    u_int pos = strhash(name);

    while (name_table[pos][0]) {
        if (strcmp(name_table[pos], name) == 0) { // FOUND
            if (op == 0) return 0;
            break;
        } else {
            ++pos;
            if (pos == MOD) pos = 0;
        }
    }


*/
int id =(ENVX(curenv->env_id));
if (op==4) {
int pos=0;
int k;
printf("        \033[0;36m[global variable]\033[m        \n");
for(k=0;k<point;k++){
//	printf("%d ",k);
	if(mode[k]){
	int p = 0;
	

        while (p < 64 && name_table[k][p]) printf("\033[0;32;34m%c\033[m",name_table[k][p++]);
        p = 0;
	printf(" = ");
        while (p < 256 && value_table[k][p]) printf("\033[0;32;34m%c\033[m",value_table[k][p++]);
printf("\n");

       // while (p < 64 && name_table[k][p]) {*(name+pos*64+p)=name_table[k][p++];}
        p = 0;
      //  while (p < 256 && value_table[k][p]) *(value+pos*256+p)=value_table[k][p++];
	pos++;
//	printf("pos %d\n",pos);
	}
}
printf("	\033[0;36m[local variable]\033[m	\n");
for(k=0;k<tmp_point[id];k++){
        if(tmp_mode[id][k]){
        int p = 0;
        while (p < 64 && tmp_name[id][k][p]) printf("\033[0;32;34m%c\033[m",tmp_name[id][k][p++]);
        p = 0;
	printf(" = ");
        while (p < 256 && tmp_value[id][k][p]) printf("\033[0;32;34m%c\033[m",tmp_value[id][k][p++]);
        pos++;
	printf("\n");
        }
}
//printf("part2\n");
return 0;}

int pos = -1;
int i;
int var_type=0;//global
for(i=0;i<point;i++) {
	if(mode[i]) {
		if(strcmp(name_table[i], name)==0) {
			pos = i;
			break;
		}
	}
}
if(pos==-1) {
	var_type = 1;//tmp

	for(i=0;i<tmp_point[id];i++) {
        if(tmp_mode[id][i]) {
                if(strcmp(tmp_name[id][i], name)==0) {
                        pos = i;
                        break;
                }
        }
}
}
if(pos==-1){
	var_type = 3;//not found
}


    if (op == 6) { //global
        strcpy(name_table[point], name);
        strcpy(value_table[point], value);
	mode[point]=1;
	point++;
    }
    else if (op == 7) {
	    strcpy(tmp_name[id][tmp_point[id]],name);
	    strcpy(tmp_value[id][tmp_point[id]],value);
	    tmp_mode[id][tmp_point[id]]=1;
	   // printf("sss%s\n",tmp_name[id][tmp_point[id]]);
	   // printf("*********************id=%d,tmp_point[id]=%d\n",id,tmp_point[id]);
	    tmp_point[id]++;

    } else if (op == 1) {
	   // printf("gettttttttttt vartype=%d",var_type);
        if (var_type==3) return -E_ENV_VAR_NOT_FOUND;
	else if(var_type==0)strcpy(value, value_table[pos]);
	else strcpy(value, tmp_value[id][pos]);
    } else if (op == 2) {
        if (var_type==3) return -E_ENV_VAR_NOT_FOUND;
        if ((var_type==0)&&(mode[pos]==3)){printf("\033[1;31mError! %s is readly-variable!\n\033[m",name); return -E_ENV_VAR_READONLY;}
	if ((var_type==1)&&(tmp_mode[id][pos]==3)) {printf("\033[1;31mError! %s is readly-variable!\n\033[m",name);return -E_ENV_VAR_READONLY;}
        if(var_type==0)strcpy(value_table[pos], value);
	else if(var_type==1)strcpy(tmp_value[id][pos],value);
    } else if (op == 3) {
        if (var_type==3) return -E_ENV_VAR_NOT_FOUND;
	if (var_type==0){
		if(mode[pos]==3)printf("\033[1;31mError! %s is readly-variable!\n\033[m",name);
		else {
        mode[pos] = 0;
        int p = 0;
        while (p < 64 && name_table[pos][p]) name_table[pos][p++] = 0;
        p = 0;
        while (p < 256 && value_table[pos][p]) value_table[pos][p++] = 0;
	}}
	else {
		if(tmp_mode[id][pos] ==3) printf("\033[1;31mError! %s is readly-variable!\n\033[m",name);
		else {
		tmp_mode[id][pos] = 0;
        int p = 0;
        while (p < 64 && tmp_name[id][pos][p]) tmp_name[id][pos][p++] = 0;
        p = 0;
        while (p < 256 && tmp_value[id][pos][p]) tmp_value[id][pos][p++] = 0;
        }}
    } else if (op == 5) {
        strcpy(name_table[point], name);
        strcpy(value_table[point], value);
        mode[point] = 3;
	point++;
    }
    return 0;
}


