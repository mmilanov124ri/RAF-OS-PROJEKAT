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
    while(simulacija_traje) {
        sem_wait(&semafor_magistrale);

        if (magistrala.zauzeta == 0) {
            magistrala.zauzeta = 1;
            magistrala.racunar_id = id;
            magistrala.kolizija = 0;
            magistrala.pt = trenutno_vreme_u_us();

            printf("Racunar %d zapoceo transmisiju\n", id);

            racunar[id].stanje = 0;

            sem_post(&semafor_magistrale);

            while(simulacija_traje){    //usleep(10000); // 10 ms
                usleep(500);

                sem_wait(&semafor_magistrale);

                if (magistrala.racunar_id != id) {
                    printf("Racunar %d transmisija zavrsena\n", id);

                    sem_post(&semafor_magistrale);
                    return 0;
                }
                if (magistrala.kolizija == 1){
                    magistrala.zauzeta = 0;
                    magistrala.racunar_id = -1;
                    magistrala.kolizija = 0;
                    printf("Racunar %d prekinut zbog kolizije\n", id);
                    sem_post(&semafor_magistrale);
                    return 1;
                }

                long long vreme = trenutno_vreme_u_us();
                if (vreme >= 10000) {
                    magistrala.brojac++;
                    magistrala.zauzeta = 0;
                    magistrala.racunar_id = -1;
                    magistrala.kolizija = 0;

                    printf("Racunar %d uspesno zavrsio transmisiju\n", id);

                    sem_post(&semafor_magistrale);
                    return 0;
                }

                sem_post(&semafor_magistrale);
            }
            return 2;
        }
        else {
            long long vreme = trenutno_vreme_u_us() - magistrala.pt;

            printf("Racunar %d vidi zauzetu magistralu, proslo=%lld us\n", id, vreme);

            if (vreme >= 10000) {
                printf("Racunar %d logicki zavrsava tudju transmisiju\n", id);

                if (magistrala.kolizija == 0) {
                    magistrala.brojac++;
                }

                magistrala.zauzeta = 0;
                magistrala.racunar_id = -1;
                magistrala.kolizija = 0;

                sem_post(&semafor_magistrale);

                continue;
            }

            if (vreme >= 0 && vreme <= 1999) {
                magistrala.kolizija = 1;

                printf("Racunar %d izazvao koliziju\n", id);

                sem_post(&semafor_magistrale);
                return 1;
            }

            long long preostalo = 10000 - vreme;

            sem_post(&semafor_magistrale);

            if (preostalo>500) {
                usleep(500);
            }
            else if (preostalo > 0) {
                usleep(preostalo);
            }else{
                usleep(100);
            }
        }
    }
    return 2;
}

void* statistika_transmisija(void* arg) {
    while (simulacija_traje) {
        sleep(1);

        sem_wait(&semafor_magistrale);

        ukupno += magistrala.brojac;

        magistrala.brojac = 0;

        sem_post(&semafor_magistrale);
    }

    printf("Ukupan broj transmisija: %d\n", ukupno);
    printf("Iskoriscenje mreze: %.2f%%\n", (ukupno/500.00)* 100.00); // 6000.00
    //printf("Iskoriscenje mreze: %.2f%%\n", (ukupno/6000.00)* 100.00);

    return NULL;
}