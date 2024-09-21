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
            printf("Child %d is created.\n", i);

            char *ptr = malloc(allocationAmount);
            if (ptr == 0)
            {
                printf("Child %d failed to allocate memory.\n", i);

                exit(1);
            }

            printf("Child %d allocated %d bytes of memory at address %p.\n", i, allocationAmount, ptr);
            printf("Child %d is going to sleep.\n", i);
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
            printf("Memory: %d bytes/%d MB\n", usedMem, pInfo.totalMemSize / (1024 * 1024));
        }
        else if (usedMem < 1024 * 1024)
        {
            printf("Memory: %d KB/%d MB\n", usedMem / 1024, pInfo.totalMemSize / (1024 * 1024));
        }
        else
        {
            printf("Memory: %d MB/%d MB\n", usedMem / (1024 * 1024), pInfo.totalMemSize / (1024 * 1024));
        }
    }

    else
    {
        printf("Failed to get system info.\n");
    }

    exit(0);
}