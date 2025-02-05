#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_trace(void)
{

  // printf("calling sys_trace for pid %d\n", myproc()->pid);
  int target_syscall; // syscall number to trace
  argint(0, &target_syscall);

  if (target_syscall < 0)
  {
    return -1; // invalid syscall number
  }

  myproc()->traced_syscall = target_syscall; // setting the traced syscall number

  return 0;
}

uint64
sys_info(void)
{
  uint64 result = info();
  return result;
}

uint64
sys_command_history(void)
{
  char history[MAXLINE];

  uint64 addr;
  argaddr(0, &addr);

  // Copying the user-provided data into the kernel buffer
  if (copyin(myproc()->pagetable, history, addr, MAXLINE) < 0)
    return -1;          
  update_command_history(history); 

  return 0; 
}

uint64
sys_get_command(void)
{
  int n;
  uint64 addr;
  argint(0, &n);
  argaddr(1, &addr);
  char command[MAXLINE];
  
  if(get_latest_command(n, command) == -1)
    return -1;  

  // Copying the command to the user-provided buffer
  if (copyout(myproc()->pagetable, addr, command, MAXLINE) < 0)
    return -1;  

  return 0; 
}
