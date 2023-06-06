#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

// for mp3
uint64
sys_thrdstop(void)
{
  int delay;
  uint64 context_id_ptr;
  uint64 handler, handler_arg;
  if (argint(0, &delay) < 0)
    return -1;
  if (argaddr(1, &context_id_ptr) < 0)
    return -1;
  if (argaddr(2, &handler) < 0)
    return -1;
  if (argaddr(3, &handler_arg) < 0)
    return -1;

  struct proc *proc = myproc();

  //TODO: mp3
  proc -> delay = delay;
  proc -> ticks = 0;
  proc -> handler = handler;
  proc -> handler_arg = handler_arg;

  int request;
  copyin(proc -> pagetable, (char * )&request, context_id_ptr, sizeof(int));

  if (request == -1){
    for (int i = 0; i < MAX_THRD_NUM; i++){
      if ((proc -> used_context)[i] == 0){
        proc -> context_id = i;
	(proc -> used_context)[i] = 1;
	copyout(proc -> pagetable, context_id_ptr, (char *)&i, sizeof(int));
	break;
      }
    }
  }
  else if (request >= 0 && request < MAX_THRD_NUM){ 
      if ((proc -> used_context)[request] == 0)
	return -1;
      proc -> context_id = request;
  }
  else
    return -1;

  return 0;
}

// for mp3
uint64
sys_cancelthrdstop(void)
{
  int context_id, is_exit;
  if (argint(0, &context_id) < 0)
    return -1;
  if (argint(1, &is_exit) < 0)
    return -1;

  if (context_id < 0 || context_id >= MAX_THRD_NUM) {
    return -1;
  }

  struct proc *proc = myproc();

  //TODO: mp3
  int ticks = proc -> ticks;
  proc -> delay = -1;

  if (is_exit == 0){
    memmove(&proc -> context_reg[context_id], proc -> trapframe, sizeof(struct trapframe));
  }
  else{
    (proc -> used_context)[context_id] = 0;
  }

  return ticks;
}

// for mp3
uint64
sys_thrdresume(void)
{
  int context_id;
  if (argint(0, &context_id) < 0)
    return -1;

  struct proc *proc = myproc();

  if (context_id >= MAX_THRD_NUM || (proc -> used_context)[context_id] == 0)
    return -1;

  //TODO: mp3
  memmove(proc -> trapframe, &(proc -> context_reg)[context_id], sizeof(struct trapframe));
  return 0;
}
