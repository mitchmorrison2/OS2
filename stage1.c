/*
 *
 * Mitchell Morrison
 * Project 2
 * Stage 1
 * March 23, 2022
 *
 */

#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "Queue.h"

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
char temp[500] = "";
FILE* fpWrite;

char* printQueue(struct Queue* q) {
    // loop through queue from front to rear and return the combined string
    // Params: Queue struct to loop through
    sprintf(temp, "");
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

    return temp;
}

void* runner(void* val) {
    // thread runner function to pull from queue and edit score
    // set proper global variables so main process recognizes certain changes
    // Params: void * val which contains the index of the thread running

    int index = *(int*)val;

    while (!finished) {
        // run loop below continuously until entire game is finished
        while (game && getSize(dealerQueue) > 0 && winner != index) {
            // get value from queue and add/subtract score
            pthread_mutex_lock(&lockQueue);
            if (finished) break;
            int v = dequeue(dealerQueue);
            if (v > -1) {
                pthread_mutex_lock(&lockPrint);
                fprintf(fpWrite, "%s delete %d: %s\n", playerNames[index], v, printQueue(dealerQueue));
                if ((turnCount[index] + 1) % 5 != 0) {
                    sums[index] += v;
                }
                else {
                    sums[index] -= v;
                    if (getSize(dealerQueue) != QUEUE_SIZE) {
                        enqueue(dealerQueue, v);
                        fprintf(fpWrite, "%s insert %d: %s\n", playerNames[index], v, printQueue(dealerQueue));
                    }
                }
                fprintf(fpWrite, "%s score: %d\n", playerNames[index], sums[index]);
                turnCount[index]++;

                if (sums[index] >= TARGET_SCORE && game) {
                    // condition to check if the current thread is a winner
                    if (winner == -1) {
                        winner = index;
                        fprintf(fpWrite, "%s reaches the target score: %d (%d)\n", playerNames[winner], sums[index], TARGET_SCORE);
                        game = 0;
                    }
                    else {
                        fprintf(fpWrite, "MUTEX MUST NOT BE WORKING BC GAME SHOULD STOP BEFORE HERE");
                    }
                }
                pthread_mutex_unlock(&lockPrint);
            }
            pthread_mutex_unlock(&lockQueue); // unlock queues since printing and queue edits are finished
            struct timespec remaining, request = {1, v};
            if (v > -1 && nanosleep(&request, &remaining) == -1) {
                fprintf(fpWrite, "NANOSLEEP ERROR WAITING FOR NEXT CHANCE TO GRAB\n");
            }
        }
    }
    pthread_exit(0);
}

int insertToHalfFull(struct Queue* q) {
    // insert queue to half full size locking queue while happening
    // Params: queue struct passed in

    while (getSize(q) * 2 < QUEUE_SIZE) {
        int r = rand() % 50 + 1;
        pthread_mutex_lock(&lockQueue);
        enqueue(q, r);
        fprintf(fpWrite, "Dealer insert %d: %s\n", r, printQueue(dealerQueue));
        pthread_mutex_unlock(&lockQueue);

    }
    return 1;
}


int main(int argc, char** argv) {
    // main function to guide program
    FILE* fp;
    fp = fopen(argv[1], "r");
    if(!fp){
        fprintf(fpWrite, "Can't open file\n");
        exit(1);
    }
    else {
        fscanf(fp, "%d %d %d %d\n", &NUM_THREADS, &QUEUE_SIZE, &TARGET_SCORE, &NUM_PLAYERS);
    }
    char fileName[25] = "stage1_output.txt";
    fpWrite = fopen(fileName, "w+");
    if (fpWrite) {
//        printf("%s \n", "output file");
    }
    else {
        printf("Not opening");
    }

//    fprintf(fpWrite, "%d %d %d %d\n", NUM_THREADS, QUEUE_SIZE, TARGET_SCORE, NUM_PLAYERS);

    int MAX_LINE_LEN = 40;
    playerNames = (char**)malloc(NUM_PLAYERS* sizeof(char*));
    for (int i = 0; i < NUM_PLAYERS; i++) {
        // set player names
        playerNames[i] = malloc(MAX_LINE_LEN * sizeof(char));
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
        fprintf(fpWrite, "%s ", playerNames[i]);
    }
    fprintf(fpWrite, "\n");

    dealerQueue = createQueue(QUEUE_SIZE);
    insertToHalfFull(dealerQueue);

    // init both mutexes for queue and printing
    int x = pthread_mutex_init(&lockPrint, NULL) != 0;
    if (pthread_mutex_init(&lockQueue, NULL) != 0) {
        fprintf(fpWrite, "\n mutex init has failed\n");
        return 1;
    }

    // start threads ability to play game
    game = 1;
    int offsetNumThreads = 0;
    for (int i = 0; i <= NUM_PLAYERS - NUM_THREADS; i++) {
        // for x rounds execute gane and find winners
        game = 1;
        while(!finished) {
            int r = rand() % 50 + 1;
            if (getSize(dealerQueue) < QUEUE_SIZE && game) {
                // dealer add items to queue based on condition
                pthread_mutex_lock(&lockQueue);
                enqueue(dealerQueue, r);
                fprintf(fpWrite, "Dealer insert %d: %s\n", r, printQueue(dealerQueue));
                pthread_mutex_unlock(&lockQueue);

            }
            else {
                struct timespec remaining, request = {1, r};
                if (nanosleep(&request, &remaining) == -1) {
                    fprintf(fpWrite, "NANOSLEEP ERROR WAITING FOR DEALER TO INSERT NEXT\n");
                }
            }
            if (winner != -1) {
                // check if winner of this round has happened, if so print and replace player
                pthread_mutex_lock(&lockPrint);
                game = 0;
                fprintf(fpWrite, "End of game %d detected\n", i);
                fprintf(fpWrite, "Winner %s with score %d\n", playerNames[winner], sums[winner]);
                if (i < NUM_PLAYERS - NUM_THREADS) {
                    fprintf(fpWrite, "New game: %s replacing %s in slot %d\n", playerNames[NUM_THREADS + offsetNumThreads], playerNames[winner], winner);
                    playerNames[winner] = playerNames[NUM_THREADS + offsetNumThreads];
                }

                pthread_mutex_unlock(&lockPrint);

                // reset global sum and count values
                winner = -1;
                offsetNumThreads++;
                for (int c = 0; c < NUM_THREADS; c++) {
                    sums[c] = 0;
                    turnCount[c] = 0;
                }


                // reset queue to half full
                if (i < NUM_PLAYERS - NUM_THREADS) {
                    // empty the queue while threads wait for signal to start again
                    while (getSize(dealerQueue) > 0) {
                        int it = dequeue(dealerQueue);
                        fprintf(fpWrite, "Dealer delete %d: %s\n", it, printQueue(dealerQueue));
                    }
                    insertToHalfFull(dealerQueue);
                }
                break;
            }
        }

    }


    return 0;
}

