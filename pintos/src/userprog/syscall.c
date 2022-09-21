#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);
void address_protect (void * addr)
{
  if (!is_user_vaddr(addr))
  {
    exit(-1);
  }
}
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  uint32_t* args = f->esp;
  switch(*(uint32_t*)(f->esp)) {
    case SYS_EXIT:
      address_protect(f->esp + 4);
      exit((int) * (uint32_t*)(f->esp+ 4));
      break;
  }
  thread_exit ();
}
