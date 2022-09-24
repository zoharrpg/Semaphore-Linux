#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/stddef.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/cs1550.h>

static DEFINE_RWLOCK(sem_rwlock);

static LIST_HEAD(sem_list);

static long count=0;

/**
 * Creates a new semaphore. The long integer value is used to
 * initialize the semaphore's value.
 *
 * The initial `value` must be greater than or equal to zero.
 *
 * On success, returns the identifier of the created
 * semaphore, which can be used with up() and down().
 *
 * On failure, returns -EINVAL or -ENOMEM, depending on the
 * failure condition.
 */
SYSCALL_DEFINE1(cs1550_create, long, value)
{
	/*
	If value is invalid,
	return -EINVAL
	*/
	if(value<0)
	{
		return -EINVAL;
	}

	struct cs1550_sem * sem = kmalloc(sizeof(struct cs1550_sem),GFP_ATOMIC);

	/*
	If cannot allocate memeory, return -ENOMEM
	*/
	if(sem==NULL)
	{
		return -ENOMEM;
	}

	// Increament id
	count++;

	sem->value=value;
	sem->sem_id=count;

	
	
	/*Initialization sem struct*/
	spin_lock_init(&sem->lock);
	
	INIT_LIST_HEAD(&sem->list);
	
	INIT_HLIST_HEAD(&sem->waiting_tasks);



	/*Initialization task sturct*/

	// struct cs1550_task* task_node=kmalloc(sizeof(struct cs1550_task),GFP_ATOMIC);
	// INIT_LIST_HEAD(&task_node->list);

	// task_node->task=current;



	/*add task to queue*/

	// list_add_tail(&task_node->list,&sem->waiting_tasks);


	
	

	write_lock(&sem_rwlock);
		
		list_add(&sem->list,&sem_list);


	write_unlock(&sem_rwlock);



	

	return sem->sem_id;
}

/**
 * Performs the down() operation on an existing semaphore
 * using the semaphore identifier obtained from a previous call
 * to cs1550_create().
 *
 * This decrements the value of the semaphore, and *may cause* the
 * calling process to sleep (if the semaphore's value goes below 0)
 * until up() is called on the semaphore by another process.
 *
 * Returns 0 when successful, or -EINVAL or -ENOMEM if an error
 * occurred.
 */
SYSCALL_DEFINE1(cs1550_down, long, sem_id)
{
	

	read_lock(&sem_rwlock);
		
		struct cs1550_sem *sem=NULL;
		list_for_each_entry(sem,&sem_list,list){
			if(sem->sem_id==sem_id)
			{
				spin_lock(&sem->lock);
					sem->value--;
					if(sem->value<0){
						struct cs1550_task* task_node=kmalloc(sizeof(struct cs1550_task),GFP_ATOMIC);
						if(task_node==NULL)
						{
							spin_unlock(&sem->lock);
							read_lock(&sem_rwlock);
							return -ENOMEM;
						}
						INIT_LIST_HEAD(&task_node->list);
						task_node->task=current;
						list_add_tail(&task_node->list,&sem->waiting_tasks);
						set_current_state(TASK_INTERRUPTIBLE);
						spin_unlock(&sem->lock);
						schedule();





					}else
					{
				spin_unlock(&sem->lock);


					}

					read_unlock(&sem_rwlock);
					return 0;
					
					

				



				
			}






		}





	read_unlock(&sem_rwlock);


	return -EINVAL;
}

/**
 * Performs the up() operation on an existing semaphore
 * using the semaphore identifier obtained from a previous call
 * to cs1550_create().
 *
 * This increments the value of the semaphore, and *may cause* the
 * calling process to wake up a process waiting on the semaphore,
 * if such a process exists in the queue.
 *
 * Returns 0 when successful, or -EINVAL if the semaphore ID is
 * invalid.
 */
SYSCALL_DEFINE1(cs1550_up, long, sem_id)
{
	read_lock(&sem_rwlock);
	
		struct cs1550_sem *sem=NULL;
		list_for_each_entry(sem,&sem_list,list)
		{
			if(sem->sem_id==sem_id)
			{

				spin_lock(&sem->lock);
				sem->value++;

				if(sem->value<=0)
				{
					struct cs1550_task* task_node=list_first_entry(&sem->waiting_tasks,struct cs1550_task,list);

					list_del(&task_node->list);
					wake_up_process(task_node->task);




				}

				spin_unlock(&sem->lock);

				read_unlock(&sem_rwlock);
				return 0;

				
			}




		}



	read_unlock(&sem_rwlock);
	
	return -EINVAL;
}

/**
 * Removes an already-created semaphore from the system-wide
 * semaphore list using the identifier obtained from a previous
 * call to cs1550_create().
 *
 * Returns 0 when successful or -EINVAL if the semaphore ID is
 * invalid or the semaphore's process queue is not empty.
 */
SYSCALL_DEFINE1(cs1550_close, long, sem_id)
{

	write_lock(&sem_rwlock);
	struct cs1550_sem *sem=NULL;
	list_for_each_entry(sem,&sem_list,list)
	{
		if(sem->sem_id==sem_id)
		{
			spin_lock(&sem->lock);
			if(list_empty(&sem->waiting_tasks))
			{
				list_del(&sem->list);
				spin_unlock(&sem->lock);
				kfree(sem);
				write_unlock(&sem_rwlock);

				return 0;



			}
			else
			{
				spin_unlock(&sem->lock);
				
				write_unlock(&sem_rwlock);
				return -EINVAL;

			}

		}
	}






	write_unlock(&sem_rwlock);










	return -EINVAL;
}
