#include "ipc.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/wait.h>

#define TOBACCO 1
#define PAPER   2
#define GLUE    3

int main() {
    printf("=== Smoking Problem Started ===\n");
    
    // 清理可能残留的信号量
    int semid;
    if ((semid = get_ipc_id("/proc/sysvipc/sem", SUPPLIER_KEY)) >= 0) {
        semctl(semid, 0, IPC_RMID);
        printf("Cleaned existing semaphore %d\n", semid);
    }

    srand(time(NULL));
    printf("Initializing semaphores...\n");
    
    int supplier_sem = set_sem(SUPPLIER_KEY, 1, IPC_CREAT | 0644);
    int smoker_A = set_sem(SMOKER_A_KEY, 0, IPC_CREAT | 0644);
    int smoker_B = set_sem(SMOKER_B_KEY, 0, IPC_CREAT | 0644);
    int smoker_C = set_sem(SMOKER_C_KEY, 0, IPC_CREAT | 0644);

    printf("Semaphores initialized:\n");
    printf("  Supplier: %d\n", supplier_sem);
    printf("  Smoker A: %d\n", smoker_A);
    printf("  Smoker B: %d\n", smoker_B);
    printf("  Smoker C: %d\n", smoker_C);

    // 供应者进程
    if (fork() == 0) {
        printf("Supplier process started (PID=%d)\n", getpid());
        while (1) {
            printf("[Supplier] Waiting for turn...\n");
            down(supplier_sem);
            
            int choice = rand() % 3;
            switch (choice) {
                case 0:
                    printf("[Supplier] Providing TOBACCO+PAPER\n");
                    up(smoker_C);
                    break;
                case 1:
                    printf("[Supplier] Providing PAPER+GLUE\n");
                    up(smoker_A);
                    break;
                case 2:
                    printf("[Supplier] Providing GLUE+TOBACCO\n");
                    up(smoker_B);
                    break;
            }
            sleep(1);
        }
        exit(0);
    }

    // 抽烟者进程
    for (int i = 0; i < 3; i++) {
        if (fork() == 0) {
            char *name;
            int *my_sem;
            if (i == 0) { name = "A (needs TOBACCO)"; my_sem = &smoker_A; }
            if (i == 1) { name = "B (needs PAPER)";   my_sem = &smoker_B; }
            if (i == 2) { name = "C (needs GLUE)";    my_sem = &smoker_C; }

            printf("Smoker %s started (PID=%d)\n", name, getpid());
            while (1) {
                printf("[Smoker %s] Waiting for materials...\n", name);
                down(*my_sem);
                printf("[Smoker %s] Smoking...\n", name);
                sleep(2);
                printf("[Smoker %s] Notifying supplier\n", name);
                up(supplier_sem);
            }
            exit(0);
        }
    }

    // 父进程
    printf("Main process waiting (PID=%d)\n", getpid());
    while (1) {
        sleep(1);
        int status;
        waitpid(-1, &status, WNOHANG);
    }
}