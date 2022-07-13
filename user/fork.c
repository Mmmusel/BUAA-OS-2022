// implement fork from user space
#include "lib.h"
#include <mmu.h>
#include <env.h>
/* ----------------- help functions ---------------- */
/* Overview:
 * 	Copy `len` bytes from `src` to `dst`.
 *
 * Pre-Condition:
 * 	`src` and `dst` can't be NULL. Also, the `src` area
 * 	 shouldn't overlap the `dest`, otherwise the behavior of this
 * 	 function is undefined.
 */
void user_bcopy(const void *src, void *dst, size_t len)
{
	void *max;
	//	writef("~~~~~~~~~~~~~~~~ src:%x dst:%x len:%x\n",(int)src,(int)dst,len);
	max = dst + len;
	// copy machine words while possible
	if (((int)src % 4 == 0) && ((int)dst % 4 == 0)) {
		while (dst + 3 < max) {
			*(int *)dst = *(int *)src;
			dst += 4;
			src += 4;
		}
	}
	// finish remaining 0-3 bytes
	while (dst < max) {
		*(char *)dst = *(char *)src;
		dst += 1;
		src += 1;
	}
	//for(;;);
}
/* Overview:
 * 	Sets the first n bytes of the block of memory
 * pointed by `v` to zero.
 *
 * Pre-Condition:
 * 	`v` must be valid.
 *
 * Post-Condition:
 * 	the content of the space(from `v` to `v`+ n)
 * will be set to zero.
 */
void user_bzero(void *v, u_int n)
{
	char *p;
	int m;
	p = v;
	m = n;
	while (--m >= 0) {
		*p++ = 0;
	}
}
/*--------------------------------------------------------------*/
/* Overview:
 * 	Custom page fault handler - if faulting page is copy-on-write,
 * map in our own private writable copy.
 *
 * Pre-Condition:
 * 	`va` is the address which leads to a TLBS exception.
 *
 * Post-Condition:
 *  Launch a user_panic if `va` is not a copy-on-write page.
 * Otherwise, this handler should map a private writable copy of
 * the faulting page at correct address.
 */
/*** exercise 4.13 ***/
static void
pgfault(u_int va)
{
	/*
	u_int *tmp;
    //sys_mem_alloc(int sysno, u_int envid, u_int va, u_int perm)
    //user_bcopy(const void *src, void *dst, size_t len)
    //sys_mem_map(int sysno, u_int srcid, u_int srcva, u_int dstid, u_int dstva,
	//			u_int perm)
    //sys_mem_unmap(int sysno, u_int envid, u_int va)
    //memcpy(void *destaddr, void const *srcaddr, u_int len)????
    int ret;
	//	writef("fork.c:pgfault():\t va:%x\n",va);
    if ((((Pte *)(*vpt))[VPN(va)] & PTE_COW) == 0)
	{
		user_panic("User pgfault face a not COW page!");
	}
	va = ROUNDDOWN(va, BY2PG);

	tmp = USTACKTOP; 
	ret = syscall_mem_alloc(0, tmp, PTE_V | PTE_R);
	if (ret < 0)
	{
		user_panic("User pgfault alloc faild!");
	}

	//copy the content
	user_bcopy(va, tmp, BY2PG);
    //map the page on the appropriate place
	ret = syscall_mem_map(0, tmp, 0, va, PTE_V | PTE_R);
	if (ret < 0)
	{
		user_panic("User pgfault map faild!");
	}
    //unmap the temporary place
	ret = syscall_mem_unmap(0, tmp);
	if (ret < 0)
	{
		user_panic("User pgfault umap faild!");
	}
	return;


    
	//	writef("fork.c:pgfault():\t va:%x\n",va);
	//map the new page at a temporary place
	//copy the content
	//map the page on the appropriate place
	//unmap the temporary place
	*/
	u_int *tmp;
	u_long perm;
	//	writef("fork.c:pgfault():\t va:%x\n",va);

	perm = (*vpt)[VPN(va)] & (BY2PG - 1);
	if ((perm & PTE_COW) == 0) user_panic("`va` is not a copy-on-write page.");
	perm -= PTE_COW;
	tmp = USTACKTOP;

    //map the new page at a temporary place

	syscall_mem_alloc(0, tmp, perm);

	//copy the content

	user_bcopy(ROUNDDOWN(va, BY2PG), tmp, BY2PG);

    //map the page on the appropriate place

	syscall_mem_map(0, tmp, 0, va, perm);

    //unmap the temporary place

	syscall_mem_unmap(0, tmp);
}
/* Overview:
 * 	Map our virtual page `pn` (address pn*BY2PG) into the target `envid`
 * at the same virtual address.
 *
 * Post-Condition:
 *  if the page is writable or copy-on-write, the new mapping must be
 * created copy on write and then our mapping must be marked
 * copy on write as well. In another word, both of the new mapping and
 * our mapping should be copy-on-write if the page is writable or
 * copy-on-write.
 *
 * Hint:
 * 	PTE_LIBRARY indicates that the page is shared between processes.
 * A page with PTE_LIBRARY may have PTE_R at the same time. You
 * should process it correctly.
 */
/*** exercise 4.10 ***/
static void
duppage(u_int envid, u_int pn)
{
    //env = envs+ENVX(newenvid);
    //Pde *old_pgdir = curenv->env_pgdir;
    //Pde *new_pgdir = env->env_pgdir;
    //Pte *old_vpt = (UVPT+pn*4096);
    //Pte *new_vpt;

	u_int addr = pn*BY2PG;
	//u_int perm = ((u_long)(UVPT+pn) & 0xFFF);
    u_int perm = ((Pte *)(*vpt))[pn] & 0xfff;
    
    if (((perm & PTE_R)!=PTE_R) ||((perm & PTE_COW) == PTE_COW)|| ((perm & PTE_LIBRARY) == PTE_LIBRARY)) {
        if(syscall_mem_map(0, addr, envid, addr, perm) < 0)
		{
			user_panic("user panic mem map error!1");
		}
	    return;

    }

    perm = (perm | PTE_COW);
    
    //if ((perm & PTE_COW) != PTE_COW) {
    
    if(syscall_mem_map(0, addr, envid, addr, perm | PTE_COW) < 0)
		{
			user_panic("user panic mem map error!4");
		}
		if(syscall_mem_map(0, addr, 0, addr, perm | PTE_COW) < 0)
		{
			user_panic("user panic mem map error!5");
		}
    //不应该显式操作存储管理和env的相关内容
    //vpt的地址？能访存
    //vpt的作用并不是访存，只是获得perm

    return;
	//	user_panic("duppage not implemented");
}
/* Overview:
 * 	User-level fork. Create a child and then copy our address space
 * and page fault handler setup to the child.
 *
 * Hint: use vpd, vpt, and duppage.
 * Hint: remember to fix "env" in the child process!
 * Note: `set_pgfault_handler`(user/pgfault.c) is different from
 *       `syscall_set_pgfault_handler`.
 */
/*** exercise 4.9 4.15***/
extern void __asm_pgfault_handler(void);
int
fork(void)
{
	u_int newenvid;
	extern struct Env *envs;
	extern struct Env *env;
	u_int i,j,pppn;

    //Error: set_pgfault_handler(__asm_pgfault_handler);
    //这个函数为什么要传值？直接无参数返回不行吗？
    set_pgfault_handler(pgfault);

    newenvid = syscall_env_alloc();

    if (newenvid == 0) {
        newenvid = syscall_getenvid();
        
        env = envs+ENVX(newenvid);
        return 0;
    }
	//The parent installs pgfault using set_pgfault_handler
	//alloc a new alloc
    
        //env = envs+ENVX(newenvid);

        //Pde *old_pgdir = curenv->env_pgdir;
        //Pde *new_pgdir = env->env_pgdir;
        //Pte *old_vpt;
        //Pte *new_vpt;
        //u_int perm;

        for (i = 0; i < VPN(USTACKTOP); i++)
        {
	        if (((*vpd)[i >> 10] & PTE_V) && ((*vpt)[i] & PTE_V)) duppage(newenvid, i);
        }
        //alloc
        syscall_mem_alloc(newenvid, UXSTACKTOP - BY2PG, PTE_V | PTE_R);
        syscall_set_pgfault_handler(newenvid,__asm_pgfault_handler,UXSTACKTOP);
        syscall_set_env_status( newenvid, ENV_RUNNABLE);

    
	return newenvid;
}

/*
perm = (old_vpt[j] & 0xFFF);
if((perm & PTE_R)==PTE_R) {
if ((perm & PTE_LIBRARY) != PTE_LIBRARY) {
    old_vpt[j]|=PTE_COW;
    new_vpt[j]|=PTE_COW;
} else {
    syscall_mem_map(0, curenv->env_id, (i*1024+j)*4096, newenvid, (i*1024+j)*4096,perm);
}
}
*/

// Challenge!
int
sfork(void)
{
	user_panic("sfork not implemented");
	return -E_INVAL;
}
