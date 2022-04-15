//
// Created by Mitchell Morrison on 3/14/22.
//

#ifndef OS2_QUEUE_H
#define OS2_QUEUE_H


    struct Queue* createQueue(unsigned);
    int getSize(struct Queue*);
    int isFull(struct Queue*);
    int isEmpty(struct Queue*);
    void enqueue(struct Queue* , int );
    int dequeue(struct Queue* );
    int front(struct Queue* );
    int rear(struct Queue* );

    struct Queue {
        int front, rear, size;
        unsigned capacity;
        int* array;
    };

#endif //OS2_QUEUE_H
