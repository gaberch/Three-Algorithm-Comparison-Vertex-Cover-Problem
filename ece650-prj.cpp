#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include "timer.h"
#include "thread_functions.h"

sem_t input_ready_sem[THREAD_NUM];
sem_t min_vc_done;
sem_t approx_vc_done[2];
/*
 * 0: CNF-SAT_VC thread
 * 1: APPROX-VC-1 thread
 * 2: APPROX-VC-2 thread
 * 3: IO    thread
 */
pthread_t thread_id[THREAD_NUM];
clockid_t cid[THREAD_NUM];

using namespace std;

int main() {
    int status;

    sem_init(&input_ready_sem[0], 0, 0);
    sem_init(&input_ready_sem[1], 0, 0);
    sem_init(&input_ready_sem[2], 0, 0);
    sem_init(&min_vc_done, 0, 0);
    sem_init(&approx_vc_done[0], 0, 0);
    sem_init(&approx_vc_done[1], 0, 0);

    pthread_create( &thread_id[3], NULL, run_io_thread, NULL);  // start io thread
    pthread_create( &thread_id[1], NULL, run_approx_vc_1_thread, NULL); // start approx-vc-1
    pthread_create( &thread_id[2], NULL, run_approx_vc_2_thread, NULL); // start approx-vc-2
    status = pthread_getcpuclockid(thread_id[1], &cid[1]);
    if (status != 0) {
        cerr << "ERROR: get run_approx_vc_1_thread cpu clock id failed" << endl;
    }
    status = pthread_getcpuclockid(thread_id[2], &cid[2]);
    if (status != 0) {
        cerr << "ERROR: get run_approx_vc_2_thread cpu clock id failed" << endl;
    }

    int sig = SIGINT;
    sigset_t sigSet;
    sigemptyset (&sigSet);
    sigaddset (&sigSet, sig);

    pthread_sigmask (SIG_BLOCK, &sigSet, NULL);

#ifdef PROJ_DEBUG
    clog << "Got CtrlD, wait threads done" << endl;
#endif
    pthread_join(thread_id[3], NULL);   // wait io thread done, that means received CtrlD quit from program
    pthread_cancel(thread_id[1]);
    pthread_cancel(thread_id[2]);

    pthread_join(thread_id[1], NULL);
    pthread_join(thread_id[2], NULL);

    return 0;
}

