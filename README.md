# 第4次操作系统上机

### 4.3完成示例

**消费者和生成者**
运行代码


ipc.c
```c
#include "ipc.h"
#include <unistd.h>  // 添加此头文件以解决 sleep()/getpid() 警告
#include <stdlib.h>
#include <stdio.h>
key_t buff_key;
int buff_num;
char *buff_ptr;

// 生产者放产品位置的共享指针
key_t pput_key;
int pput_num;
int *pput_ptr;

// 消费者取产品位置的共享指针
key_t cget_key;
int cget_num;
int *cget_ptr;

// 生产者有关的信号量
key_t prod_key;
key_t pmtx_key;
int prod_sem;
int pmtx_sem;

// 消费者有关的信号量
key_t cons_key;
key_t cmtx_key;
int cons_sem;
int cmtx_sem;

int sem_val;
int sem_flg;
int shm_flg;
int get_ipc_id(char *proc_file, key_t key) {
    FILE *pf;
    int i, j;
    char line[BUFSZ], column[BUFSZ];

    if ((pf = fopen(proc_file, "r")) == NULL) {
        perror("Proc file not open");
        exit(EXIT_FAILURE);
    }

    fgets(line, BUFSZ, pf);
    while (!feof(pf)) {
        i = j = 0;
        fgets(line, BUFSZ, pf);
        while (line[i] == ' ') i++;
        while (line[i] != ' ') column[j++] = line[i++];
        column[j] = '\0';
        if (atoi(column) != key) continue;

        j = 0;
        while (line[i] == ' ') i++;
        while (line[i] != ' ') column[j++] = line[i++];
        column[j] = '\0';
        i = atoi(column);
        fclose(pf);
        return i;
    }
    fclose(pf);
    return -1;
}

int down(int sem_id) {
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("down error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

int up(int sem_id) {
    struct sembuf buf;
    buf.sem_op = 1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("up error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

int set_sem(key_t sem_key, int sem_val, int sem_flg) {
    int sem_id;
    Sem_uns sem_arg;

    if ((sem_id = get_ipc_id("/proc/sysvipc/sem", sem_key)) < 0) {
        if ((sem_id = semget(sem_key, 1, sem_flg)) < 0) {
            perror("semaphore create error");
            exit(EXIT_FAILURE);
        }
        sem_arg.val = sem_val;
        if (semctl(sem_id, 0, SETVAL, sem_arg) < 0) {
            perror("semaphore set error");
            exit(EXIT_FAILURE);
        }
    }
    return sem_id;
}

char *set_shm(key_t shm_key, int shm_num, int shm_flg) {
    int i, shm_id;
    char *shm_buf;

    if ((shm_id = get_ipc_id("/proc/sysvipc/shm", shm_key)) < 0) {
        if ((shm_id = shmget(shm_key, shm_num, shm_flg)) < 0) {
            perror("shareMemory set error");
            exit(EXIT_FAILURE);
        }
        if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
            perror("get shareMemory error");
            exit(EXIT_FAILURE);
        }
        for (i = 0; i < shm_num; i++) shm_buf[i] = 0;  // 确保这行完整
    } else {
        if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
            perror("get shareMemory error");
            exit(EXIT_FAILURE);
        }
    }
    return shm_buf;  // 确保函数有返回语句
}
int set_msg(key_t msg_key, int msg_flg) {
    int msg_id;
    if ((msg_id = get_ipc_id("/proc/sysvipc/msg", msg_key)) < 0) {
        if ((msg_id = msgget(msg_key, msg_flg)) < 0) {
            perror("messageQueue set error");
            exit(EXIT_FAILURE);
        }
    }
    return msg_id;
}
```

ipc.h
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#define BUFSZ 256
// 抽烟者问题的信号量键值
#define SUPPLIER_KEY 401
#define SMOKER_A_KEY 501
#define SMOKER_B_KEY 502
#define SMOKER_C_KEY 503
// 建立或获取 IPC 的一组函数的原型说明
int get_ipc_id(char *proc_file, key_t key);
char *set_shm(key_t shm_key, int shm_num, int shm_flag);
int set_sem(key_t sem_key, int sem_val, int sem_flag);
int set_msg(key_t msg_key, int msg_flag);
int down(int sem_id);
int up(int sem_id);

/* 信号灯控制用的共同体 */
typedef union semuns {
    int val;
} Sem_uns;

/* 消息结构体 */
typedef struct msgbuf {
    long mtype;
    char mtext[1];
} Msg_buf;

// ipc.h 中的变量声明部分修改为：

// 生产消费者共享缓冲区即其有关的变量
extern key_t buff_key;
extern int buff_num;
extern char *buff_ptr;

// 生产者放产品位置的共享指针
extern key_t pput_key;
extern int pput_num;
extern int *pput_ptr;

// 消费者取产品位置的共享指针
extern key_t cget_key;
extern int cget_num;
extern int *cget_ptr;

// 生产者有关的信号量
extern key_t prod_key;
extern key_t pmtx_key;
extern int prod_sem;
extern int pmtx_sem;

// 消费者有关的信号量
extern key_t cons_key;
extern key_t cmtx_key;
extern int cons_sem;
extern int cmtx_sem;

extern int sem_val;
extern int sem_flg;
extern int shm_flg;
```
consumer.c
```c
#include "ipc.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
int main(int argc, char *argv[]) {
    int rate;
    if (argv[1] != NULL) rate = atoi(argv[1]);
    else rate = 3;

    // 共享内存使用的变量
    buff_key = 101;
    buff_num = 8;
    cget_key = 103;
    cget_num = 1;
    shm_flg = IPC_CREAT | 0644;

    buff_ptr = (char *)set_shm(buff_key, buff_num, shm_flg);
    cget_ptr = (int *)set_shm(cget_key, cget_num, shm_flg);

    // 信号量使用的变量
    prod_key = 201;
    pmtx_key = 202;
    cons_key = 301;
    cmtx_key = 302;
    sem_flg = IPC_CREAT | 0644;

    sem_val = buff_num;
    prod_sem = set_sem(prod_key, sem_val, sem_flg);

    sem_val = 0;
    cons_sem = set_sem(cons_key, sem_val, sem_flg);

    sem_val = 1;
    cmtx_sem = set_sem(cmtx_key, sem_val, sem_flg);

    while (1) {
        down(cons_sem);
        down(cmtx_sem);

        sleep(rate);
        printf("%d consumer get: %c from Buffer[%d]\n", getpid(), buff_ptr[*cget_ptr], *cget_ptr);

        *cget_ptr = (*cget_ptr + 1) % buff_num;

        up(cmtx_sem);
        up(prod_sem);
    }
    return EXIT_SUCCESS;
}
```
producer.c
```c
#include "ipc.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
int main(int argc, char *argv[]) {
    int rate;
    if (argv[1] != NULL) rate = atoi(argv[1]);
    else rate = 3;

    // 共享内存使用的变量
    buff_key = 101;
    buff_num = 8;
    pput_key = 102;
    pput_num = 1;
    shm_flg = IPC_CREAT | 0644;

    buff_ptr = (char *)set_shm(buff_key, buff_num, shm_flg);
    pput_ptr = (int *)set_shm(pput_key, pput_num, shm_flg);

    // 信号量使用的变量
    prod_key = 201;
    pmtx_key = 202;
    cons_key = 301;
    cmtx_key = 302;
    sem_flg = IPC_CREAT | 0644;

    sem_val = buff_num;
    prod_sem = set_sem(prod_key, sem_val, sem_flg);

    sem_val = 0;
    cons_sem = set_sem(cons_key, sem_val, sem_flg);

    sem_val = 1;
    pmtx_sem = set_sem(pmtx_key, sem_val, sem_flg);

    while (1) {
        down(prod_sem);
        down(pmtx_sem);

        buff_ptr[*pput_ptr] = 'A' + *pput_ptr;
        sleep(rate);
        printf("%d producer put: %c to Buffer[%d]\n", getpid(), buff_ptr[*pput_ptr], *pput_ptr);

        *pput_ptr = (*pput_ptr + 1) % buff_num;

        up(pmtx_sem);
        up(cons_sem);
    }
    return EXIT_SUCCESS;
}
```
## 运行结果

![1](QQ20250526-193948.png)

在当前终端窗体中启动执行速率为 1 秒的一个生产者进程
 ```
 $./producer 1 

 12263 producer put: A to Buffer[0] 
 12263 producer put: B to Buffer[1] 
 12263 producer put: C to Buffer[2] 
 12263 producer put: D to Buffer[3] 
 12263 producer put: E to Buffer[4] 
 12263 producer put: F to Buffer[5] 
 12263 producer put: G to Buffer[6] 
 12263 producer put: H to Buffer[7] 
 ```
可以看到 12263 号进程在向共享内存中连续写入了 8 个字符后因为缓冲区满而
阻塞。
打开另一终端窗体，进入当前工作目录，从中再启动另一执行速率为 3 的生产
者进程: 
```
 $ ./producer 3 
 ```
可以看到该生产者进程因为缓冲区已满而立即阻塞。
再打开另外两个终端窗体，进入当前工作目录，从中启动执行速率为 2 和 4 的两
个消费者进程:
## 独立实验--抽烟者
由于实验目的没有完成理解，所以误采用了fork子进程完成
```c
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
```
运行结果
![2](QQ20250526-201559.png)

