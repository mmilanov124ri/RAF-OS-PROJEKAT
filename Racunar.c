#include "Racunar.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile int simulacija_traje = 1;

struct racunar racunar[BRPC];

void* racunar_nit(void* arg) {
    int id = *(int*)arg;

    while (simulacija_traje) {
        if (racunar[id].stanje == 2) {
            int n = racunar[id].k;
            if (n>10)
                n = 10;

            int r = rand() % (1<<n);

            int kasnjenje = r * 2;

            printf("Racunar %d backoff: k= %d, r= %d, kasnjenje= %d ms\n",id, racunar[id].k, r, kasnjenje);

            usleep(kasnjenje * 1000);

        }else {
            int cekanje_ms = 50 + rand() % 101; // 50-150 ms
            usleep(cekanje_ms * 1000);
        }

        if (!simulacija_traje) break;

        int rez = pokusaj_transmisiju(id);

        if (rez == 0) {
            racunar[id].k = 0;
            racunar[id].stanje = 1;
        }
        if (rez == 1) {
            racunar[id].k++;
            racunar[id].stanje = 2;
        }
        if (rez == 2) {
            racunar[id].stanje = 1;
        }
        printf("Racunar %d pokusava transmisiju (rez %d, stanje %d, k = %d)\n", id,rez,racunar[id].stanje,racunar[id].k);
    }
    return NULL;
}