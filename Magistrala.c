#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "Magistrala.h"

#include "Racunar.h"

struct timeval t;

sem_t semafor_magistrale;
struct magistrala magistrala;

int ukupno = 0;

long long trenutno_vreme_u_us() {
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000000LL + t.tv_usec;
}

int pokusaj_transmisiju(int id) {
    sem_wait(&semafor_magistrale);

    if (magistrala.zauzeta == 0) {
        magistrala.zauzeta = 1;
        magistrala.racunar_id = id;
        magistrala.pt = trenutno_vreme_u_us();

        printf("Racunar %d ZAPOCEO transmisiju\n", id);

        sem_post(&semafor_magistrale);

        usleep(10000); // 10 ms

        sem_wait(&semafor_magistrale);

        if (magistrala.racunar_id == id) {
            magistrala.brojac++;
            magistrala.zauzeta = 0;
            magistrala.racunar_id = -1;

            printf("Racunar %d USPESNO zavrsio transmisiju\n", id);

        }

        sem_post(&semafor_magistrale);
        return 0;
    }
    else {
        long long vreme = trenutno_vreme_u_us() - magistrala.pt;

        printf("Racunar %d vidi zauzetu magistralu, proslo=%lld us\n",
           id, vreme);

        if (vreme <= 1999) {
            sem_post(&semafor_magistrale);
            return 1;
        }
        else{
            sem_post(&semafor_magistrale);
            return 2;
        }
    }
}

void* statistika_transmisija(void* arg) {
    while (simulacija_traje){
        sleep(1);

        sem_wait(&semafor_magistrale);

        ukupno += magistrala.brojac;

        magistrala.brojac = 0;

        sem_post(&semafor_magistrale);
    }
    printf("Ukupan broj transmisija: %d\n", ukupno);
    //printf("Iskoriscenje mreze: %.2f%%\n", (ukupno/500.00)* 100.00); // 6000.00
    printf("Iskoriscenje mreze: %.2f%%\n", (ukupno/6000.00)* 100.00);

    return NULL;
}