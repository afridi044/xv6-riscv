#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if (addr >= p->sz || addr + sizeof(uint64) > p->sz) // both tests needed, in case of overflow
    return -1;
  if (copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  if (copyinstr(p->pagetable, buf, addr, max) < 0)
    return -1;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n)
  {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
void argint(int n, int *ip)
{
  *ip = argraw(n);
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
void argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int argstr(int n, char *buf, int max)
{
  uint64 addr;
  argaddr(n, &addr);
  return fetchstr(addr, buf, max);
}

// Prototypes for the functions that handle system calls.
extern uint64 sys_fork(void);
extern uint64 sys_exit(void);
extern uint64 sys_wait(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_kill(void);
extern uint64 sys_exec(void);
extern uint64 sys_fstat(void);
extern uint64 sys_chdir(void);
extern uint64 sys_dup(void);
extern uint64 sys_getpid(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_uptime(void);
extern uint64 sys_open(void);
extern uint64 sys_write(void);
extern uint64 sys_mknod(void);
extern uint64 sys_unlink(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_close(void);
extern uint64 sys_trace(void);
extern uint64 sys_info(void);
extern uint64 sys_command_history(void);
extern uint64 sys_get_command(void);

// An array mapping syscall numbers from syscall.h
// to the function that handles the system call.
static uint64 (*syscalls[])(void) = {
    [SYS_fork] sys_fork,
    [SYS_exit] sys_exit,
    [SYS_wait] sys_wait,
    [SYS_pipe] sys_pipe,
    [SYS_read] sys_read,
    [SYS_kill] sys_kill,
    [SYS_exec] sys_exec,
    [SYS_fstat] sys_fstat,
    [SYS_chdir] sys_chdir,
    [SYS_dup] sys_dup,
    [SYS_getpid] sys_getpid,
    [SYS_sbrk] sys_sbrk,
    [SYS_sleep] sys_sleep,
    [SYS_uptime] sys_uptime,
    [SYS_open] sys_open,
    [SYS_write] sys_write,
    [SYS_mknod] sys_mknod,
    [SYS_unlink] sys_unlink,
    [SYS_link] sys_link,
    [SYS_mkdir] sys_mkdir,
    [SYS_close] sys_close,
    [SYS_trace] sys_trace,
    [SYS_info] sys_info,
    [SYS_command_history] sys_command_history,
    [SYS_get_command] sys_get_command,
};

// making an array of the system calls names
char *syscall_names[] = {
    [SYS_fork] = "fork",
    [SYS_exit] = "exit",
    [SYS_wait] = "wait",
    [SYS_pipe] = "pipe",
    [SYS_read] = "read",
    [SYS_kill] = "kill",
    [SYS_exec] = "exec",
    [SYS_fstat] = "fstat",
    [SYS_chdir] = "chdir",
    [SYS_dup] = "dup",
    [SYS_getpid] = "getpid",
    [SYS_sbrk] = "sbrk",
    [SYS_sleep] = "sleep",
    [SYS_uptime] = "uptime",
    [SYS_open] = "open",
    [SYS_write] = "write",
    [SYS_mknod] = "mknod",
    [SYS_unlink] = "unlink",
    [SYS_link] = "link",
    [SYS_mkdir] = "mkdir",
    [SYS_close] = "close",
    [SYS_trace] = "trace",
    [SYS_info] = "info",
    [SYS_command_history] = "command_history",
    [SYS_get_command] = "get_command",
};

void syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if (num > 0 && num < NELEM(syscalls) && syscalls[num])
  {
    // Use num to lookup the system call function for num, call it,
    // and store its return value in p->trapframe->a0

    char *path = "";                           // path to the file
    void *addr;                                // buffer
    int n1, n2;                                // for the arguments of the system call
    uint64 firstArg = argraw(0);               // first argument of the system call is saved
    p->trapframe->a0 = syscalls[num]();

    if (num == p->traced_syscall)
    {

      printf("pid: %d, syscall: %s, ", p->pid, syscall_names[num]);
      // print the arguments of the system call
      // like this args: (3, 0x0000000000001010, 1023)

      switch (num)
      {

      case SYS_fork:
        printf("args: ( )");
        break;

      case SYS_exit:
        n1 = firstArg;
        printf("args: (%d)", n1);
        break;

      case SYS_wait:
        addr = (void *)firstArg;
        printf("args: (%p)", addr);
        break;

      case SYS_pipe:
        addr = (void *)firstArg;
        printf("args: (%p)", addr);
        break;

      case SYS_read:
        n1 = firstArg;
        argaddr(1, (uint64 *)&addr);
        argint(2, &n2);
        printf("args: (%d, %p, %d)", n1, addr, n2);
        break;

      case SYS_kill:
        n1 = firstArg;
        printf("args: (%d)", n1);
        break;

      case SYS_exec:
    
        if(fetchstr(firstArg, path, MAXPATH) < 0){
          printf("error in fetchstr");
        }
        argaddr(1, (uint64 *)&addr);
        printf("args: (%s, %p)", path, addr);
        break;

      case SYS_fstat:

        n1 = firstArg;
        argaddr(1, (uint64 *)&addr);
        printf("args: (%d, %p)", n1, addr);
        break;

      case SYS_chdir:

        fetchstr(firstArg, path, MAXPATH);
        printf("args: (%s)", path);
        break;

      case SYS_dup:

        n1 = firstArg;
        printf("args: (%d)", n1);
        break;

      case SYS_getpid:
        printf("args: ( )");
        break;

      case SYS_sbrk:

        n1 = firstArg;
        printf("args: (%d)", n1);
        break;

      case SYS_sleep:

        n1 = firstArg;
        printf("args: (%d)", n1);
        break;

      case SYS_uptime:

        printf("args: ( )");
        break;

      case SYS_open:

        fetchstr(firstArg, path, MAXPATH);
        argint(1, &n1);
        printf("args: (%s, %d)", path, n1);
        break;

      case SYS_write:

        n1 = firstArg;
        argaddr(1, (uint64 *)&addr);
        argint(2, &n2);
        printf("args: (%d, %p, %d)", n1, addr, n2);
        break;

      case SYS_mknod:

        fetchstr(firstArg, path, MAXPATH);
        argint(1, &n1);
        argint(2, &n2);
        printf("args: (%s, %d, %d)", path, n1, n2);
        break;

      case SYS_unlink:

        fetchstr(firstArg, path, MAXPATH);
        printf("args: (%s)", path);
        break;

      case SYS_link:

        char *path2 = ""; // path to the file
        fetchstr(firstArg, path, MAXPATH);
        argstr(1, path2, MAXPATH);
        printf("args: (%s, %s)", path, path2);
        break;

      case SYS_mkdir:

        fetchstr(firstArg, path, MAXPATH);
        printf("args: (%s)", path);
        break;

      case SYS_close:

        n1 = firstArg;
        printf("args: (%d)", n1);
        break;

      case SYS_trace:

        n1 = firstArg;
        printf("args: (%d)", n1);
        break;

      case SYS_info:

        addr = (void *)firstArg;
        printf("args: (%p)", addr);
        break;

      case SYS_command_history:

        addr = (void *)firstArg;
        printf("args: (%p)", addr);
        break;

      case SYS_get_command:

        n1 = firstArg;
        argaddr(1, (uint64 *)&addr);
        printf("args: (%d, %p)", n1, addr);
        break;

      default:
        break;
      }

      printf(", return: %lu \n", p->trapframe->a0);
    }
  }
  else
  {
    printf("%d %s: unknown sys call %d\n",
           p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
