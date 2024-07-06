#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_RUNWAYS 10
#define MIN_LOAD_CAPACITY 1000
#define MAX_LOAD_CAPACITY 12000
#define BACKUP_LOAD_CAPACITY 15000
#define MAXLEN 100
#define INT_MAX 2147483647

struct  Plane {
    int plane_id;
    int type; // 0 for cargo, 1 for passenger
    int occupied_seats; // For passenger planes
    int cargo_items; // For cargo planes
    int departure_airport;
    int arrival_airport;
    int total_weight;
};

struct msg_buffer{
    long msg_type;
    struct Plane plane;
    char action_type;
};

struct msg_snd{
    long msg_type;
    struct Plane plane;
    int planeStatus;  
};

int load_capacity[11];
sem_t runway_sem[11];
int airport_num;
int num_runways;
int msgid;
int findBestFitRunway(int plane_weight) {
            int best_fit_runway = -1;
            int min_difference = INT_MAX;

            for (int i = 0; i < num_runways+1; i++) {
                int difference = load_capacity[i] - plane_weight;
                if (difference >= 0 && difference < min_difference) {
                    min_difference = difference;
                    best_fit_runway = i;
                }
            }

            return best_fit_runway;
}

void* ThreadFunc(void* arg) {
    struct msg_buffer* message = (struct msg_buffer*)arg;
    int best_fit_runway = findBestFitRunway(message->plane.total_weight);

    sem_wait(&runway_sem[best_fit_runway]);
    struct msg_snd message_to_atc;
    message_to_atc.msg_type = 1;
    message_to_atc.plane = message->plane;
    if(message->action_type == 'L') {
        sleep(5);
        sleep(30);
        printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n", message->plane.plane_id, best_fit_runway+1, airport_num);
        message_to_atc.planeStatus = 2;
    } else {
        sleep(5);
        printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d\n", message->plane.plane_id, best_fit_runway+1, airport_num);
        message_to_atc.planeStatus = 1;
    }
    if(msgsnd(msgid, (void*)&message_to_atc, sizeof(struct msg_snd), 0) == -1){
        printf("error in sending message\n");
        exit(1);
    }
    sem_post(&runway_sem[best_fit_runway]);
    pthread_exit(0);
}


int main() {
    key_t key;
    struct msg_buffer message; 
    printf("Enter Airport Number: \n");
    scanf("%d", &airport_num);

    printf("Enter Number of Runways: \n");
    scanf("%d", &num_runways);
    getchar();

    for(int i = 0; i < num_runways+1; i++) {
        sem_init(&runway_sem[i], 0, 1);
    }

    char load_capacity_input[100];
    char d[] = " ";
    printf("Enter loadCapacity of Runways (give as a space separated list in a single line): \n");
    fgets(load_capacity_input, 100, stdin);
    //printf("%s\n", load_capacity_input);
    char* token = strtok(load_capacity_input, d);
    int i = 0;
    while(token != NULL) {
        load_capacity[i++] = atoi(token);
        token = strtok(NULL, " \n");
    }
    load_capacity[i] = BACKUP_LOAD_CAPACITY;
    key = ftok("AirTrafficController.txt", 'B');
    if (key == -1){
        printf("error in creating unique key\n");
        exit(1);
    }

    msgid = msgget(key, IPC_CREAT | 0644);    
    if (msgid == -1){
        printf("error in creating message queue\n");
        exit(1);
    }
    int j  = 0;
    while(1){
        if (msgrcv(msgid, (void *)&message, sizeof(struct msg_buffer), airport_num+10, IPC_NOWAIT) == -1){
            continue;
        }
        if(message.action_type == 'S'){
            break;
        }
        pthread_t thread;
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        if (pthread_create(&thread, NULL, ThreadFunc, (void*)&message) != 0) {
                    printf("error in creating thread\n");
                    exit(1);
        }
    }    
    return 0;
}
