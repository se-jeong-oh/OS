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
#include "filesys/file.h"
#include "devices/input.h"
#include "devices/shutdown.h"
#include <string.h>


struct lock lock_file;
static void syscall_handler (struct intr_frame *);
int wait(pid_t pid);
pid_t exec(const char* file_name);
void address_protect (const void * vaddr);
void halt(void);
void exit(int status);
int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);
void isvalid_fd(int fd);

void address_protect (const void * vaddr)
{
  if (!is_user_vaddr(vaddr))
  {
    if (lock_held_by_current_thread(&lock_file))
      lock_release (&lock_file);
    exit(-1);
  }
}

void isvalid_fd(int fd) {
  if(thread_current()->fd[fd]==NULL) exit(-1);
  return;
}

void
syscall_init (void) 
{
  lock_init(&lock_file);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //lock_init(&lock_file);
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
    case SYS_CREATE:
      address_protect(f->esp+4);
      address_protect(f->esp+8);
      f->eax = create((const char *)*(uint32_t *)(f->esp+4), (unsigned)*(uint32_t *)(f->esp+8));
      break;
    case SYS_REMOVE:
      address_protect(f->esp+4);
      f->eax = remove((const char *)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_OPEN:
      address_protect(f->esp+4);
      f->eax = open((const char *)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_FILESIZE:
      address_protect(f->esp+4);
      f->eax = filesize((int)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_SEEK:
      address_protect(f->esp+4);
      address_protect(f->esp+8);
      seek((int)*(uint32_t *)(f->esp+4), (unsigned)*(uint32_t *)(f->esp+8));
      break;
    case SYS_TELL:
      address_protect(f->esp+4);
      f->eax = tell((int)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_CLOSE:
      address_protect(f->esp+4);
      close((int)*(uint32_t *)(f->esp + 4));
      break;
  }
}
bool create (const char *file, unsigned initial_size) {
  bool check;
  if (file != NULL) {
    address_protect(file);
    check = filesys_create(file, initial_size);
    return check;
  }
  else exit(-1);

}
bool remove (const char *file) {
  if (file != NULL) {
    address_protect(file);
    return filesys_remove(file);
  }
  else exit(-1);
}
int open (const char *file) {
  if (file == NULL) exit(-1);
  address_protect(file);
  lock_acquire(&lock_file);
  int i, idx = -1;
  struct file *file_open = filesys_open(file);
  struct thread *t = thread_current();
  char *parse_ptr;
  char *save_ptr;
  char cp_fn[16];
  strlcpy(cp_fn, t->name, strlen(t->name));
  parse_ptr = strtok_r(cp_fn, " ", &save_ptr);
  if (file_open == NULL) {
    lock_release(&lock_file);
    return -1;
  }
  //printf("%s %s\n", thread_name(), file);

  for (i=3;i<128;i++) {
    if(t->fd[i] == NULL){
      if(strcmp(t->name, file) == 0)
        file_deny_write(file_open);
      t->fd[i] = file_open;
      idx = i;
      break;
    }
  }
  lock_release(&lock_file);
  return idx;
}
int filesize (int fd) {
  int len;
  isvalid_fd(fd);
  len = (int)file_length(thread_current()->fd[fd]);
  return len;
}
void seek (int fd, unsigned position) {
  struct thread *t = thread_current();
  isvalid_fd(fd);
  file_seek(t->fd[fd], position);
}
unsigned tell (int fd) {
  struct thread *t = thread_current();
  isvalid_fd(fd);
  return (unsigned)file_tell(t->fd[fd]);
}
void close (int fd) {
  struct thread *t = thread_current();
  if(t->fd[fd] == NULL) exit(-1);
  t->fd[fd] = NULL;
  file_close(t->fd[fd]);
}

int read (int fd, void *buffer, unsigned length) {
  int i = -1;
  struct thread *curr = thread_current();
  address_protect(buffer);
  lock_acquire(&lock_file);
  if (fd == 0) {
    for (i=0;i<(int)length;i++) {
      if(((char*)buffer)[i]=='\0') break;
    }
    lock_release(&lock_file);
    return i;
  }
  else if (fd > 2 && fd < 128) {
   // except STDIN, STDOUT, STDERR
    if(curr->fd[fd] == NULL) {
      lock_release(&lock_file);
      exit(-1);
    }
    i = file_read(curr->fd[fd], buffer, length);
  }
  lock_release(&lock_file);
  return i;
}
int write (int fd, const void *buffer, unsigned length) {
  address_protect(buffer);
  lock_acquire(&lock_file);
  int len = -1;
  if (fd == 1) {
  //printf("write fd : %d, buffer : %s, length : %d\n", (int)fd, (char *)buffer, (int)length);
  //printf("buffer : %s\n", (char *)buffer);
    putbuf(buffer, length);
    len = length;
    lock_release(&lock_file);
    return len;
  }
  else if (fd > 2 && fd < 128) {
    struct thread *curr = thread_current();
    struct file *curr_file = curr->fd[fd];
    if (curr_file==NULL) {
      lock_release(&lock_file);
      exit(-1);
    }
    if(curr_file->deny_write) {
      file_deny_write(curr_file);
      //len = 0;
    }
    len = file_write(curr_file, buffer, length);
  }
  lock_release(&lock_file);
  return len;
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
  int i;
  struct thread *t = thread_current();
  struct list_elem* elem = NULL;
  struct thread *curr_th = NULL;
  t->exit_status = status;
  char *parse_ptr;
  char *save_ptr;
  char *process_name = (char *)malloc((sizeof(char)*strlen(t->name)+2));
  strlcpy(process_name, t->name, strlen(t->name)+1);
  parse_ptr = strtok_r(process_name, " ", &save_ptr);
  printf("%s: exit(%d)\n",parse_ptr,status);
  free(process_name);
  for(i=3;i<128;i++) 
    if(t->fd[i] != NULL) close(i); // if not closed fild exists, close it. 
  for (elem = list_begin(&(thread_current()->child)); elem != list_end(&(thread_current()->child)); elem = list_next(elem)) {
    curr_th = list_entry(elem, struct thread, child_elem);
    process_wait(curr_th->tid);
  }  
  thread_exit();
}
void halt(void) {
  shutdown_power_off();
}