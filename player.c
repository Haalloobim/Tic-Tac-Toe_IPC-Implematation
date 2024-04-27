#include <stdlib.h>
#include <stdbool.h>
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

int count = 1;

typedef struct mesg_buffer {
	long mesg_type;
	int msgCol;
    int isValid;
    int isWinner;
} messageQueue;
typedef struct WillBeShared{
    int pl4yerTyp3[2]; 
    int playerTurn;
    int b04rdG4m3[3][3];
} WillBeShared;

void showpl4yerTyp3Choose(){
    printf("Available player type:\n");
    printf("1. X\n");
    printf("2. O\n");
}

void b04rdG4m3Screen( int b04rdG4m3[3][3] ){
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf(" %c ", b04rdG4m3[i][j] == -1 ? ' ' : b04rdG4m3[i][j] == 0 ? 'X' : 'O');
            if (j < 2) printf("|"); 
        }
        puts("");
        if (i < 2) printf("---|---|---\n"); 
    }
}

void GameStartedBanner( char pl4yerTyp3 ){
    count = 1; 
    printf("\n----Game Started!----\n"); 
    printf("You play as '%c'\n", pl4yerTyp3);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf(" %d ", count++);
            if (j < 2) printf("|"); 
        }
        puts("");
        if (i < 2) printf("---|---|---\n"); 
    }
    printf("Select the corresponding c3ll number to choose a cell.\n\n");
}

int main(){
    key_t key = 1333337, keyMsg = ftok("progfile", 1337);
    
    int sharedMemId = shmget(key, sizeof(WillBeShared), IPC_CREAT | 0776), flag = 0, pl4yerTyp3 = -1;
    
    WillBeShared *sharedStructData = (WillBeShared *)shmat(sharedMemId, NULL, 0);

    messageQueue *msgQ; 
    
    while(1){
        showpl4yerTyp3Choose();
        printf("Enter your choice (1 or 2):");
        scanf("%d", &pl4yerTyp3);
        printf("%d %d %d\n", sharedStructData->pl4yerTyp3[0], sharedStructData->pl4yerTyp3[1], sharedStructData->playerTurn);
        if(sharedStructData->pl4yerTyp3[0] == 1 && sharedStructData->pl4yerTyp3[1] == 1){
            printf("Game is full!\n");
            shmdt(sharedStructData);
            shmctl(sharedMemId, IPC_RMID, NULL);
            return 0;
        }
        else if ((pl4yerTyp3 != 1 && pl4yerTyp3 != 2)){
            printf("Invalid player type!\n");
            continue;
        }
        else if (pl4yerTyp3 == 1 && sharedStructData->pl4yerTyp3[0] == 1){
            printf("Player 'X' is already taken!\n");
            continue;
        }
        else if (pl4yerTyp3 == 2 && sharedStructData->pl4yerTyp3[1] == 1){
            printf("Player 'O' is already taken!\n");
            continue;
        }
        if(pl4yerTyp3 == 1){
            sharedStructData->pl4yerTyp3[0] = 1;
            break;
        }
        else {
            sharedStructData->pl4yerTyp3[1] = 1;
            break;
        }
    }

    while(sharedStructData->pl4yerTyp3[0] != 1 || sharedStructData->pl4yerTyp3[1] != 1){
        printf("Waiting for other player to join...\n"); 
        sleep(2);
    }

    int msgid = msgget(keyMsg, 0776 | IPC_CREAT);
    
    msgQ = (messageQueue *)malloc(sizeof(messageQueue));
    msgQ->isWinner = -1; 
    
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            sharedStructData->b04rdG4m3[i][j] = -1;
        }
    }
    
    sharedStructData->playerTurn = 0; 
    while(1){
        // sharedStructData->playerTurn++;
        int choosenCell = 0; 
        printf("Round = %d\n", sharedStructData->playerTurn);
        if (flag){
            if ((sharedStructData->playerTurn) % 2 == 0 ){
                printf("\nPlayer 'X' turn\n");
                if (pl4yerTyp3 == 1){
                    printf("[X] Enter the cell number: ");
                    scanf("%d", &choosenCell);
                    msgQ->msgCol = choosenCell;
                    msgQ->mesg_type = 1;
                    msgsnd(msgid, msgQ, sizeof(msgQ), 0);
                    msgrcv(msgid, msgQ, sizeof(messageQueue), 1, 0);
                    if (msgQ->isValid == -1){
                        printf("Invalid move! Try again.\n");
                        continue;
                    }
                    else {
                        sharedStructData->playerTurn++;
                        sharedStructData->b04rdG4m3[(msgQ->msgCol - 1) / 3][(msgQ->msgCol - 1) % 3] = 0;
                        b04rdG4m3Screen(sharedStructData->b04rdG4m3);
                        if (msgQ->isWinner != -1){
                            break; 
                        }
                    }
                }      
                else {
                    printf("[O] [Waiting for player 'X' to make a move...]\n");
                    sleep(1);
                    msgrcv(msgid, msgQ, sizeof(messageQueue), 2, 0);
                    if (msgQ->isValid == -1){
                        printf("Invalid move! Try again.\n");
                        continue;
                    }
                    else {
                        sharedStructData->b04rdG4m3[(msgQ->msgCol - 1) / 3][(msgQ->msgCol - 1) % 3] = 0;
                        b04rdG4m3Screen(sharedStructData->b04rdG4m3);
                        if (msgQ->isWinner != -1){
                            break; 
                        }
                    }
                }
            }
            else if ((sharedStructData->playerTurn) % 2 == 1){
                printf("\nPlayer 'O' turn\n");
                if (pl4yerTyp3 == 2){
                    printf("[O] Enter the cell number: ");
                    scanf("%d", &choosenCell);
                    msgQ->msgCol = choosenCell;
                    msgQ->mesg_type = 1;
                    msgsnd(msgid, msgQ, sizeof(msgQ), 0);
                    msgrcv(msgid, msgQ, sizeof(messageQueue), 1, 0);
                    if (msgQ->isValid == -1){
                        printf("Invalid move! Try again.\n");
                        continue;
                    }
                    else {
                        sharedStructData->playerTurn++;
                        sharedStructData->b04rdG4m3[(msgQ->msgCol - 1) / 3][(msgQ->msgCol - 1) % 3] = 1;
                        b04rdG4m3Screen(sharedStructData->b04rdG4m3);
                        if (msgQ->isWinner != -1){
                            break;
                        }
                    }
                }
                else {
                    printf("[X] [Waiting for player 'O' to make a move...]\n");
                    sleep(1);
                    msgrcv(msgid, msgQ, sizeof(messageQueue), 2, 0);
                    if (msgQ->isValid == -1){
                        printf("\nInvalid move! Try again.\n");
                        continue;
                    }
                    else {
                        sharedStructData->b04rdG4m3[(msgQ->msgCol - 1) / 3][(msgQ->msgCol - 1) % 3] = 1;
                        b04rdG4m3Screen(sharedStructData->b04rdG4m3);
                        if (msgQ->isWinner != -1){
                            break;
                        }
                    }
                }
            }
            continue;   
        } 
        GameStartedBanner(pl4yerTyp3 == 1 ? 'X' : 'O');
        flag = 1;
    }
    if (msgQ->isWinner == 0) printf("\nPlayer 'X' is the winner!\n");
    else if (msgQ->isWinner == 1) printf("\nPlayer 'O' is the winner!\n");
    else printf("\nGame is a draw!\n");
    
    shmdt(sharedStructData);
    shmctl(sharedMemId, IPC_RMID, NULL);
}