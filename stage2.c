///*
// *
// * Mitchell Morrison
// * Project 2
// * Stage 2
// * March 23, 2022
// *
// */
//
//#include <time.h>
//#include <stdio.h>
//#include <pthread.h>
//#include <stdlib.h>
//#include "Queue.h"
//
//int game = 0;
//int finished = 0;
//int offsetNumThreads = 0;
//
//pthread_mutex_t lockQueue;
//pthread_mutex_t lockPrint;
//
//struct Queue* dealerQueue;
//int* turnCount;
//int* sums;
//int* winners;
//
//int NUM_PLAYERS;
//int NUM_THREADS;
//int QUEUE_SIZE;
//int TARGET_SCORE;
//char** playerNames;
//char temp[500] = "";
//FILE* fpWrite;
//
//char* printQueue(struct Queue* q) {
//    // loop through queue from front to rear and return the combined string
//    // Params: Queue struct to loop through
//    sprintf(temp, "");
//    if (getSize(q)) {
//        if (q->rear < q->front) {
//            for (int i = q->front; i < QUEUE_SIZE; i++) {
//                sprintf(temp, "%s %d", temp, q->array[i]);
//            }
//            for (int i = 0; i <= q->rear; i++) {
//                sprintf(temp, "%s %d", temp, q->array[i]);
//            }
//        }
//        else {
//            for (int i = q->front; i <= q->rear; i++) {
//                sprintf(temp, "%s %d", temp, q->array[i]);
//            }
//        }
//    }
//    return temp;
//}
//
//void* runner(void* val) {
//    // thread runner function to pull from queue and edit score
//    // set proper global variables so main process recognizes certain changes
//    // Params: void * val which contains the index of the thread running
//
//    int index = *(int*)val;
//
//    while (!finished) {
//        while (game && getSize(dealerQueue) > 0 && winners[offsetNumThreads] != index) {
//            pthread_mutex_lock(&lockQueue); // start queue lock here to prevent multiple threads reading/writing at once
//            if (finished) break;
//            int v = dequeue(dealerQueue);
//            if (v > -1) {
//                pthread_mutex_lock(&lockPrint);
//                fprintf(fpWrite, "%s delete %d: %s\n", playerNames[index], v, printQueue(dealerQueue));
//                if ((turnCount[index] + 1) % 5 != 0) {
//                    sums[index] += v;
//                }
//                else {
//                    sums[index] -= v;
//                    if (getSize(dealerQueue) != QUEUE_SIZE) {
//                        enqueue(dealerQueue, v);
//                        fprintf(fpWrite, "%s insert %d: %s\n", playerNames[index], v, printQueue(dealerQueue));
//                    }
//                }
//                fprintf(fpWrite, "%s score: %d\n", playerNames[index], sums[index]);
//                turnCount[index]++;
//
//                if (sums[index] >= TARGET_SCORE && game) {
//                    if (winners[offsetNumThreads] == -1) {
//                        winners[offsetNumThreads] = index;
//                        fprintf(fpWrite, "%s reaches the target score: %d (%d)\n", playerNames[index], sums[index], TARGET_SCORE);
//                    }
//                    if (offsetNumThreads == NUM_PLAYERS - NUM_THREADS + 1) {
//                        finished = 1;
//                    }
//                }
//
//                pthread_mutex_unlock(&lockPrint);
//            }
//            pthread_mutex_unlock(&lockQueue); // end lock on queue since edits to queue are finished in this function
//
//            // sleep until time to run thread again
//            struct timespec remaining, request = {1, v};
//            if (v > -1 && nanosleep(&request, &remaining) == -1) {
//                fprintf(fpWrite, "NANOSLEEP ERROR WAITING FOR NEXT CHANCE TO GRAB\n");
//            }
//        }
//    }
//    pthread_exit(0);
//}
//
//int insertToHalfFull(struct Queue* q) {
//    // insert to queue until half full
//    // Paramas: queue struct passed in
//    while (getSize(q) * 2 < QUEUE_SIZE) {
//        int r = rand() % 50 + 1;
//        pthread_mutex_lock(&lockQueue); // editing queue, need to lock
//        enqueue(q, r);
//        fprintf(fpWrite, "Dealer insert %d : %s\n", r, printQueue(dealerQueue));
//        pthread_mutex_unlock(&lockQueue); // unlock queue because editing is finished
//    }
//    return 1;
//}
//
//
//int main(int argc, char** argv) {
//    // main function to guide program
//    FILE* fp;
//    fp = fopen(argv[1], "r");
//    if(!fp){
//        fprintf(fpWrite, "Can't open file\n");
//        exit(1);
//    }
//    else {
//        fscanf(fp, "%d %d %d %d\n", &NUM_THREADS, &QUEUE_SIZE, &TARGET_SCORE, &NUM_PLAYERS);
//    }
//    char fileName[25] = "stage2_output.txt";
//    fpWrite = fopen(fileName, "w+");
//    if (fpWrite) {
////        printf("%s \n", "output file");
//    }
//    else {
//        printf("Not opening");
//    }
//
//    int MAX_LINE_LEN = 40;
//    playerNames = (char**)malloc(NUM_PLAYERS* sizeof(char*));
//    for (int i = 0; i < NUM_PLAYERS; i++) {
//        // set player names using text from file
//        playerNames[i] = malloc(MAX_LINE_LEN * sizeof(char));
//        char line[MAX_LINE_LEN];
//        fscanf(fp, "%s\n", playerNames[i]);
//    }
//
//    winners = (int*)malloc((NUM_PLAYERS - NUM_THREADS + 1)*sizeof(int));
//    for (int i = 0 ; i < NUM_PLAYERS - NUM_THREADS + 1; i++) {
//        // create winners array for total amount of expected rounds, set initial value to -1
//        winners[i] = -1;
//    }
//
//    // initialize threads and attributes
//    pthread_t tid[NUM_THREADS]; //make this an array of size numThreads
//    pthread_attr_t attr;
//    pthread_attr_init(&attr);
//
//    // allocate space for global variables
//    sums = (int*)malloc(sizeof(int)*NUM_THREADS);
//    turnCount = (int*)malloc(sizeof(int)*NUM_THREADS);
//
//    // initialize global variables to 0;
//    for (int i=0; i < NUM_THREADS; i++) {
//        sums[i] = 0;
//        turnCount[i] = 0;
//    }
//
//    //create each thread and store file contents in global
//    for (int i = 0; i < NUM_THREADS; i++) {
//        int* x = (int*) malloc(sizeof(int));
//        *x = i;
//        pthread_create(&(tid[i]), &attr, &runner, (void *) x);
//        // print players for round
//        fprintf(fpWrite, "%s ", playerNames[i]);
//    }
//    fprintf(fpWrite, "\n");
//
//    dealerQueue = createQueue(QUEUE_SIZE);
//    insertToHalfFull(dealerQueue);
//    int x = pthread_mutex_init(&lockPrint, NULL) != 0;
//    if (pthread_mutex_init(&lockQueue, NULL) != 0) {
//        fprintf(fpWrite, "\n mutex init has failed\n");
//        return 1;
//    }
//    // start threads ability to play game
//    game = 1;
//    for (int i = 0; i <= NUM_PLAYERS - NUM_THREADS; i++) {
//        // for x rounds play game and print output to file
//        while(!finished) {
//            int r = rand() % 50 + 1;
//            if (getSize(dealerQueue) < QUEUE_SIZE && game) {
//                // lock queue to allow dealer to add items
//                pthread_mutex_lock(&lockQueue);
//                enqueue(dealerQueue, r);
//                fprintf(fpWrite, "Dealer insert %d : %s\n", r, printQueue(dealerQueue));
//                pthread_mutex_unlock(&lockQueue);
//
//            }
//            else {
//                struct timespec remaining, request = {1, r};
//                if (nanosleep(&request, &remaining) == -1) {
//                    fprintf(fpWrite, "NANOSLEEP ERROR WAITING FOR DEALER TO INSERT NEXT\n");
//                }
//            }
//
//            if (winners[offsetNumThreads] != -1) {
//                // check if winner of this round has happened, if so print and replace player
//                pthread_mutex_lock(&lockPrint);
//                fprintf(fpWrite, "End of game %d detected\n", i);
////                fprintf(fpWrite, "Winner %s with score %d\n", playerNames[winners[offsetNumThreads]], sums[winners[offsetNumThreads]]);
//                if (i < NUM_PLAYERS - NUM_THREADS) {
//                    fprintf(fpWrite, "New game: %s replacing %s in slot %d\n", playerNames[NUM_THREADS + offsetNumThreads], playerNames[winners[offsetNumThreads]], winners[offsetNumThreads]);
//                    playerNames[winners[offsetNumThreads]] = playerNames[NUM_THREADS + offsetNumThreads];
//                }
//                pthread_mutex_unlock(&lockPrint);
//
//                // reset global sum and count values
//                sums[winners[offsetNumThreads]] = 0;
//                for (int c = 0; c < NUM_THREADS; c++) {
//                    sums[c] = sums[c]/2;
//                }
//                offsetNumThreads++;
//
//                break;
//            }
//        }
//
//    }
//
//
//    return 0;
//}
//
