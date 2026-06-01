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

/*int pokusaj_transmisiju(int id) {
    while(simulacija_traje) {
        sem_wait(&semafor_magistrale);

        if (magistrala.zauzeta == 0) {
            magistrala.zauzeta = 1;
            magistrala.racunar_id = id;
            magistrala.kolizija = 0;
            magistrala.pt = trenutno_vreme_u_us();

            printf("Racunar %d ZAPOCEO transmisiju\n", id);

            racunar[id].stanje = 0;

            sem_post(&semafor_magistrale);

            usleep(10000); // 10 ms

            sem_wait(&semafor_magistrale);

            if (magistrala.racunar_id == id && magistrala.kolizija == 0) {
                magistrala.brojac++;
                magistrala.zauzeta = 0;
                magistrala.racunar_id = -1;

                printf("Racunar %d USPESNO zavrsio transmisiju\n", id);

            }else if (magistrala.racunar_id == id && magistrala.kolizija == 1) {
                magistrala.zauzeta = 0;
                magistrala.racunar_id = -1;
                magistrala.kolizija = 0;
                printf("Racunar %d prekinut zbog kolizije\n", id);
                sem_post(&semafor_magistrale);
                return 1;
            }

            sem_post(&semafor_magistrale);
            return 0;
        }
        else {
            long long vreme = trenutno_vreme_u_us() - magistrala.pt;

            printf("Racunar %d vidi zauzetu magistralu, proslo=%lld us\n", id, vreme);

            if (vreme >= 10000) {
                printf("Racunar %d LOGICKI ZAVRSAVA TUDJU TRANSMISIJU\n", id);

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

                sem_post(&semafor_magistrale);
                return 1;
            }

            long long preostalo = 10000 - vreme;

            sem_post(&semafor_magistrale);

            if (preostalo > 0) {
                usleep(preostalo);
            } else {
                usleep(100);
            }
        }
    }
    return 2;
}*/

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


int pokusaj_transmisiju(int id) {
    while (simulacija_traje) {
        sem_wait(&semafor_magistrale);

        /*
            1) Ako je magistrala slobodna, ovaj računar započinje transmisiju.
        */
        if (magistrala.zauzeta == 0) {
            magistrala.zauzeta = 1;
            magistrala.racunar_id = id;
            magistrala.kolizija = 0;
            magistrala.pt = trenutno_vreme_u_us();

            racunar[id].stanje = 0; // 0 - transmituje

            sem_post(&semafor_magistrale);

            /*
                Ne spavamo odmah 10ms.
                Proveravamo u manjim koracima:
                - da li se desila kolizija
                - da li je prošlo 10ms
            */
            while (simulacija_traje) {
                usleep(500); // mali korak, 0.5ms

                sem_wait(&semafor_magistrale);

                /*
                    Ako racunar_id više nije ovaj id, znači da je neka druga nit
                    već logički završila ovu transmisiju.
                    To tretiramo kao uspeh za ovaj računar.
                */
                if (magistrala.racunar_id != id) {
                    sem_post(&semafor_magistrale);
                    return 0;
                }

                /*
                    Ako je u međuvremenu nastala kolizija,
                    transmisija se prekida.
                */
                if (magistrala.kolizija == 1) {
                    magistrala.zauzeta = 0;
                    magistrala.racunar_id = -1;
                    magistrala.kolizija = 0;

                    sem_post(&semafor_magistrale);
                    return 1; // kolizija
                }

                /*
                    Ako je prošlo 10ms od početka transmisije,
                    okvir je uspešno prenet.
                */
                long long vreme = trenutno_vreme_u_us() - magistrala.pt;

                if (vreme >= 10000) {
                    magistrala.brojac++;

                    magistrala.zauzeta = 0;
                    magistrala.racunar_id = -1;
                    magistrala.kolizija = 0;

                    sem_post(&semafor_magistrale);
                    return 0; // uspešna transmisija
                }

                sem_post(&semafor_magistrale);
            }

            return 2;
        }

        /*
            2) Ako je magistrala zauzeta, gledamo koliko je prošlo
            od početka tuđe transmisije.
        */
        long long vreme = trenutno_vreme_u_us() - magistrala.pt;

        /*
            Ako je prošlo 10ms ili više, a nije bilo kolizije,
            trenutna transmisija je logički završena.
            Ne čekamo originalnu nit da se probudi.
        */
        if (vreme >= 10000) {
            if (magistrala.kolizija == 0) {
                magistrala.brojac++;

                magistrala.zauzeta = 0;
                magistrala.racunar_id = -1;
                magistrala.kolizija = 0;

                sem_post(&semafor_magistrale);

                /*
                    Nema return.
                    Ova nit je samo oslobodila magistralu,
                    pa odmah pokušava ponovo da transmituje.
                */
                continue;
            } else {
                /*
                    Ako je bila kolizija, ne brojimo uspešan okvir.
                    Pustimo originalnu nit da detektuje koliziju i očisti stanje.
                */
                sem_post(&semafor_magistrale);
                usleep(500);
                continue;
            }
        }

        /*
            Ako drugi računar pokuša u prvih 1999us,
            dolazi do kolizije.
        */
        if (vreme >= 0 && vreme <= 1999) {
            magistrala.kolizija = 1;

            sem_post(&semafor_magistrale);
            return 1; // kolizija za ovaj računar
        }

        /*
            Ako je prošlo više od 1999us, ali manje od 10000us,
            nema kolizije. Čeka se kraj trenutne transmisije.
        */
        long long preostalo = 10000 - vreme;

        sem_post(&semafor_magistrale);

        if (preostalo > 500) {
            usleep(500);
        } else if (preostalo > 0) {
            usleep(preostalo);
        } else {
            usleep(100);
        }

        /*
            Nema return.
            Petlja ide od početka.
        */
    }

    return 2;
}