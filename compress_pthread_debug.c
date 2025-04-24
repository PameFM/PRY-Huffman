// gcc compress_pthread_debug.c -o compress_pthread_debug -lpthread
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>


// Para no copiar todo el codigo de compress.c, lo renombramos
// y lo incluimos como un archivo de cabecera

//Renombramos el main de compress.c
#define main compress_serial_main
#include "compress.c"   
#undef main          



void compressFile(const char *inName, const char *outName) {
    FILE *in = fopen(inName, "rb");
    if (!in) { perror(inName); return; }

    int freq[256];
    calculateFrequencies(in, freq);
    HuffmanNode *root = buildHuffmanTree(freq);

    char *codes[256] = {0};
    char codeBuf[MAX_CODE_LENGTH];
    generateCodes(root, codes, codeBuf, 0);

    FILE *out = fopen(outName, "w");
    if (!out) { perror(outName); fclose(in); freeHuffmanTree(root); return; }
    compressToTextFile(in, out, codes);

    fclose(in);
    fclose(out);
    freeHuffmanTree(root);
    for (int i = 0; i < 256; i++) free(codes[i]);
}
//HILOS


typedef struct {
    const char *inFile;   // archivo de entrada
    const char *outFile;  // archivo de salida
    int threadID;         // índice de hilo
} ThreadArg;

// funcion que ejecuta cada hilo: llama a compressFile()
void *threadFunc(void *arg) {
    ThreadArg *t = (ThreadArg*)arg;
    printf("[Thread %d | TID %lu] START: %s -> %s\n",
           t->threadID, (unsigned long)pthread_self(),
           t->inFile, t->outFile);

    compressFile(t->inFile, t->outFile);

    printf("[Thread %d | TID %lu] END  : %s\n",
           t->threadID, (unsigned long)pthread_self(),
           t->outFile);
    return NULL;
}

#define NUM_FILES 2

int main() {
    const char *inputs[NUM_FILES] = { "prueba1.txt", "prueba2.txt" };
    pthread_t threads[NUM_FILES];
    ThreadArg args[NUM_FILES];
    struct timespec t0, t1;

    // tiempo de inicio
    clock_gettime(CLOCK_MONOTONIC, &t0);

    // lanzamos los hilos
    for (int i = 0; i < NUM_FILES; ++i) {
        args[i].inFile    = inputs[i];
        args[i].threadID  = i;
        char *outName     = malloc(strlen(inputs[i]) + 20);
        sprintf(outName, "comp_%s.txt", inputs[i]);
        args[i].outFile   = outName;

        if (pthread_create(&threads[i], NULL, threadFunc, &args[i]) != 0) {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }

    // esperamos a todos
    for (int i = 0; i < NUM_FILES; ++i) {
        pthread_join(threads[i], NULL);
        free((void*)args[i].outFile);
    }

    // tiempo de fin
    clock_gettime(CLOCK_MONOTONIC, &t1);
    long elapsed_ns = (t1.tv_sec - t0.tv_sec) * 1000000000L
                    + (t1.tv_nsec - t0.tv_nsec);
    printf("Total parallel time: %ld ns\n", elapsed_ns);

    return 0;
}
