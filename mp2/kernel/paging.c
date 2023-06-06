#include "param.h"

#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

/* NTU OS 2022 */
/* Page fault handler */
int handle_pgfault() {
  /* Find the address that caused the fault */
  uint64 va = r_stval(); 
  va = PGROUNDDOWN(va);

  /* TODO */
  void * pa = kalloc();
  memset(pa, 0, PGSIZE);
  pagetable_t pagetable = myproc() -> pagetable;

  pte_t * pte = walk(pagetable, va, 0);

  if (*pte & PTE_S){
    
	uint blockno = PTE2BLOCKNO(*pte);
        mappages(pagetable, va, PGSIZE, (uint64)pa, PTE_FLAGS(*pte) & ~PTE_S);

	begin_op();
        read_page_from_disk(ROOTDEV, (char *)pa , blockno);
        bfree_page(ROOTDEV, blockno);
        end_op();
  }
  else{
    uint64 perm = PTE_U | PTE_R | PTE_W | PTE_X;
    mappages(pagetable, PGROUNDDOWN(va), PGSIZE, (uint64)pa, perm);
  }
  return 0;
}
