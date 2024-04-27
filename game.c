#include <stdlib.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <syslog.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>

typedef struct mesg_buffer {
	long mesg_type;
	int msgCol;
    int isValid;
    int isWinner;
} messageQueue;

void printMessage(messageQueue *recievedSharedData, char PlayerType){
    printf("Message Recieved\n");
    printf("Sender: player %c\n", PlayerType);
    printf("Message: %d \n", recievedSharedData->msgCol);
}

int isInputForBoardValid(int b04rdG4m3[3][3], int userInput){
    if (userInput < 1 || userInput > 9) return -1;
    int row = (userInput - 1) / 3;
    int col = (userInput - 1) % 3;
    if (b04rdG4m3[row][col] != -1) return -1;
    return 1;
}

int isBoardHaveWinner(int b04rdG4m3[3][3], int playerType){
    for (int i = 0; i < 3; i++){
        if (b04rdG4m3[i][0] == playerType && b04rdG4m3[i][1] == playerType && b04rdG4m3[i][2] == playerType) return 1;
        if (b04rdG4m3[0][i] == playerType && b04rdG4m3[1][i] == playerType && b04rdG4m3[2][i] == playerType) return 1;
    }
    if (b04rdG4m3[0][0] == playerType && b04rdG4m3[1][1] == playerType && b04rdG4m3[2][2] == playerType) return 1;
    if (b04rdG4m3[0][2] == playerType && b04rdG4m3[1][1] == playerType && b04rdG4m3[2][0] == playerType) return 1;
    return 0;
}

int main(){
    key_t keyMsg = ftok("game.c", 1337);
    
    int b04rdG4m3[3][3] = {{-1,-1,-1}, {-1,-1,-1}, {-1,-1,-1}}; 

    int msgid = msgget(keyMsg, 0776 | IPC_CREAT), roundCount = 1;
    
    messageQueue *recievedSharedData = (messageQueue *)malloc(sizeof(messageQueue));
    recievedSharedData->isWinner = -1; 
    printf("--Game Server Has Been Started--\n");
    
    while(1){
        printf("\n\nWaiting for data..\n");
        if (roundCount % 2 == 1){
            msgrcv(msgid, recievedSharedData, sizeof(messageQueue), 1, 0);
            printf("Round = %d\n", roundCount);
            printMessage(recievedSharedData, 'X');
            int isValid = isInputForBoardValid(b04rdG4m3, recievedSharedData->msgCol);
            if (isValid == -1){
                recievedSharedData->mesg_type = 1; 
                recievedSharedData->isValid = isValid;
                msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
                recievedSharedData->mesg_type = 2;
                recievedSharedData->isValid = isValid; 
                msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
            }
            else{
                b04rdG4m3[(recievedSharedData->msgCol - 1) / 3][(recievedSharedData->msgCol - 1) % 3] = 0;
                int WinnerFlag = isBoardHaveWinner(b04rdG4m3, 0);
                if (WinnerFlag){
                    printf("\nPlayer X is the winner!\n");
                    recievedSharedData->isWinner = 0;
                    recievedSharedData->mesg_type = 1; 
                    recievedSharedData->isValid = isValid; 
                    msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
                    recievedSharedData->mesg_type = 2;
                    recievedSharedData->isValid = isValid; 
                    msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
                    msgctl(msgid, IPC_RMID, NULL);
                    free(recievedSharedData);
                    return 0; 
                }
                else {
                    if (roundCount == 9){
                        printf("\nIt's a draw!\n");
                        recievedSharedData->isWinner = 2;
                        recievedSharedData->mesg_type = 1; 
                        recievedSharedData->isValid = isValid; 
                        msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
                        recievedSharedData->mesg_type = 2;
                        recievedSharedData->isValid = isValid; 
                        msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
                        msgctl(msgid, IPC_RMID, NULL);
                        free(recievedSharedData);
                        return 0; 
                    }
                    recievedSharedData->mesg_type = 1; 
                    recievedSharedData->isValid = isValid; 
                    msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
                    recievedSharedData->mesg_type = 2;
                    recievedSharedData->isValid = isValid; 
                    msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
                    roundCount++;
                }
            }
        }
        else if (roundCount % 2 == 0){
            msgrcv(msgid, recievedSharedData, sizeof(messageQueue), 1, 0);
            printf("Round = %d\n", roundCount);
            printMessage(recievedSharedData, 'O');
            int isValid = isInputForBoardValid(b04rdG4m3, recievedSharedData->msgCol);
            if (isValid == -1){
                recievedSharedData->mesg_type = 1; 
                recievedSharedData->isValid = isValid;
                msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
                recievedSharedData->mesg_type = 2;
                recievedSharedData->isValid = isValid; 
                msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
            }
            else{
                b04rdG4m3[(recievedSharedData->msgCol - 1) / 3][(recievedSharedData->msgCol - 1) % 3] = 1;
                int WinnerFlag = isBoardHaveWinner(b04rdG4m3, 1);
                if (WinnerFlag){
                    printf("\nPlayer O is the winner!\n");
                    recievedSharedData->isWinner = 1;
                    recievedSharedData->mesg_type = 1; 
                    recievedSharedData->isValid = isValid; 
                    msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
                    recievedSharedData->mesg_type = 2;
                    recievedSharedData->isValid = isValid; 
                    msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
                    msgctl(msgid, IPC_RMID, NULL);
                    free(recievedSharedData);
                    return 0; 
                }
                else {
                    recievedSharedData->mesg_type = 1; 
                    recievedSharedData->isValid = isValid;
                    msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
                    recievedSharedData->mesg_type = 2;
                    recievedSharedData->isValid = isValid; 
                    msgsnd(msgid, recievedSharedData, sizeof(messageQueue), 0);
                    roundCount++;
                }
            }
        } 
    }

    free(recievedSharedData);
    return 0;
}
