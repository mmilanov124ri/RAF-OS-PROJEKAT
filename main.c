#include <semaphore.h>
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "D:\SVASTA\Programiranje\CLion\RAF PROJEKAT OS\Magistrala.h"
#include "D:\SVASTA\Programiranje\CLion\RAF PROJEKAT OS\Racunar.h"

#define BRPC 10

struct timeval tv;

struct racunar racunar[BRPC];
struct magistrala magistrala;

sem_t semaformagistrale;

int main(void) {
    sem_init(&semaformagistrale, 0, 1);

    // sem_wait(&semaformagistrale);
    // sem_post(&semaformagistrale);

    gettimeofday(&tv, NULL);
    int vreme = tv.tv_usec;

    magistrala.pt = 0;
    magistrala.racunar_id = -1;
    magistrala.brojac = 0;
    magistrala.zauzeta = 0;

    for (int i = 0;i<BRPC;i++) {
        racunar[i].stanje = 1;
        racunar[i].k = 0;
    }

    pthread_t niti[BRPC];
    int ids[BRPC];

    for (int i = 0; i < BRPC; i++) {
        ids[i] = i;
        pthread_create(&niti[i], NULL, racunar_nit, &ids[i]);
    }

    sleep(5);
    simulacija_traje = 0;

    for (int i = 0; i < BRPC; i++) {
        pthread_join(niti[i], NULL);
    }


    sem_destroy(&semaformagistrale);


    return 0;
}