#include <cmath>
#include <gtest/gtest.h>
#include <semaphore.h>
#include <thread>

#define smp_mb() asm volatile("lock; addl $0, -132(%%rsp)" ::: "memory", "cc")

int N = 1000000;
TEST(CPU, StorageBuffer) { 
    sem_t sem1, sem2, sem3, sem4;
    sem_init(&sem1, 0, 0);
    sem_init(&sem2, 0, 0);
    sem_init(&sem3, 0, 0);
    sem_init(&sem4, 0, 0);
    int x = 0, y = 0;
    int EAX = 0, EBX = 0;
    int detected = 0;

    std::thread t1([&]() {
        for (int i = 0; i < N; i++) {
            sem_wait(&sem1);
            x = 1;
            //smp_mb();
            EAX = y;
            sem_post(&sem3);
        }
    });
    std::thread t2([&]() {
        for (int i = 0; i < N; i++) {
            sem_wait(&sem2);
            y = 1;
            //smp_mb();
            EBX = x;
            sem_post(&sem4);
        }
    });
    std::thread t3([&]() {
        for (int i = 0; i < N; i++) {
            x = 0;
            y = 0;
            smp_mb();
            sem_post(&sem1);
            sem_post(&sem2);
            sem_wait(&sem3);
            sem_wait(&sem4);
            if (EAX == 0 && EBX == 0) {
                detected++;
                //printf("Something wrong happened\n");
            }
        }
    });
    t1.join();
    t2.join();
    t3.join();
    EXPECT_EQ(0, detected);
}
