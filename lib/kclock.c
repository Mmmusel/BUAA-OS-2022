/* See COPYRIGHT for copyright information. */

/* The Run Time Clock and other NVRAM access functions that go with it. */
/* The run time clock is hard-wired to IRQ4. */


#include <kclock.h>

extern void set_timer();

/*** exercise 3.14 ***/
void
kclock_init(void)
{
//	printf("clockinit begin\n");
	set_timer();
//	printf("clock init done\n");
	// hint: use set_timer()
	
}

