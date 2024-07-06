#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define MAX_PASS_SEATS 10
#define MAX_CARGO_ITEMS 100
#define MAX_WEIGHT_LUGGAGE 25
#define MAX_WEIGHT_PASSENGER 100
#define MAX_WEIGHT_CARGO 100
#define CREW_MEMBERS 7
#define PILOTS 2
#define FLIGHT_ATTENDANTS 5
#define BOARDING_TIME 3
#define JOURNEY_DURATION 30
#define MAXLEN 200
// Structure to hold plane details
struct  Plane {
    int plane_id;
    int type; // 0 for cargo, 1 for passenger
    int occupied_seats; // For passenger planes
    int cargo_items; // For cargo planes
    int departure_airport;
    int arrival_airport;
    int total_weight;
};


struct msg_sent{
    long msg_type;
    struct Plane plane;
    int planeStatus;
};

struct msg_recieved{
    long msg_type;
    int status;
};

int main(){
    struct Plane plane;
    printf("Enter Plane ID: ");
    scanf("%d", &plane.plane_id);
    int pln_id = plane.plane_id;
    // Get type of plane
    printf("Enter Type of Plane (0 for cargo, 1 for passenger): ");
    scanf("%d", &plane.type);

    // Get departure and arrival airports
    printf("Enter Airport Number for Departure: ");
    int departure_airport;
    scanf("%d", &plane.departure_airport);
    printf("Enter Airport Number for Arrival: ");
    int arrival_airport;
    scanf("%d", &plane.arrival_airport);
 
     if (plane.type == 1) { // Passenger plane
        printf("Enter Number of Occupied Seats: ");
        scanf("%d", &plane.occupied_seats);
    } else { // Cargo plane    
        printf("Enter Number of Cargo Items: ");
        scanf("%d", &plane.cargo_items);
    }
   
   

     if (plane.type == 1){
        int fd_lw[plane.occupied_seats][2];
        int fd_pw[plane.occupied_seats][2];
        pid_t pid;
        int i;
        for (i = 0; i < plane.occupied_seats; i++) {
            pipe(fd_lw[i]);
            pipe(fd_pw[i]);
            pid = fork();
            if (pid == 0) {
                // Child process
                 // Close write end of the pipe
                close(fd_lw[i][0]);
                close(fd_pw[i][0]);       
                int luggage_weight;
                printf("Passenger %d: Enter Weight of Your Luggage: ", i + 1);
                scanf("%d", &luggage_weight);
                write(fd_lw[i][1], &luggage_weight, sizeof(luggage_weight));
                // Read body weight from user
                int body_weight;
                printf("Passenger %d: Enter Your Body Weight: ", i + 1);
                scanf("%d", &body_weight);
                write(fd_pw[i][1], &body_weight, sizeof(body_weight));
                // Close pipe
                close(fd_lw[i][1]);
                close(fd_pw[i][1]);
                exit(0);
            } else if (pid < 0) {
                // Error
                perror("fork");
                exit(EXIT_FAILURE);
            }
            else{
                wait(NULL);
            }
        }

        close(fd_lw[i][1]);
        close(fd_pw[i][1]);
        int passengers_weight;
        int luggage_weight;

        int total_passengers_weight = 0;
        int total_luggage_weight = 0;
        for(int i = 0; i<plane.occupied_seats; i++){
           read(fd_lw[i][0], &luggage_weight, 4);
           read(fd_pw[i][0], &passengers_weight, 4);
           total_luggage_weight += luggage_weight;
           total_passengers_weight += passengers_weight; 
        }
        int crew_weight = CREW_MEMBERS * 75; 
        plane.total_weight = crew_weight + total_passengers_weight + total_luggage_weight;
    }
    else { // Cargo plane
        printf("Enter Average Weight of Cargo Items: ");
        int cargo_weight;
        scanf("%d", &cargo_weight);
        plane.total_weight = cargo_weight * plane.cargo_items + (PILOTS * 75);
    }

    // Message Queue
    struct msg_sent message;
    key_t key;
    int msgid;
    char buf[MAXLEN];
    key = ftok("AirTrafficController.txt", 'B');
    if (key == -1){
        printf("error in creating unique key\n");
        exit(1);
    }

    msgid = msgget(key, 0644|IPC_CREAT);    
    if (msgid == -1){
        printf("error in creating message queue\n");
        exit(1);
    }

    
    
    message.msg_type = 1;
    message.plane = plane;
    message.planeStatus = 0;
 // Code to send message 
    if(msgsnd(msgid, (void *)&message, MAXLEN, 0) == -1){
    	   printf("error in sending message \n");
        exit(1);
    }
    struct msg_recieved message2;
    if (msgrcv(msgid, (void*)&message2, MAXLEN, pln_id+20, 0) == -1) {
        printf("error in receiving message\n");
        exit(EXIT_FAILURE);
    }
    if(message2.status == 1){
        printf("Plane %d has successfully traveled from Airport %d to Airport %d! \n", pln_id, plane.departure_airport, plane.arrival_airport);
    }
    else{
        printf("Plane could not take off \n");
    }
   

}