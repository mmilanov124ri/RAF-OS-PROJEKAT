//
// Created by HP on 5/29/2026.
//

#ifndef RAF_PROJEKAT_OS_RACUNAR_H
#define RAF_PROJEKAT_OS_RACUNAR_H

struct racunar {
    int stanje; // 0-transmituje 1-ceka 2-cekanje retransmisije
    int k; // broj uzasstopnih kolizija
};

void* racunar_nit(void* arg);

extern volatile int simulacija_traje;

#endif //RAF_PROJEKAT_OS_RACUNAR_H