#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>


#define MSG_SIZE 128
#define KEY 1234

struct  Plane {
    int plane_id;
    int type; // 0 for cargo, 1 for passenger
    int occupied_seats; // For passenger planes
    int cargo_items; // For cargo planes
    int departure_airport;
    int arrival_airport;
    int total_weight;
};
// Structure for the message
struct msg_buffer {
   long msg_type;
   struct Plane plane;
   int planeStatus;
};


int main() {


   // Create a message queue for communication with Air Traffic Controller
   key_t atc_key = ftok("AirTrafficController.txt", 'B');
   int atc_msgid = msgget(atc_key, 0644 | IPC_CREAT);


   if (atc_msgid == -1) {
       perror("msgget");
       exit(EXIT_FAILURE);
   }


   struct msg_buffer message;
   message.msg_type = 1;

   while (1) {
       // Display message and prompt user for input
       char input;
       printf("Do you want the Air Traffic Control System to terminate? (Y for Yes and N for No): ");
       scanf("%c", &input);
       getchar();
        if(input == 'Y'){
            message.planeStatus = 3;
             if (msgsnd(atc_msgid, (void*)&message, sizeof(message), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
             }  
            break;
        }
   }


   return 0;
}
