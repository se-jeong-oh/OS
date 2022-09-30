#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void address_protect (const void * vaddr);
//static void syscall_handler (struct intr_frame *);
void halt(void);
void exit(int status);
//int wait(pid_t pid);
//pid_t exec(const char* file_name);
int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);

#endif /* userprog/syscall.h */
