#include <stdio.h>      // For standard input/output functions
#include <stdlib.h>     // For dynamic memory allocation and other utilities
#include <pthread.h>    // For threading functionality
#include <semaphore.h>  // For semaphores
#include <unistd.h>     // For sleep function
#include <time.h>       // For random number seeding

// Struct to hold buffer and synchronization variables
typedef struct {
    int *buffer;                   // Pointer to the buffer array
    int bufferSize;                // Size of the buffer
    int inIndex;                   // Index where the producer will add the next item
    int outIndex;                  // Index where the consumer will remove the next item
    int bufferCount;               // Tracks the number of items currently in the buffer
    sem_t fullSlot;                // Semaphore to track the number of filled slots
    sem_t emptySlot;               // Semaphore to track the number of empty slots
    pthread_mutex_t mutexToAccessBuffer; // Mutex to protect shared data access
} Buffer;

void printBuffer(int *myBuffer, int bufferSize) {
    int loopVar;
	printf("Buffer: [");
    for (loopVar = 0; loopVar < bufferSize; loopVar++) {
        if (loopVar == bufferSize - 1)
            printf("%d", myBuffer[loopVar]);
        else
            printf("%d, ", myBuffer[loopVar]);
    }
    printf("]\n");
}

// Producer thread function
void *producer(void *arg) {
    Buffer *myBuffer = (Buffer *)arg;        // Cast the argument to a Buffer pointer
    int id = pthread_self() % 10000;         // Generate a pseudo-unique thread ID
	srand(id);
    while (1) {                              // Infinite loop for continuous production
        sem_wait(&myBuffer->emptySlot);      // Wait until there is at least one empty slot
        pthread_mutex_lock(&myBuffer->mutexToAccessBuffer); // Lock the mutex to access shared data safely

        // Add an item to the buffer
        myBuffer->buffer[myBuffer->inIndex] = rand() % (1000 - 1 + 1) + 0; // Simulate producing an item
        printf("Producer %d added item %d at index %d.\n", id, myBuffer->buffer[myBuffer->inIndex], myBuffer->inIndex);

        myBuffer->inIndex = (myBuffer->inIndex + 1) % myBuffer->bufferSize; // Update the index for the next item
        myBuffer->bufferCount++;              // Increment the item count
        printf("Items in buffer after producer %d: %d\n", id, myBuffer->bufferCount);
        
        // Print the current state of the buffer
        printBuffer(myBuffer->buffer, myBuffer->bufferSize);

        pthread_mutex_unlock(&myBuffer->mutexToAccessBuffer); // Unlock the mutex after updating shared data
        sem_post(&myBuffer->fullSlot);         // Signal that there is now one more filled slot

        sleep(rand() % 3 + 1);                // Sleep for 1-3 seconds to simulate variable production time
    }
}

// Consumer thread function
void *consumer(void *arg) {
    Buffer *myBuffer = (Buffer *)arg;        // Cast the argument to a Buffer pointer
    int id = pthread_self() % 10000;         // Generate a pseudo-unique thread ID

    while (1) {                              // Infinite loop for continuous consumption
        sem_wait(&myBuffer->fullSlot);       // Wait until there is at least one filled slot
        pthread_mutex_lock(&myBuffer->mutexToAccessBuffer); // Lock the mutex to access shared data safely

        // Remove an item from the buffer
        int item = myBuffer->buffer[myBuffer->outIndex]; // Simulate retrieving the item
        myBuffer->buffer[myBuffer->outIndex] = 0;
        printf("Consumer %d removed item %d from index %d.\n", id, item, myBuffer->outIndex);

        myBuffer->outIndex = (myBuffer->outIndex + 1) % myBuffer->bufferSize; // Update the index for the next item
        myBuffer->bufferCount--;             // Decrement the item count
        printf("Items in buffer after consumer %d: %d\n", id, myBuffer->bufferCount);
        
        // Print the current state of the buffer
        printBuffer(myBuffer->buffer, myBuffer->bufferSize);

        pthread_mutex_unlock(&myBuffer->mutexToAccessBuffer); // Unlock the mutex after updating shared data
        sem_post(&myBuffer->emptySlot);      // Signal that there is now one more empty slot

        sleep(rand() % 3 + 1);               // Sleep for 1-3 seconds to simulate variable consumption time
    }
}


int main() {
    int bufferSize, numProducers, numConsumers, loopVar;

    // Get user input for buffer size, number of producers, and consumers
    do{
    	printf("Enter buffer size: ");
    	scanf("%d", &bufferSize);
	}while(bufferSize<=0);
	do{
	    printf("Enter number of producers: ");
	    scanf("%d", &numProducers);
	}while(numProducers<=0);
	do{
		printf("Enter number of consumers: ");
    	scanf("%d", &numConsumers);
	}while(numConsumers<=0);
	
    

    srand(time(NULL));  // Seed the random number generator for random sleep times

    // Initialize the buffer and synchronization primitives
    Buffer myBuffer;
    myBuffer.bufferSize = bufferSize;
    myBuffer.buffer = (int *)malloc(bufferSize * sizeof(int)); // Allocate memory for the buffer
    
    for(loopVar = 0; loopVar< bufferSize; loopVar++){
    	myBuffer.buffer[loopVar] = 0;
	}
    myBuffer.inIndex = 0;             // Initialize producer index
    myBuffer.outIndex = 0;            // Initialize consumer index
    myBuffer.bufferCount = 0;         // Initialize item count
    sem_init(&myBuffer.fullSlot, 0, 0); // Initialize "full" semaphore with 0 (no items initially)
    sem_init(&myBuffer.emptySlot, 0, bufferSize); // Initialize "empty" semaphore with the buffer size
    pthread_mutex_init(&myBuffer.mutexToAccessBuffer, NULL); // Initialize the mutex

    // Allocate memory for producer and consumer thread handles
    pthread_t *producers = (pthread_t *)malloc(numProducers * sizeof(pthread_t));
    pthread_t *consumers = (pthread_t *)malloc(numConsumers * sizeof(pthread_t));

    // Create producer threads
    for (loopVar = 0; loopVar < numProducers; loopVar++) {
        pthread_create(&producers[loopVar], NULL, producer, (void *)&myBuffer);
    }

    // Create consumer threads
    for (loopVar = 0; loopVar < numConsumers; loopVar++) {
        pthread_create(&consumers[loopVar], NULL, consumer, (void *)&myBuffer);
    }

    // Wait for all producer threads to finish (infinite loop means this won't happen)
    for (loopVar = 0; loopVar < numProducers; loopVar++) {
        pthread_join(producers[loopVar], NULL);
    }

    // Wait for all consumer threads to finish (infinite loop means this won't happen)
    for (loopVar = 0; loopVar < numConsumers; loopVar++) {
        pthread_join(consumers[loopVar], NULL);
    }

    // Free allocated resources and destroy synchronization primitives
    free(myBuffer.buffer);                  // Free the buffer memory
    free(producers);                        // Free producer thread handles
    free(consumers);                        // Free consumer thread handles
    sem_destroy(&myBuffer.fullSlot);        // Destroy the "full" semaphore
    sem_destroy(&myBuffer.emptySlot);       // Destroy the "empty" semaphore
    pthread_mutex_destroy(&myBuffer.mutexToAccessBuffer); // Destroy the mutex

    return 0; // Exit the program
}

