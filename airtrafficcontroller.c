//airtrafficcontroller.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define MAXLEN 512
// Structure for the message
struct  Plane {
    int plane_id;
    int type; // 0 for cargo, 1 for passenger
    int occupied_seats; // For passenger planes
    int cargo_items; // For cargo planes
    int departure_airport;
    int arrival_airport;
    int total_weight;
};


struct msg_rcv{
   long msg_type;
   struct Plane plane;
   int planeStatus;  // 0 -> coming from plane, 1 -> coming from departure airport, 2 -> coming from arrival airport, 3 -> termination request from cleanup
};

struct msg_snd{
    long msg_type;
    struct Plane plane;
    char action_type;
};

struct msg_to_plane{
    long msg_type;
    int status;

};


int main(){
	struct msg_rcv msgrcved;
	int airportnum;
    int termination_status = 0;
    int remaining_planes = 0;
	printf("Enter the number of airports to be handled/managed:");
	scanf("%d",&airportnum);
	
    key_t key = ftok("AirTrafficController.txt", 'B');
	if (key == -1){
        printf("error in creating key\n");
        exit(1);
    }
    int msgid = msgget(key, IPC_CREAT | 0644);

    if (msgid == -1) {
       printf("Another instance of the cleanup process is already running.\n");
       exit(EXIT_FAILURE);
   }

    while(1){
        if(termination_status == 1 && remaining_planes == 0){
            break;
        }
        if(msgrcv(msgid, &msgrcved, MAXLEN, 1, IPC_NOWAIT)==-1)
   	        continue;
        if(msgrcved.planeStatus == 0){
                if(termination_status == 1){
                    struct msg_to_plane msgtoplane;
                    msgtoplane.msg_type = 20+msgrcved.plane.plane_id;
                    msgtoplane.status = 0;
                    if(msgsnd(msgid,(void*) &msgtoplane, MAXLEN,0)==-1)
    	                printf("Error in sending message \n");
                    continue;
                }

                struct msg_snd msgsend_t;

                msgsend_t.msg_type = 10+msgrcved.plane.departure_airport;
                msgsend_t.plane = msgrcved.plane;
                msgsend_t.action_type = 'T';
                
                if(msgsnd(msgid, (void*)&msgsend_t, sizeof(struct msg_snd),0)==-1)
    	            printf("Error in sending message \n");       
                remaining_planes++;
        }
        else if(msgrcved.planeStatus == 1){


            struct msg_snd msgsend_l;
            msgsend_l.msg_type = 10+msgrcved.plane.arrival_airport;
            msgsend_l.plane = msgrcved.plane;
            msgsend_l.action_type = 'L';

            if(msgsnd(msgid, (void*)&msgsend_l, sizeof(struct msg_snd),0)==-1)
                printf("Error in sending message \n");
            FILE *fp;
            fp = fopen("AirTrafficController.txt", "a");
            if (fp == NULL) {
                printf("Error opening file.\n");
                return 1;
            }
            fprintf(fp, "Plane %d has departed from Airport %d and will land at Airport %d.\n", msgrcved.plane.plane_id, msgrcved.plane.departure_airport, msgrcved.plane.arrival_airport);
            fflush(fp);
            fclose(fp);
        }
        else if(msgrcved.planeStatus == 2){
                struct msg_to_plane msgtoplane;
                msgtoplane.msg_type = 20+msgrcved.plane.plane_id;
                msgtoplane.status = 1;
                if(msgsnd(msgid, (void*)&msgtoplane, sizeof(struct msg_to_plane), 0)==-1)
    	            printf("Error in sending message \n");
                remaining_planes--;
        }
        else{
            termination_status = 1;
        }
    }
    for(int i = 1; i<=airportnum; i++){
        struct msg_snd msgsend;
        msgsend.msg_type = 10+i;
        msgsend.action_type = 'S';
        if(msgsnd(msgid, (void*)&msgsend, sizeof(struct msg_snd),0)== -1)
            printf("Error in sending message \n");
        }
    printf("Cleanup Completed Successfully. \n");    
    if(msgctl(msgid, IPC_RMID, NULL) == -1){
            printf("error in deleting message queue\n");
            exit(1);
    }
	exit(0);
	return 0;
}
