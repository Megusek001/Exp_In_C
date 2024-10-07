#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <sys/wait.h>


typedef struct {
    int numero;
    int listo;  
    pthread_mutex_t mutex;
} MemoriaCompartida;

void proceso_padre(MemoriaCompartida *mem) {
    srand(time(NULL)); 
    for (int i = 0; i < 10; i++) {
        sleep(2);
        pthread_mutex_lock(&mem->mutex);

        int num = rand() % 10 + 1;
        mem->numero = num;
        mem->listo = 1;
        printf("Padre: escribiendo %d\n", num);

        pthread_mutex_unlock(&mem->mutex);
    }

    
    sleep(3);


    FILE *file = fopen("resultados.txt", "r");
    if (file != NULL) {
        char ch;
        while ((ch = fgetc(file)) != EOF) {
            putchar(ch);
        }
        fclose(file);
    }
}

void proceso_hijo(MemoriaCompartida *mem) {
    int numeros[10];
    int suma = 0;

    for (int i = 0; i < 10; i++) {
        sleep(1);
        pthread_mutex_lock(&mem->mutex);

        while (mem->listo == 0) {
            pthread_mutex_unlock(&mem->mutex);
            sleep(1);  
            pthread_mutex_lock(&mem->mutex);
        }

        int num = mem->numero;
        numeros[i] = num;
        suma += num;
        printf("Hijo: leyendo %d\n", num);
        mem->listo = 0;

        pthread_mutex_unlock(&mem->mutex);
    }


    FILE *file = fopen("resultados.txt", "w");
    if (file != NULL) {
        for (int i = 0; i < 10; i++) {
            fprintf(file, "%d", numeros[i]);
            if (i < 9) fprintf(file, ", ");
        }
        fprintf(file, "\nPromedio: %.2f\n", suma / 10.0);
        fclose(file);
    }
}

int main() {
    MemoriaCompartida *mem = mmap(NULL, sizeof(MemoriaCompartida),
                                  PROT_READ | PROT_WRITE,
                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    mem->listo = 0;
    pthread_mutex_init(&mem->mutex, NULL);

    pid_t pid = fork();
    if (pid < 0) {
        perror("Error al crear el proceso");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Proceso padre
        proceso_padre(mem);
        wait(NULL);  
    } else {
        proceso_hijo(mem);
        exit(0);
    }

    pthread_mutex_destroy(&mem->mutex);
    munmap(mem, sizeof(MemoriaCompartida));

    return 0;
}
