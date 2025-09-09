#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define THINKING 0
#define HUNGRY 1
#define EATING 2

// To prevent deadlock in the Dining Philosophers problem:
// Philosophers can only be allowed to pickup his chopsticks if both chopsticks are available at their critical time (or time of holding mutex)

struct SharedData{
	int numberOfPhilosophers;
	int *philosopherState;
	sem_t mutexToChangeState;
	sem_t *philosopherSemaphore;
};

struct PhilosopherArgs{
	int id;
	struct SharedData *data;
};

// Function to get the left Id of a philosopher, modulo is used to ensure it limits 0 to numberOfPhilosophers
int getLeft(int philosopherId, int numberOfPhilosophers){
	return (philosopherId + numberOfPhilosophers - 1) % numberOfPhilosophers;
}

// Function to get the right Id of a philosopher
int getRight(int philosopherId, int numberOfPhilosophers){
	return (philosopherId + 1) % numberOfPhilosophers;
}

// This function is called once a Philosopher changes from thinking to hungry
// They check if they can eat, which is only the case if philosophers beside them aren't both eating
// This is true since they share forks
// Example Philosopher 3 needs fork 2 and 3
// Philosopher 2 needs fork 2 while Philosopher 4 needs fork 3
// If a Philosopher successfully eats, they increment their own semaphore
// This is relevant to the takeFork function and putFork function
void checkCanEat(int philosopherId, struct SharedData *data){
	int numberOfPhilosophers = data->numberOfPhilosophers;
	if (data->philosopherState[philosopherId] == HUNGRY
        && data->philosopherState[getLeft(philosopherId, numberOfPhilosophers)] != EATING
        && data->philosopherState[getRight(philosopherId, numberOfPhilosophers)] != EATING) {
        // If current Philosopher is hungry AND nearby philosophers aren't eating
        data->philosopherState[philosopherId] = EATING; // Set current philosopher to eating

        sleep(2); // Sleep for 2 seconds, intended so program doesn't scroll too fast
        printf("Philosopher %d takes fork %d and %d\n", 
               philosopherId + 1, getLeft(philosopherId, numberOfPhilosophers) + 1, philosopherId + 1);
        printf("Philosopher %d is Eating\n", philosopherId + 1);
		// Show that current philosopher is taking forks and eating
        sem_post(&data->philosopherSemaphore[philosopherId]);
        // This is intended for 2 methods
        // (1) If ever Philosopher is hungry and attempts to take fork but cannot eat due to the condition
        // Then they will be signaled to eat RIGHT after a philosopher beside them finishes eating
        // This will be seen in the putFork function
        // (2) If ever Philosopher is hungry and attempts to take fork and is successful
        // Then they proceed normally since takeFork function's wait function will immediately decrement the post
        // That happened in this function
	}
}

void takeFork(int philosopherId, struct SharedData *data){
	sem_wait(&data->mutexToChangeState); // The ability to change states is done one at a time only for stability
	
	sleep(rand() % 3 + 1);  // Simulating random time before getting hungry
	
	data->philosopherState[philosopherId] = HUNGRY;
	
	printf("Philosopher %d is Hungry\n", philosopherId + 1); // Show philosopher is hungry

    checkCanEat(philosopherId, data); // Try to eat (not guaranteed)

    sem_post(&data->mutexToChangeState); // Give other philosophers the ability to change states
    sem_wait(&data->philosopherSemaphore[philosopherId]); 
    // If having eaten, then this automatically executes
	// If not, wait until allowed to eat (pinged by a nearby philosopher putting down their fork)

    sleep(1);
}

void putFork(int philosopherId, struct SharedData *data){
	sem_wait(&data->mutexToChangeState); // The ability to change states is done one at a time only for stability

    data->philosopherState[philosopherId] = THINKING; // Now thinking, since done eating
    printf("Philosopher %d putting fork %d and %d down\n",
           philosopherId + 1, getLeft(philosopherId, data->numberOfPhilosophers) + 1, philosopherId + 1);
    printf("Philosopher %d is Thinking\n", philosopherId + 1);

	// This is now where a philosopher who is done eating signals nearby philosophers to eat if they were hungry
	// They would now be able to eat as conditions satisfy, and post now allows them to exit the takeFork function
	// As their semaphore can now be decremented
    checkCanEat(getLeft(philosopherId, data->numberOfPhilosophers), data);  // Check if left neighbor can eat
    checkCanEat(getRight(philosopherId, data->numberOfPhilosophers), data); // Check if right neighbor can eat

    sem_post(&data->mutexToChangeState); // Give other philosophers the ability to change states
}

void* philosopherRoutine(void* args) {
	// Cast the argument to the struct PhilosopherArgs * type
	// This is because the argument is casted to void prior to creating the thread
    struct PhilosopherArgs *philosopherArgs = (struct PhilosopherArgs *)args;
    int id = philosopherArgs->id; // Assign an id to the philosopher
    struct SharedData *data = philosopherArgs->data; // Assigned shared data
	srand(time(NULL) + id); // Randomize philosopher's time parameters
    while (1) {
        // Thinking for a random time between 1 and 3 seconds
        //printf("Philosopher %d is Thinking\n", id + 1);
        sleep(rand() % 3 + 1);  // Thinking for 1-3 seconds

        // Getting hungry and trying to take the fork
        takeFork(id, data);

        // Eating for a random time between 1 and 3 seconds
        //printf("Philosopher %d is Eating\n", id + 1);
        sleep(rand() % 3 + 1);  // Eating for 1-3 seconds

        // Putting forks down and thinking again
        putFork(id, data);
    }
}

int main() {
    int numberOfPhilosophers;
	// Ask for the number of philosophers, minimum is 2
	do{
		printf("Enter the number of philosophers: ");
    	scanf("%d", &numberOfPhilosophers);	
	}while(numberOfPhilosophers<2);
    
    // Allocate shared data, a struct containing all relevant info
    struct SharedData data;
    data.numberOfPhilosophers = numberOfPhilosophers;
    data.philosopherState = (int*)malloc(numberOfPhilosophers * sizeof(int));
    data.philosopherSemaphore = (sem_t*)malloc(numberOfPhilosophers * sizeof(sem_t));

	// We make the threads
    pthread_t *thread_id = (pthread_t*)malloc(numberOfPhilosophers * sizeof(pthread_t));
    
    // We make philosopherArgs, which contains id and shared data
    // We do this since philosopherRoutine, the thread function, needs 1 void argument
    struct PhilosopherArgs *philosopherArgs = (struct PhilosopherArgs*)malloc(numberOfPhilosophers * sizeof(struct PhilosopherArgs));

    // Initialize semaphores and states
    // We initialize the mutex for changing states
    // 0 signifies shared between threads, then the 1 signifies only 1 can access it at a time
    sem_init(&data.mutexToChangeState, 0, 1);
    
    int loopVar = 0;
    // In this loop, we initialize philosopherStates to thinking
    // Initialize philosopherSemaphore (shared between threads, and 0 since it is a polling system)
    // Initialize philosopherArgs to id and point to shared data
    for (loopVar = 0; loopVar < numberOfPhilosophers; loopVar++) {
        data.philosopherState[loopVar] = THINKING;
        sem_init(&data.philosopherSemaphore[loopVar], 0, 0);
        philosopherArgs[loopVar].id = loopVar;
        philosopherArgs[loopVar].data = &data;
    }

    // Create philosopher threads
    // Each thread will run the philosopherRoutine with argument philosopherArgs containing their id
    // Then immediately start thinking
    for (loopVar = 0; loopVar < numberOfPhilosophers; loopVar++) {
        pthread_create(&thread_id[loopVar], NULL, philosopherRoutine, &philosopherArgs[loopVar]);
        printf("Philosopher %d is Thinking\n", loopVar + 1);
    }

    // Join threads (keeps the main function alive)
    // Main function will wait until all threads finish executing
    for (loopVar = 0; loopVar < numberOfPhilosophers; loopVar++) {
        pthread_join(thread_id[loopVar], NULL);
    }

    // Cleanup
    free(data.philosopherState);
    free(data.philosopherSemaphore);
    free(thread_id);
    free(philosopherArgs);
    sem_destroy(&data.mutexToChangeState);
    for (loopVar = 0; loopVar < numberOfPhilosophers; loopVar++) {
        sem_destroy(&data.philosopherSemaphore[loopVar]);
    }

    return 0;
}
