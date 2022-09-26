#include "userprog/syscall.h"
#include "userprog/process.h"
#include "user/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include <string.h>

static void syscall_handler (struct intr_frame *);
void halt(void);
void exit(int status);
int wait(pid_t pid);
pid_t exec(const char* file_name);
int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);
/*
void address_protect (void * addr)
{
  if (!is_user_vaddr(addr))
  {
    //exit(-1);
  }
}
*/
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
  printf("%d\n", *args);
  switch(*args) {
    case SYS_HALT:
      halt();
      break;
    case SYS_EXIT:
      exit((int)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_EXEC:
      f->eax = exec((const char *)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_WAIT:
      f->eax = wait((pid_t)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_WRITE:
      f->eax = write((int)*(uint32_t *)(f->esp + 4), (void *)*(uint32_t *)(f->esp + 4), (unsigned)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_READ:
      f->eax = read((int)*(uint32_t *)(f->esp + 4), (const void *)*(uint32_t *)(f->esp + 4), (unsigned)*(uint32_t *)(f->esp + 4));
      break;
    
  }
  thread_exit ();
}
int read (int fd, void *buffer, unsigned length) {
  int i;
  if (fd != 0) return -1;
  for (i=0;i<length;i++) {
    if(input_getc()=='\0') break;
  }
  return i;

}
int write (int fd, const void *buffer, unsigned length) {
  if (fd != 1) return -1;
  putbuf(buffer, length);
  return length;
}
int wait(pid_t pid) {
  /*not yet implemented*/
  return process_wait((tid_t)pid);
}
pid_t exec(const char* file_name) {
  /*not yet implemented*/
  printf("%s\n", file_name);
  while(1){}
  pid_t result = (pid_t)process_execute(file_name);
  return 0;
}
void exit (int status) {
  struct thread *t = thread_current();
  char *process_name = (char *)malloc(sizeof(char)*16);
  strlcpy(process_name, t->name, strlen(t->name)+1);
  printf("%s: exit(%d)\n", process_name, status);
  t->status = status;
  thread_exit();
  return;
}
void halt(void) {
  shutdown_power_off();
  return;
}