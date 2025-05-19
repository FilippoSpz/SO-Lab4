#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_CARS 100
#define SEM_PREFIX "/car_sem_"

typedef struct {
    char id[50];
    int busy;
} Car;

Car cars[MAX_CARS];
int car_count = 0;

void load_catalog(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening catalog file");
        exit(EXIT_FAILURE);
    }

    while (fscanf(file, "%s", cars[car_count].id) != EOF) {
        cars[car_count].busy = 0;
        car_count++;
    }

    fclose(file);
}

sem_t *get_semaphore(const char *car_id) {
    char sem_name[100];
    snprintf(sem_name, sizeof(sem_name), "%s%s", SEM_PREFIX, car_id);
    sem_t *sem = sem_open(sem_name, O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("Error creating semaphore");
        exit(EXIT_FAILURE);
    }
    return sem;
}

void view_cars() {
    for (int i = 0; i < car_count; i++) {
        printf("Car: %s, status: %s\n", cars[i].id, cars[i].busy ? "busy" : "free");
    }
}

void lock_car(const char *car_id) {
    for (int i = 0; i < car_count; i++) {
        if (strcmp(cars[i].id, car_id) == 0) {
            sem_t *sem = get_semaphore(car_id);
            sem_wait(sem);
            if (cars[i].busy) {
                printf("Error. Car %s already locked\n", car_id);
            } else {
                cars[i].busy = 1;
                printf("Car: %s is now locked\n", car_id);
            }
            sem_post(sem);
            sem_close(sem);
            return;
        }
    }
    printf("Cannot find car %s\n", car_id);
}

void release_car(const char *car_id) {
    for (int i = 0; i < car_count; i++) {
        if (strcmp(cars[i].id, car_id) == 0) {
            sem_t *sem = get_semaphore(car_id);
            sem_wait(sem);
            if (!cars[i].busy) {
                printf("Error. Car %s already free\n", car_id);
            } else {
                cars[i].busy = 0;
                printf("Car: %s is now free\n", car_id);
            }
            sem_post(sem);
            sem_close(sem);
            return;
        }
    }
    printf("Cannot find car %s\n", car_id);
}

void cleanup_semaphores() {
    for (int i = 0; i < car_count; i++) {
        char sem_name[100];
        snprintf(sem_name, sizeof(sem_name), "%s%s", SEM_PREFIX, cars[i].id);
        sem_unlink(sem_name);
    }
}

int main() {
    load_catalog("catalog.txt");

    char command[100];
    char car_id[50];

    while (1) {
        printf("\033[31mCommand:\033[0m ");
        scanf("%s", command);

        if (strcmp(command, "view") == 0) {
            view_cars();
        } else if (strcmp(command, "lock") == 0) {
            scanf("%s", car_id);
            lock_car(car_id);
        } else if (strcmp(command, "release") == 0) {
            scanf("%s", car_id);
            release_car(car_id);
        } else if (strcmp(command, "quit") == 0) {
            cleanup_semaphores();
            break;
        } else {
            printf("Unknown Command\n");
        }
    }

    return 0;
}