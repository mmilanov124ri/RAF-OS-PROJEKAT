#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile int simulacija_traje = 1;

void* racunar_nit(void* arg) {
    int id = *(int*)arg;

    while (simulacija_traje) {
        int cekanje_ms = 50 + rand() % 101; // 50-150 ms
        usleep(cekanje_ms * 1000);

        printf("Racunar %d pokusava transmisiju\n", id);
    }

    return NULL;
}