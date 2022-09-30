#include "threads/malloc.h"
#include "userprog/syscall.h"
#include "userprog/process.h"
#include "user/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "devices/input.h"
#include "devices/shutdown.h"
#include <string.h>


void address_protect (const void * vaddr);
static void syscall_handler (struct intr_frame *);
void halt(void);
void exit(int status);
int wait(pid_t pid);
pid_t exec(const char* file_name);
int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);

void address_protect (const void * vaddr)
{
  if (!is_user_vaddr(vaddr))
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
  //printf ("system call!\n");
  uint32_t* args = f->esp;
  //hex_dump(f->esp, f->esp, 100,1);
  //printf("%d\n", *args);
  address_protect(f->esp);
  switch(*args) {
    case SYS_HALT:
      halt();
      break;
    case SYS_EXIT:
      address_protect(f->esp+4);
      exit((int)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_EXEC:
      address_protect(f->esp+4);
      f->eax = exec((const char *)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_WAIT:
      address_protect(f->esp+4);
      f->eax = wait((pid_t)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_WRITE:
      address_protect(f->esp+4);
      address_protect(f->esp+8);
      address_protect(f->esp+12);
      f->eax = write((int)*(uint32_t *)(f->esp + 4), (const void *)*(uint32_t *)(f->esp + 8), (unsigned)*(uint32_t *)(f->esp + 12));
      break;
    case SYS_READ:
      address_protect(f->esp+4);
      address_protect(f->esp+8);
      address_protect(f->esp+12);
      f->eax = read((int)*(uint32_t *)(f->esp + 4), (void *)*(uint32_t *)(f->esp + 8), (unsigned)*(uint32_t *)(f->esp + 12));
      break; 
  }
}
int read (int fd, void *buffer, unsigned length) {
  int i;
  if (fd == 0) {
    for (i=0;i<(int)length;i++) {
      if(((char*)buffer)[i]=='\0') break;
    }
    return i;
  }
  return -1;
}
int write (int fd, const void *buffer, unsigned length) {
  if (fd == 1) {
  //printf("write fd : %d, buffer : %s, length : %d\n", (int)fd, (char *)buffer, (int)length);
  //printf("buffer : %s\n", (char *)buffer);
    putbuf(buffer, length);
    return length;
  }
  return -1;
}
int wait(pid_t pid) {
  return process_wait(pid);
}
pid_t exec(const char* file_name) {
  char *parse_ptr;
  char *save_ptr;
  char *r_file_name = (char *)malloc(sizeof(char)*(strlen(file_name)+2));
  strlcpy(r_file_name, file_name, strlen(file_name)+1);
  parse_ptr = strtok_r(r_file_name, " ", &save_ptr);
  if(filesys_open(parse_ptr) == NULL) return -1;
  free(r_file_name);
  return (pid_t)process_execute(file_name);
}
void exit (int status) {
  struct thread *t = thread_current();
  t->exit_status = status;
  char *parse_ptr;
  char *save_ptr;
  char *process_name = (char *)malloc((sizeof(char)*strlen(t->name)+2));
  strlcpy(process_name, t->name, strlen(t->name)+1);
  parse_ptr = strtok_r(process_name, " ", &save_ptr);
  printf("%s: exit(%d)\n",parse_ptr,status);
  free(process_name);
  thread_exit();
}
void halt(void) {
  shutdown_power_off();
}