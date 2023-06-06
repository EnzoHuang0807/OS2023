#include "kernel/types.h"
#include "user/user.h"
#include "user/list.h"
#include "user/threads.h"
#include "user/threads_sched.h"

#define NULL 0

/* default scheduling algorithm */
struct threads_sched_result schedule_default(struct threads_sched_args args)
{
    struct thread *thread_with_smallest_id = NULL;
    struct thread *th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_with_smallest_id == NULL || th->ID < thread_with_smallest_id->ID) {
            thread_with_smallest_id = th;
        }
    }

    struct threads_sched_result r;
    if (thread_with_smallest_id != NULL) {
        r.scheduled_thread_list_member = &thread_with_smallest_id->thread_list;
        r.allocated_time = thread_with_smallest_id->remaining_time;
    } else {
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = 1;
    }

    return r;
}

/* Earliest-Deadline-First scheduling */
struct threads_sched_result schedule_edf(struct threads_sched_args args)
{
    struct thread * thread_deadline = NULL;
    struct thread * thread_edf = NULL;
    struct thread * th = NULL;

    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_edf == NULL || th -> current_deadline < thread_edf -> current_deadline || 
	   (th -> current_deadline == thread_edf -> current_deadline && th -> ID < thread_edf -> ID)) {
            thread_edf = th;
        }

	//check missing deadline
	if (args.current_time > th -> current_deadline ||
	   (args.current_time == th -> current_deadline && th -> remaining_time > 0)){
           if (thread_deadline == NULL || th -> ID < thread_deadline -> ID)
	     thread_deadline = th;
	}
    }

    if (thread_deadline != NULL){
      struct threads_sched_result r;
      r.scheduled_thread_list_member = &thread_deadline -> thread_list;
      r.allocated_time = 0;
      return r;
    }

    //debug message
    //printf("current time: %d\n", args.current_time); 
    //printf("thread id = %d\n", thread_edf -> ID);

    //find next release thread / next release thread with earilier deadline in release_queue
    struct release_queue_entry * next = NULL;
    struct release_queue_entry * entry_preempt = NULL;
    struct release_queue_entry * ent = NULL;
    list_for_each_entry(ent, args.release_queue, thread_list){

        if (next == NULL || ent -> release_time < next -> release_time || 
	    (ent -> release_time == next -> release_time && ent -> thrd -> ID < next -> thrd -> ID))
          next = ent;

        if (thread_edf != NULL && ent -> release_time + ent -> thrd -> period < thread_edf -> current_deadline) {
	  if (entry_preempt == NULL || ent -> release_time < entry_preempt -> release_time ||
	      (ent -> release_time == entry_preempt -> release_time && 
	       ent -> thrd -> ID < entry_preempt -> thrd -> ID))
            entry_preempt = ent;
        }
    }

    struct threads_sched_result r;
    if (thread_edf != NULL) {
        r.scheduled_thread_list_member = &thread_edf -> thread_list;
        r.allocated_time = thread_edf -> remaining_time;

	if (r.allocated_time > thread_edf -> current_deadline - args.current_time){
	  // current thread terminated by own deadline
	  r.allocated_time = thread_edf -> current_deadline - args.current_time;
	}
	if(entry_preempt != NULL && 
	    r.allocated_time > entry_preempt -> release_time - args.current_time){
	  // some thread will be released later but have eairlier deadline
	  r.allocated_time = entry_preempt -> release_time - args.current_time;
	}
    } 
    else {
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = next -> release_time - args.current_time;
    }

    return r;
}

/* Rate-Monotonic Scheduling */
struct threads_sched_result schedule_rm(struct threads_sched_args args)
{
    struct thread * thread_deadline = NULL;
    struct thread * thread_rm = NULL;
    struct thread * th = NULL;

    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_rm == NULL || th -> period < thread_rm -> period || 
	   (th -> period == thread_rm -> period && th -> ID < thread_rm -> ID)) {
            thread_rm = th;
        }

	//check missing deadline
	if (args.current_time > th -> current_deadline ||
	   (args.current_time == th -> current_deadline && th -> remaining_time > 0)){
           if (thread_deadline == NULL || th -> ID < thread_deadline -> ID)
	     thread_deadline = th;
	}
    }

    if (thread_deadline != NULL){
      struct threads_sched_result r;
      r.scheduled_thread_list_member = &thread_deadline -> thread_list;
      r.allocated_time = 0;
      return r;
    }

    //debug message
    //printf("current time: %d\n", args.current_time); 
    //printf("thread id = %d\n", thread_edf -> ID);

    //find next release thread / next release thread with smaller period
    struct release_queue_entry * next = NULL;
    struct release_queue_entry * entry_preempt = NULL;
    struct release_queue_entry * ent = NULL;
    list_for_each_entry(ent, args.release_queue, thread_list){

        if (next == NULL || ent -> release_time < next -> release_time || 
	    (ent -> release_time == next -> release_time && ent -> thrd -> ID < next -> thrd -> ID))
          next = ent;

        if (thread_rm != NULL && ent -> release_time < thread_rm -> current_deadline && 
	    ent -> thrd -> period < thread_rm  -> period) {

	  if (entry_preempt == NULL || ent -> release_time < entry_preempt -> release_time ||
	      (ent -> release_time == entry_preempt -> release_time && 
	       ent -> thrd -> ID < entry_preempt -> thrd -> ID))
            entry_preempt = ent;
        }
    }

    struct threads_sched_result r;
    if (thread_rm != NULL) {
        r.scheduled_thread_list_member = &thread_rm -> thread_list;
        r.allocated_time = thread_rm -> remaining_time;

	if (r.allocated_time > thread_rm -> current_deadline - args.current_time){
	  // current thread terminated by own deadline
	  r.allocated_time = thread_rm -> current_deadline - args.current_time;
	}
	if(entry_preempt != NULL && 
	    r.allocated_time > entry_preempt -> release_time - args.current_time){
	  // some thread will be released later but have eairlier deadline
	  r.allocated_time = entry_preempt -> release_time - args.current_time;
	}
    } 
    else {
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = next -> release_time - args.current_time;
    }

    return r;
}
