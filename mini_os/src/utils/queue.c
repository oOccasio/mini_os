#include "../header/Header.h"


void initQueue(Queue * queue){
    queue -> front = 0;
    queue -> rear = -1;
    queue -> size = 0;
}


bool isEmpty(Queue * queue){
    return queue->size == 0;
}


bool isFull(Queue * queue){
    return queue->size == MAX_QUEUE_SIZE;
}

void enqueue(Queue * queue, const char * str){
    if (isFull(queue)){
        fprintf(stderr, "Queue overflow\n");
        return;
    }
    queue -> rear = (queue -> rear + 1) % MAX_QUEUE_SIZE;
    queue->items[queue->rear] = strdup(str); // 내부에서 strdup로 복사
    queue->size++;
}


char * dequeue(Queue * queue){
    if (isEmpty(queue)){
        fprintf(stderr, "Queue underflow\n"); //Queue is Empty 도 ㄱㅊ아 보임
        return NULL;
    }
    char * str = queue->items[queue->front];
    queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
    queue->size--;
    return str;
}

char * peek(Queue * queue){
    if (isEmpty(queue)){
        fprintf(stderr, "Queue is empty\n");
        return NULL;
    }
    return queue->items[queue->front];
}

//메모리 해제
void freeQueue(Queue * queue){
    while (!isEmpty(queue)){
        free(dequeue(queue));
    }
}

//문자열을 '/' 문자를 기반으로 큐에 집어 넣은 큐를 형성
void buildQueue(Queue * queue, char * queuestr){
    if(queuestr == NULL){
        return;
    } else{
        // enqueue할 때 복사해서 저장하는게 안전함
        char *copy = strdup(queuestr);
        if (!copy) {
            perror("strdup failed");
            exit(EXIT_FAILURE);
        }
        enqueue(queue, copy);

        queuestr = strtok(NULL, "/");
        buildQueue(queue, queuestr);
    } 
    
}
