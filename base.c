///*
// *
// * Mitchell Morrison
// * Project 2
// * Base Case
// * March 23, 2022
// *
// */
//
//
//#include <time.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <stdlib.h>
//#include <pthread.h>
//#include "Queue.h"
//
//
//int game = 0;
//int finished = 0;
//pthread_mutex_t lockQueue;
//pthread_mutex_t lockPrint;
//
//struct Queue* dealerQueue;
//int* turnCount;
//int* sums;
//int winners[2] = {-1, -1};
//
//int NUM_THREADS;
//int QUEUE_SIZE;
//int TARGET_SCORE;
//FILE* fpWrite;
//char temp[500] = "";
//
//char* printQueue(struct Queue* q) {
//
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
//
//
//    return temp;
//}
//
//void* runner(void* val) {
//    int index = *(int*)val;
//
//    while (!finished) {
//
//        while (game && getSize(dealerQueue) > 0 && winners[0] != index && winners[1] != index) {
//            pthread_mutex_lock(&lockQueue);
//            if (finished) break;
//            int v = dequeue(dealerQueue);
//            if (v > -1) {
//                pthread_mutex_lock(&lockPrint);
//                fprintf(fpWrite ,"Player %d delete %d : %s\n", index, v, printQueue(dealerQueue));
//                if ((turnCount[index] + 1) % 5 != 0) {
//                    sums[index] += v;
//                }
//                else {
//                    sums[index] -= v;
//                    if (getSize(dealerQueue) != QUEUE_SIZE) {
//                        enqueue(dealerQueue, v);
//                        fprintf(fpWrite ,"Player %d insert %d : %s\n", index, v, printQueue(dealerQueue));
//                    }
//                }
//                fprintf(fpWrite ,"Player %d score : %d\n", index, sums[index]);
//                turnCount[index]++;
//
//                if (sums[index] >= TARGET_SCORE) {
//                    if (winners[0] == -1) {
//                        winners[0] = index;
//                        fprintf(fpWrite ,"Player %d reaches the target score : %d (%d)\n", winners[0], sums[index], TARGET_SCORE);
//                    }
//                    else if (winners[1] == -1) {
//                        winners[1] = index;
//                        fprintf(fpWrite ,"Player %d reaches the target score : %d (%d)\n", winners[1], sums[index], TARGET_SCORE);
//                        finished = 1;
//                    }
//                    else {
//                        fprintf(fpWrite ,"MUTEX MUST NOT BE WORKING BC GAME SHOULD STOP BEFORE HERE");
//                    }
//                }
//                pthread_mutex_unlock(&lockPrint);
//            }
//            pthread_mutex_unlock(&lockQueue);
//            struct timespec remaining, request = {1, v};
//            if (v > -1 && nanosleep(&request, &remaining) == -1) {
//                fprintf(fpWrite ,"NANOSLEEP ERROR WAITING FOR NEXT CHANCE TO GRAB\n");
//            }
//        }
//
//    }
//
//    pthread_exit(0);
//
//}
//
//int insertToHalfFull(struct Queue* q) {
//    while (getSize(q) * 2 < QUEUE_SIZE) {
//        int r = rand() % 10 + 1;
//        enqueue(q, r);
//        fprintf(fpWrite ,"Dealer insert %d : %s\n", r, printQueue(dealerQueue));
//    }
//
//    return 1;
//}
//
//
//int main(int argc, char** argv) {
//
//    NUM_THREADS = atoi(argv[1]);
//    QUEUE_SIZE = atoi(argv[2]);
//    TARGET_SCORE = atoi(argv[3]);
//
//    char outputFile[25] = "base_output.txt";
//    fpWrite = fopen(outputFile, "w+");
//    if (fpWrite) {
////        printf("%s \n", "output file");
//    }
//    else {
//        printf("Not opening");
//    }
//    // initialize threads and attributes
//    pthread_t tid[NUM_THREADS]; //make this an array of size numThreads
//    pthread_attr_t attr;
//    pthread_attr_init(&attr);
//
//    // allocate space fore global variables
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
//    }
//
//    dealerQueue = createQueue(QUEUE_SIZE);
//    insertToHalfFull(dealerQueue);
//    int x = pthread_mutex_init(&lockPrint, NULL) != 0;
//    if (pthread_mutex_init(&lockQueue, NULL) != 0) {
//        fprintf(fpWrite ,"\n mutex init has failed\n");
//        return 1;
//    }
//    // start threads ability to play game
//    game = 1;
//
//    while(!finished) {
//        int r = rand() % 10 + 1;
//        if (getSize(dealerQueue) < QUEUE_SIZE) {
//            pthread_mutex_lock(&lockQueue);
//            enqueue(dealerQueue, r);
//            pthread_mutex_unlock(&lockQueue);
//
//            pthread_mutex_lock(&lockPrint);
//            fprintf(fpWrite ,"Dealer insert %d : %s\n", r, printQueue(dealerQueue));
//            pthread_mutex_unlock(&lockPrint);
//        }
//        else {
//            struct timespec remaining, request = {1, r};
//            if (nanosleep(&request, &remaining) == -1) {
//                printf("NANOSLEEP ERROR WAITING FOR DEALER TO INSERT NEXT\n");
//            }
//        }
//        if (winners[0] != -1 && winners[1] != -1) {
//            // if 2 winners exist, end game
//            finished = 1;
//
//            pthread_mutex_lock(&lockPrint);
//            fprintf(fpWrite ,"End of game detected\n");
//            fprintf(fpWrite ,"Winner Player %d with score %d. Runner up Player %d with score %d", winners[0], sums[winners[0]], winners[1], sums[winners[1]]);
//            pthread_mutex_unlock(&lockPrint);
//
//            break;
//        }
//    }
//
//
//    return 0;
//}
//
