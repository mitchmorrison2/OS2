//#include <iostream>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "Queue.h"
#include "stage1.h"

//using namespace std;

int game = 0;
int finished = 0;
pthread_mutex_t lockQueue;
pthread_mutex_t lockPrint;

struct Queue* dealerQueue;
int* turnCount;
int* sums;
int winner = -1;

int NUM_PLAYERS;
int NUM_THREADS;
int QUEUE_SIZE;
int TARGET_SCORE;
char** playerNames;

char* printQueue(struct Queue* q) {
//    pthread_mutex_lock(&lockPrint);

    char temp[100] = "";
    if (getSize(q)) {
        if (q->rear < q->front) {
            for (int i = q->front; i < QUEUE_SIZE; i++) {
                sprintf(temp, "%s %d", temp, q->array[i]);
            }
            for (int i = 0; i <= q->rear; i++) {
                sprintf(temp, "%s %d", temp, q->array[i]);
            }
        }
        else {
            for (int i = q->front; i <= q->rear; i++) {
                sprintf(temp, "%s %d", temp, q->array[i]);
            }
        }
    }
//    pthread_mutex_unlock(&lockPrint);

    return temp;
}

void* runner(void* val) {
    int index = *(int*)val;

    while (!finished) {

        while (game && getSize(dealerQueue) > 0 && winner != index) {
            pthread_mutex_lock(&lockQueue);
            if (finished) break;
            int v = dequeue(dealerQueue);
            if (v > -1) {
                printf("%s delete %d: %s\n", playerNames[index], v, printQueue(dealerQueue));
                if ((turnCount[index] + 1) % 5 != 0) {
                    sums[index] += v;
                }
                else {
                    sums[index] -= v;
                    if (getSize(dealerQueue) != QUEUE_SIZE) {
                        enqueue(dealerQueue, v);
                        printf("%s insert %d: %s\n", playerNames[index], v, printQueue(dealerQueue));
                    }
                }
                printf("%s score: %d\n", playerNames[index], sums[index]);
                turnCount[index]++;

                if (sums[index] >= TARGET_SCORE && game) {
                    if (winner == -1) {
                        winner = index;
                        printf("%s reaches the target score: %d %d\n", playerNames[winner], sums[index], TARGET_SCORE);
                        game = 0;
                    }
                    else {
                        printf("MUTEX MUST NOT BE WORKING BC GAME SHOULD STOP BEFORE HERE");
                    }
                }
            }
            pthread_mutex_unlock(&lockQueue);
            struct timespec remaining, request = {1, v};
            if (v > -1 && nanosleep(&request, &remaining) == -1) {
                printf("NANOSLEEP ERROR WAITING FOR NEXT CHANCE TO GRAB\n");
            }
        }
    }
    pthread_exit(0);
}

int insertToHalfFull(struct Queue* q, int QUEUE_SIZE) {
    while (getSize(q) * 2 < QUEUE_SIZE) {
        int r = rand() % 10 + 1;
        pthread_mutex_lock(&lockQueue);
        enqueue(q, r);
        printf("Dealer insert %d: %s\n", r, printQueue(dealerQueue));

        pthread_mutex_unlock(&lockQueue);
    }

    printf("%d\n", getSize(q));
    return 1;
}


int main(int argc, char** argv) {
//    cout << argc;

    FILE* fp;
    fp = fopen(argv[1], "r");
    if(!fp){
        printf("Can't open file\n");
        exit(1);
    }
    else {
        fscanf(fp, "%d %d %d %d\n", &NUM_THREADS, &QUEUE_SIZE, &TARGET_SCORE, &NUM_PLAYERS);
    }

    printf("%d %d %d %d\n", NUM_THREADS, QUEUE_SIZE, TARGET_SCORE, NUM_PLAYERS);

    int MAX_LINE_LEN = 40;
    playerNames = (char**)malloc(NUM_PLAYERS* sizeof(char*));
    for (int i = 0; i < NUM_PLAYERS; i++) {
        playerNames[i] = malloc(MAX_LINE_LEN * sizeof(char));
        char line[MAX_LINE_LEN];
        fscanf(fp, "%s\n", playerNames[i]);
    }


    // initialize threads and attributes
    pthread_t tid[NUM_THREADS]; //make this an array of size numThreads
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // allocate space for global variables
    sums = (int*)malloc(sizeof(int)*NUM_THREADS);
    turnCount = (int*)malloc(sizeof(int)*NUM_THREADS);

    // initialize global variables to 0;
    for (int i=0; i < NUM_THREADS; i++) {
        sums[i] = 0;
        turnCount[i] = 0;
    }

    //create each thread and store file contents in global
    for (int i = 0; i < NUM_THREADS; i++) {
        int* x = (int*) malloc(sizeof(int));
        *x = i;
        pthread_create(&(tid[i]), &attr, &runner, (void *) x);
        // print players for round
        printf("%s ", playerNames[i]);
    }
    printf("\n");

    dealerQueue = createQueue(QUEUE_SIZE);
    insertToHalfFull(dealerQueue, QUEUE_SIZE);
    int x = pthread_mutex_init(&lockPrint, NULL) != 0;
    if (pthread_mutex_init(&lockQueue, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }
    // start threads ability to play game
    game = 1;
    int offsetNumThreads = 0;
    for (int i = 0; i <= NUM_PLAYERS - NUM_THREADS; i++) {
        game = 1;
        while(!finished) {
            int r = rand() % 10 + 1;
            if (getSize(dealerQueue) < QUEUE_SIZE && game) {
//                pthread_mutex_lock(&lockQueue);
                enqueue(dealerQueue, r);
                printf("Dealer insert %d: %s\n", r, printQueue(dealerQueue));
//                pthread_mutex_unlock(&lockQueue);

            }
            else {
                struct timespec remaining, request = {1, r};
                if (nanosleep(&request, &remaining) == -1) {
                    printf("NANOSLEEP ERROR WAITING FOR DEALER TO INSERT NEXT\n");
                }
            }
            if (winner != -1) {
                game = 0;
                printf("End of game %d detected\n", i);
                printf("Winner %s with score %d\n", playerNames[winner], sums[winner]);
                if (i < NUM_PLAYERS - NUM_THREADS) {
                    char* x = playerNames[NUM_THREADS + offsetNumThreads];
                    printf("%s replacing %s in slot %d\n", playerNames[NUM_THREADS + offsetNumThreads], playerNames[winner], winner);
                    playerNames[winner] = playerNames[NUM_THREADS + offsetNumThreads];
                }

                // print names of next round players and reset global sum and count values
                winner = -1;

                offsetNumThreads++;
                for (int i = 0; i < NUM_THREADS; i++) {
                    sums[i] = 0;
                    turnCount[i] = 0;
                }
                break;
            }
        }

    }


    return 0;
}

