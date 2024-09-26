#include "kernel/types.h"
#include "user/user.h"



int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        printf("Usage: load <childCount> <allocationAmount>\n");

        exit(1);
    }

    int childCount = atoi(argv[1]);
    int allocationAmount = atoi(argv[2]);
    int i;
    printf("Parent going to sleep.\n");

    for (i = 0; i < childCount; i++)
    {
        int pid = fork();
        if (pid == 0)
        { // we are in the child process
            printf("Child %d is created.\n", i+1);

            char *ptr = malloc(allocationAmount);
            if (ptr == 0)
            {
                printf("Child %d failed to allocate RAM.\n", i+1);

                exit(1);
            }

            printf("Child %d allocated %d bytes of RAM .\n", i+1, allocationAmount);
            printf("Child %d is going to sleep.\n", i+1);
            sleep(1000);
            exit(0);
        }
        else if (pid > 0)
        {
            sleep(10);
            continue;
        }

        else
        {
            printf("Fork failed.\n");

            exit(1);
        }
    }

    sleep(100);
    
 
    printf("Parent wake up.\n");

    struct procInfo pInfo;
    if (info(&pInfo) == 0)
    {

        printf("Current system information:\n");
        printf("Processes: %d/%d\n", pInfo.activeProcess, pInfo.totalProcess);
        int usedMem = pInfo.memsize;
        if (usedMem < 1024)
        {
            // Print in bytes
            printf("RAM: %d bytes/%d MB\n", usedMem, pInfo.totalMemSize / (1024 * 1024));
        }
        else if (usedMem < 1024 * 1024)
        {
            // Converting to KB and multiply by 100 for two decimal places
            int usedMemKB = (usedMem * 100) / 1024;
            printf("RAM: %d.%d KB/%d MB\n", usedMemKB / 100, usedMemKB % 100, pInfo.totalMemSize / (1024 * 1024));
        }
        else
        {
            // Converting to MB and multiply by 100 for two decimal places
            int usedMemMB = (usedMem * 100) / (1024 * 1024);
            printf("RAM: %d.%d MB/%d MB\n", usedMemMB / 100, usedMemMB % 100, pInfo.totalMemSize / (1024 * 1024));
        }
    }

    else
    {
        printf("Failed to get system info.\n");
    }

   

    exit(0);
}