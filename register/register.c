
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/desc.h>
#include <asm/pgtable.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/highmem.h>
#include <linux/preempt.h>
#include <asm/topology.h>
#include <linux/kthread.h>
#include <linux/kernel_stat.h>
#include <linux/percpu-defs.h>
#include <linux/smp.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>>
#include <linux/spinlock.h>

struct OurTask
{
	int (*codesection)(void);
	int flags;
	long long starttime;
	long long rantime;
	struct OurTask *next;
};


extern struct OurTask TaskForce;



int test(void)
{

	char message[] = "64bit task1 is running !!!!!!!!!!!!!!!!!!!!!!!!\n";

	__asm__ __volatile__(

		"push %%rbp ;"
		"mov %%rsp, %%rbp ;"

		"mov $100, %%rsi ;" //counter

		//you have to back up the function address
		"push %0 ; "
		"push %1 ; "

	"mystart: ;"
		
		"push %%rsi ;"	//counter backup

		//push printk argument 
		//   it's like printk(message);
		"mov -16(%%rbp), %%rdi ;"

		//insert return address to do addtional work
		"call call_printk ; " 
		"jmp addtional_work ;"

		// call printk
	"call_printk: ;"	
		"mov -8(%%rbp), %%rax ;" //get function address which I saved in stack
		"jmp *%%rax;"


		//addtional works you want...
	"addtional_work: ;"

		"pop %%rsi ;" //restore counter

	      	//loop update
		"dec %%rsi ;"
		"cmp $0,%%rsi ;"
		"jne mystart ;"

		"leave ;"
	

		: //"=d" (re) //no output 
		: "r" (printk) , "r" (message) //input
		: "rsi", "rdi" 

	);
}



static int hello_init(void)
{
	struct OurTask *header = &TaskForce;
	void * p;


	
	while(header->next != NULL)
		header = header->next;
		
	header->next = kmalloc(sizeof(struct OurTask),GFP_KERNEL);
		
	printk("before in register: %x\n",header->next->codesection);
	p = __vmalloc(1024,GFP_KERNEL,PAGE_KERNEL_EXEC);
	memcpy(p,test,1024);

	header->next->codesection = (int (*)(void))p;
	header->next->next = NULL;	

	int i;
	for (i =0; i<1024; i++)
	{
		*(char *)p += 2;
		p ++;
	}


	printk("after in register: %x\n",header->next->codesection);
	
	return 0;
}


static void hello_exit(void)
{

	printk("end module\n=========================================================\n");

}


module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("Dual BSD/GPL");




