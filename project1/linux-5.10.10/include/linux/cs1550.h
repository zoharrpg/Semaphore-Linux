#ifndef _LINUX_CS1550_H
#define _LINUX_CS1550_H

#include <linux/list.h>

/**
 * A generic semaphore, providing serialized signaling and
 * waiting based upon a user-supplied integer value.
 */
struct cs1550_sem
{
	/* Current value. If nonpositive, wait() will block */
	long value;

	/* Sequential numeric ID of the semaphore */
	long sem_id;

	/* Per-semaphore lock, serializes access to value */
	spinlock_t lock;
	
	/*sem_list*/
	struct list_head list;
	/*Task list*/
	struct list_head waiting_tasks;




	
};

struct cs1550_task{

	/*Task list*/
	struct list_head list;

	/*Task pinter*/
	struct task_struct *task;


};

#endif
