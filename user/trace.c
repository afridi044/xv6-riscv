#include "kernel/types.h"
#include "user/user.h"


int main(int argc, char *argv[]) {
   if (argc < 3 ){
      printf("Usage: trace <sys_call number> <command> <args>\n");
      exit(1);
   }
   int sysnum = atoi(argv[1]);
   trace(sysnum);
   exec(argv[2], &argv[2]);
   exit(0);  // not reached if exec succeeds
}