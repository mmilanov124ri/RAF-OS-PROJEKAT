//
// Created by HP on 5/29/2026.
//

#ifndef RAF_PROJEKAT_OS_MAGISTRALA_H
#define RAF_PROJEKAT_OS_MAGISTRALA_H

#include <semaphore.h>

struct magistrala {
    long long pt;
    int racunar_id;
    int brojac;
    int zauzeta;
    int kolizija;
};

extern sem_t semafor_magistrale;
extern struct magistrala magistrala;

int pokusaj_transmisiju(int id);

void* statistika_transmisija(void* arg);

#endif //RAF_PROJEKAT_OS_MAGISTRALA_H