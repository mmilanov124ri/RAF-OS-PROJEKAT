//
// Created by HP on 5/29/2026.
//

#ifndef RAF_PROJEKAT_OS_RACUNAR_H
#define RAF_PROJEKAT_OS_RACUNAR_H

#define BRPC 10

struct racunar {
    int stanje; // 0-transmituje 1-ceka 2-cekanje retransmisije
    int k; // broj uzasstopnih kolizija
};

extern struct racunar racunar[BRPC];

void* racunar_nit(void* arg);

int pokusaj_transmisiju(int id);

extern volatile int simulacija_traje;

#endif //RAF_PROJEKAT_OS_RACUNAR_H