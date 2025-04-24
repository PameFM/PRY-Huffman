// gcc compress_pthread_debug.c -o compress_pthread_debug -lpthread
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include <sys/stat.h>  // para mkdir()



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
    int threadID;         // indice de hilo
} ThreadArg;

// funcion que ejecuta cada hlo: llama a compressFile()
void *threadFunc(void *arg) {
    ThreadArg *t = (ThreadArg*)arg;
    printf("[Hilo %d | TID %lu] INICIO: %s → %s\n",
           t->threadID, (unsigned long)pthread_self(),
           t->inFile, t->outFile);

    // Llamo al compresor serial que ya existe
    compressFile(t->inFile, t->outFile);

    printf("[Hilo %d | TID %lu] FIN   : %s\n",
           t->threadID, (unsigned long)pthread_self(),
           t->outFile);
    return NULL;
}

//#define NUM_FILES 2

// Nuevo main que acepta N archivos por línea de comandos:
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <libro1.txt> [libro2.txt ...]\n", argv[0]);
        return 1;
    }

    int n = argc - 1;
    pthread_t *threads = malloc(n * sizeof(pthread_t));
    ThreadArg *args   = malloc(n * sizeof(ThreadArg));
    struct timespec t0, t1;

    mkdir("salida", 0755);

    // Marco tiempo de inicio
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < n; ++i) {
        args[i].inFile   = argv[i+1];
        args[i].threadID = i;
    
        // 1) Sólo el nombre de fichero (sin directorio)
        const char *full = argv[i+1];
        const char *base = strrchr(full, '/');
        base = base ? base + 1 : full;
    
     


        char *outName = malloc(strlen("salida/comp_") + strlen(base) + 1);
        if (!outName) { perror("malloc"); exit(EXIT_FAILURE); }
        sprintf(outName, "salida/comp_%s", base);
        args[i].outFile = outName;
    
        // 3) Lanzo el hilo
        if (pthread_create(&threads[i], NULL, threadFunc, &args[i]) != 0) {
            perror("Error creando hilo");
            exit(EXIT_FAILURE);
        }
    }
    
    

    // Espero a que terminen todos
    for (int i = 0; i < n; ++i) {
        pthread_join(threads[i], NULL);
        free((void*)args[i].outFile);
    }

    // Marco tiempo de fin y muestro duración total
    clock_gettime(CLOCK_MONOTONIC, &t1);
    long dur_ns = (t1.tv_sec  - t0.tv_sec)  * 1000000000L
                + (t1.tv_nsec - t0.tv_nsec);
    printf("→ Tiempo total paralelo: %ld ns\n", dur_ns);
    printf("Archivos procesados: %d\n", n);

    free(threads);
    free(args);
    return 0;
}