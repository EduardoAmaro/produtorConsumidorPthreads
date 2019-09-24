#include <stdlib.h>
#include <stdio.h>
#include <time.h> 
#include <pthread.h>
#include <list>
#include <iostream>
#include <unistd.h>

using namespace std;

typedef struct {
    list<int> *l;
    int v, t;
} arg;

pthread_mutex_t m;
pthread_cond_t empty_buffer;
pthread_cond_t full_buffer;
arg* pthreadArgs;

void *consumidor(void *args);
void *produtor(void *args);

int main(int argc, char *argv[]) {
    int iterProd = atoi(argv[1]); //iterações produtor
    int numProd = atoi(argv[2]); //número de produtores
    int numCons = atoi(argv[3]); //número de consumidores
    int tamBuffer = atoi(argv[4]); //tamanho do buffer
    list<int> lista;

    pthreadArgs = (arg*) malloc(sizeof (arg));
    pthread_t prod[numProd];
    pthread_t cons[numCons];

    pthreadArgs[0].l = &lista;
    pthreadArgs[0].t = tamBuffer;
    pthreadArgs[0].v = iterProd;

    //inicializa
    pthread_cond_init(&full_buffer, NULL);
    pthread_cond_init(&empty_buffer, NULL);
    pthread_mutex_init(&m, NULL);

    srand(time(0));

    for (int i = 0; i < numProd; i++) {
        pthread_create(&prod[i], NULL, produtor, (void *) &pthreadArgs[0]);
    }

    for (int i = 0; i < numCons; i++) {
        pthread_create(&cons[i], NULL, consumidor, (void *) &pthreadArgs[0]);
    }

    for (int i = 0; i < numProd; i++) {
        pthread_join(prod[i], NULL);
    }

    //insere -1 na lista
    for (int i = 0; i < numCons; i++) {
        pthreadArgs[0].l->push_front(-1);
        pthread_cond_signal(&empty_buffer);
    }

    for (int i = 0; i < numCons; i++) {
        pthread_join(cons[i], NULL);
    }

}

void *consumidor(void *args) {
    arg* argm = (arg*) args;
    while (true) {
        pthread_mutex_lock(&m);
        while (argm->l->size() <= 0) {
            pthread_cond_wait(&empty_buffer, &m);
        }
        int it = argm->l->front();
        //printf("%d\n", it);
        argm->l->pop_front();
        
        if (it < 0) {
            pthread_mutex_unlock(&m);
            return NULL;
        }
        
        //verifica primo
        int div = 0;
        for (int j = 1; j <= it; j++) {
            if (it % j == 0) {
                div++;
            }
        }
        if (div == 2) {
            printf("[%lu:%d]\n", pthread_self(), it);
        }

        pthread_cond_signal(&full_buffer);
        pthread_mutex_unlock(&m);
    }
}

void *produtor(void *args) {
    arg *argm = (arg*) args;
    for (int i = 0; i < argm->v; i++) {
        pthread_mutex_lock(&m);
        while (argm->l->size() == argm->t) {
            pthread_cond_wait(&full_buffer, &m);
        }
        argm->l->push_front(rand() % 100);
        pthread_cond_signal(&empty_buffer);
        pthread_mutex_unlock(&m);
    }
    return NULL;
}
