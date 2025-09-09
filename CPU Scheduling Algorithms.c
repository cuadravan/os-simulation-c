#include <stdio.h>
#include <stdlib.h>

enum Priority{ // For use in priority of processes
	LOW = 3,
	MEDIUM = 2,
	HIGH = 1
};

enum SortingMetric{ // For use in linked list insertions
	LEASTARRIVALTIME,
	LEASTBURSTTIME,
	HIGHESTPRIORITY,
	PROCESSID
};

struct Process{
	int processID;
	int arrivalTime;
	int originalArrivalTime;
	int burstTime;
	enum Priority priority;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
    int remainingTime;
    struct Process *nextPtr;
}; // struct for the Process, containing all relevant variables

// INPUT FUNCTIONS
void clearInputBuffer();
void insertToJobQueue(struct Process **JobQueue, int processNumber, int arrivalTime, int burstTime, enum Priority priority);
void clearProcesses(struct Process **JobQueue);

// ALGORITHM FUNCTIONS
void firstComeFirstServe(struct Process **JobQueue, int numberOfProcesses);
void roundRobinScheduling(struct Process **JobQueue, int numberOfProcesses, int timeQuantum);
void firstComeFirstServeWithoutIdleTime(struct Process **JobQueue, int numberOfProcesses);
void roundRobinSchedulingWithoutIdleTime(struct Process **JobQueue, int numberOfProcesses, int timeQuantum);

// SCHEDULING FUNCTIONS
void longTermScheduler(struct Process **ReadyQueue, struct Process **JobQueue, int currentTime, enum SortingMetric sortingMetric);
void longTermSchedulerNoIdleTime(struct Process **JobQueue, struct Process **ReadyQueue, int currentTime, enum SortingMetric sortingMetric);
void insertToQueue(struct Process **ReadyQueue, struct Process *PtrToTransfer, enum SortingMetric sortingMetric);
void finishProcess(struct Process **ReadyQueue, struct Process **TerminationQueue);
void moveFirstToEnd(struct Process **ReadyQueue);
void splitQueue(struct Process **JobQueue, struct Process **NewJobQueue, enum Priority priority);

// INFORMATION FUNCTIONS
void printTable(struct Process *JobQueue);
void printAveragesHeader();
void printAverages(struct Process *JobQueue, int totalCompletionTime, double numberOfProcesses);

// MISCELLANEOUS FUNCTIONS
void printLinkedList(struct Process *head);

int main(){
	struct Process *JobQueue = NULL;
	
	int numberOfProcesses = 7;
	
	insertToJobQueue(&JobQueue, 1, 9, 5, HIGH);
	insertToJobQueue(&JobQueue, 2, 3, 7, MEDIUM);
	insertToJobQueue(&JobQueue, 3, 40, 4, LOW);
	insertToJobQueue(&JobQueue, 4, 2, 8, HIGH);
	insertToJobQueue(&JobQueue, 5, 35, 8, MEDIUM);
	insertToJobQueue(&JobQueue, 6, 42, 3, LOW);
	insertToJobQueue(&JobQueue, 7, 4, 10, HIGH);
	
	printf("\nTEST CASE 1: \n");
	
	firstComeFirstServe(&JobQueue, numberOfProcesses); // Execute FCFS
	firstComeFirstServeWithoutIdleTime(&JobQueue, numberOfProcesses); // Execute FCFS
	roundRobinScheduling(&JobQueue, numberOfProcesses, 3);
	roundRobinSchedulingWithoutIdleTime(&JobQueue, numberOfProcesses, 3);
	
	clearProcesses(&JobQueue); 
	
	insertToJobQueue(&JobQueue, 1, 30, 5, HIGH);
	insertToJobQueue(&JobQueue, 2, 3, 7, MEDIUM);
	insertToJobQueue(&JobQueue, 3, 45, 4, LOW);
	insertToJobQueue(&JobQueue, 4, 2, 8, HIGH);
	insertToJobQueue(&JobQueue, 5, 35, 8, MEDIUM);
	insertToJobQueue(&JobQueue, 6, 47, 3, LOW);
	insertToJobQueue(&JobQueue, 7, 4, 10, HIGH);	
	
	printf("\n\nTEST CASE 2: \n");
	
	firstComeFirstServe(&JobQueue, numberOfProcesses); // Execute FCFS
	firstComeFirstServeWithoutIdleTime(&JobQueue, numberOfProcesses); // Execute FCFS
	roundRobinScheduling(&JobQueue, numberOfProcesses, 3);
	roundRobinSchedulingWithoutIdleTime(&JobQueue, numberOfProcesses, 3);
	
	
	clearProcesses(&JobQueue); 
	
	return 0;
}

void insertToJobQueue(struct Process **JobQueue, int processNumber, int arrivalTime, int burstTime, enum Priority priority){
    struct Process *newPtr = malloc(sizeof(struct Process)); // First we dynamically allocate a memory of size struct Process
    
    if(newPtr != NULL){ // We check if this memory was successfully allocated 
        newPtr->processID = processNumber; // Then we input all the relevant information into the node
        newPtr->arrivalTime = arrivalTime;
        newPtr->originalArrivalTime = arrivalTime;
        newPtr->burstTime = burstTime;
        newPtr->priority = priority;
        newPtr->completionTime = 0; // These are set to zero as we have yet to use an algorithm
        newPtr->turnaroundTime = 0;
        newPtr->waitingTime = 0;
        newPtr->remainingTime = burstTime; // Remaining time defaults to burst time as it is full
        newPtr->nextPtr = NULL;
        struct Process *previousPtr = NULL;
        struct Process *currentPtr = *JobQueue;

        while(currentPtr != NULL){ // Then we traverse to the end of JobQueue for insertion
            previousPtr = currentPtr;
            currentPtr = currentPtr->nextPtr;
        }
        if(previousPtr == NULL){ // This means we are at the beginning, or we did not traverse
            newPtr->nextPtr = *JobQueue; // Hence, newPtr will point at previous first Node or NULL
            *JobQueue = newPtr; // Then newPtr is now the headPointer or first Node
        }
        else{ // Otherwise, we insert the new node in between the previousPtr and currentPtr
            previousPtr->nextPtr = newPtr; // previousPtr now points to newPtr
            newPtr->nextPtr = currentPtr; // newPtr now points to currentPtr
        }
    }
    else{ // If ever memory allocation fails, we notify.
        printf("Process %d is not inserted as there is no more memory.\n", processNumber);
    }
}

void clearProcesses(struct Process **JobQueue) { // This works like a deQueue or removing first node, and done again and again until list is empty
    while (*JobQueue != NULL) {  // Loop until all nodes are deleted
        struct Process *tempPtr = *JobQueue;  // Temporarily store the current node
        *JobQueue = (*JobQueue)->nextPtr;  // Move head pointer to the next node
        free(tempPtr);  // Free the former head node
    }
}

void firstComeFirstServe(struct Process **JobQueue, int numberOfProcesses){
	printf("\n\nProceeding to First Come First Serve Algorithm with Idle Times...\n\n"); 
	// If schedulingMode is individual, algorithm will print this along with Gantt Chart, Table of Processes, and other headers and information
	enum SortingMetric sortingMetric = LEASTARRIVALTIME;
	// First Come First Serve executes processes with lowest arrival time first
	struct Process *ReadyQueue = NULL; // We create a local Ready Queue and Termination Queue to be used within the algorithm
    struct Process *TerminationQueue = NULL;
    
    printf("Gantt chart: \n");
    int currentTime = 0; // Current time starts at zero
    
    while(*JobQueue != NULL || ReadyQueue != NULL){ // We will keep executing the algorithm until all processes have arrived AND been executed
        longTermScheduler(JobQueue, &ReadyQueue, currentTime, sortingMetric); // This lets processes arrive at current time
    	printf("[%d", currentTime);	// Print starting time of process in Gantt Chart
		if(ReadyQueue!=NULL){ // If there are processes that have arrived, or already arrived but not executed within the ReadyQueue
			printf(" %c ", ReadyQueue->processID+64); // Print PID of current process in Gantt Chart
			currentTime += ReadyQueue->burstTime; // Add current time with burst time
            ReadyQueue->completionTime = currentTime; // Completion time will be calculated
            ReadyQueue->turnaroundTime = ReadyQueue->completionTime - ReadyQueue->arrivalTime; // TAT = CT - AT
            ReadyQueue->waitingTime = ReadyQueue->turnaroundTime - ReadyQueue->burstTime; // WT = TAT - BT
            finishProcess(&ReadyQueue, &TerminationQueue); // Then transfer currently executed process to TerminationQueue
        }
        else{
            currentTime += 1; // If there are no arrived processes, we let the time increase by 1 (IDLE)
        	printf(" __ "); // Print an idle box for Gantt Chart
        }
        printf("%d]", currentTime);	// Print ending time of current process in Gantt Chart
    }
    *JobQueue = TerminationQueue; // Once all processes are in Termination Queue (which is ordered by PID), we let JobQueue reference it
    TerminationQueue = NULL; // Then TerminationQueue will now reference null
	printTable(*JobQueue); // Only print Table and Averages Header if Individual
	printAveragesHeader();
	printAverages(*JobQueue, currentTime, numberOfProcesses); // Then print Total Completion Time, Avg TAT, Avg CT, and Throughput
}

void firstComeFirstServeWithoutIdleTime(struct Process **JobQueue, int numberOfProcesses){
	printf("\n\nProceeding to First Come First Serve Algorithm without Idle Times...\n\n"); 
	 // If schedulingMode is individual, algorithm will print this along with Gantt Chart, Table of Processes, and other headers and information
	enum SortingMetric sortingMetric = LEASTARRIVALTIME;
	// First Come First Serve executes processes with lowest arrival time first
	struct Process *ReadyQueue = NULL; // We create a local Ready Queue and Termination Queue to be used within the algorithm
    struct Process *TerminationQueue = NULL;
    
    printf("Gantt chart: \n");
	
    int currentTime = 0; // Current time starts at zero
    
    while(*JobQueue != NULL || ReadyQueue != NULL){ // We will keep executing the algorithm until all processes have arrived AND been executed
		longTermScheduler(JobQueue, &ReadyQueue, currentTime, sortingMetric); // This lets processes arrive at current time
        printf("[%d", currentTime);	// Print starting time of process in Gantt Chart
	
		if(ReadyQueue!=NULL){ // If there are processes that have arrived, or already arrived but not executed within the ReadyQueue
			printf(" %c ", ReadyQueue->processID+64); // Print PID of current process in Gantt Chart
			currentTime += ReadyQueue->burstTime; // Add current time with burst time
            ReadyQueue->completionTime = currentTime; // Completion time will be calculated
            ReadyQueue->turnaroundTime = ReadyQueue->completionTime - ReadyQueue->arrivalTime; // TAT = CT - AT
            ReadyQueue->waitingTime = ReadyQueue->turnaroundTime - ReadyQueue->burstTime; // WT = TAT - BT
            finishProcess(&ReadyQueue, &TerminationQueue); // Then transfer currently executed process to TerminationQueue
        }
        else{
            longTermSchedulerNoIdleTime(JobQueue, &ReadyQueue, currentTime, sortingMetric);
			printf(" %c ", ReadyQueue->processID+64); // Print PID of current process in Gantt Chart
			ReadyQueue->arrivalTime = currentTime;
			//printf("ARRIVAL TIME: %d", ReadyQueue->arrivalTime);
			currentTime += ReadyQueue->burstTime; // Add current time with burst time
            ReadyQueue->completionTime = currentTime; // Completion time will be calculated
            ReadyQueue->turnaroundTime = ReadyQueue->completionTime - ReadyQueue->arrivalTime; // TAT = CT - AT
            ReadyQueue->waitingTime = ReadyQueue->turnaroundTime - ReadyQueue->burstTime; // WT = TAT - BT
            finishProcess(&ReadyQueue, &TerminationQueue); // Then transfer currently executed process to TerminationQueue
//            printf("Printing Job Queue: ");
//			printLinkedList(*JobQueue);
//			printf("\n");
			}
        printf("%d]", currentTime);	// Print ending time of current process in Gantt Chart
		
    }
    *JobQueue = TerminationQueue; // Once all processes are in Termination Queue (which is ordered by PID), we let JobQueue reference it
    TerminationQueue = NULL; // Then TerminationQueue will now reference null
	printTable(*JobQueue); // Only print Table and Averages Header if Individual
	printAveragesHeader();

	printAverages(*JobQueue, currentTime, numberOfProcesses); // Then print Total Completion Time, Avg TAT, Avg CT, and Throughput
}

void roundRobinScheduling(struct Process **JobQueue, int numberOfProcesses, int timeQuantum){
	printf("\n\nProceeding to Round Robin Scheduling with Idle Times...\n\n");
	enum SortingMetric sortingMetric = LEASTARRIVALTIME; // Round Robin follows a FCFS sorting within the Ready Queue, but it is preemptive wherein it only processes a specific time quantum
	
	struct Process *ReadyQueue = NULL;
    struct Process *TerminationQueue = NULL;
    
    int currentTime = 0;
    int roundRobin = 0; // We have a variable to note if a process should be placed to the end of the queue
    
    while(*JobQueue != NULL || ReadyQueue != NULL) {
        longTermScheduler(JobQueue, &ReadyQueue, currentTime, sortingMetric); // Note that we let new processes ARRIVE FIRST before doing a round robin
        if(roundRobin == 1) { // If round robin is 1, we will move the first process to the end
            moveFirstToEnd(&ReadyQueue);
        }
		printf("[%d", currentTime); 
        if(ReadyQueue != NULL) {
			printf(" %c ", ReadyQueue->processID+64);
            if(ReadyQueue->remainingTime <= timeQuantum) { // If current process will be finished with one time quantum
                currentTime += ReadyQueue->remainingTime; // We add current time with remaining time (since sometimes, processes' remaining time is less than time quantum, so full time quantum is not consumed)
                ReadyQueue->remainingTime = 0; // Remaining time is now zero, since process is finished
                ReadyQueue->completionTime = currentTime; // Set CT to currentTime of finishing
                ReadyQueue->turnaroundTime = ReadyQueue->completionTime - ReadyQueue->arrivalTime;
                ReadyQueue->waitingTime = ReadyQueue->turnaroundTime - ReadyQueue->burstTime;
                finishProcess(&ReadyQueue, &TerminationQueue);
                roundRobin = 0; // Set roundRobin to zero, since we instead moved process to termination queue, so no round robin
            }
            else { // Else if process is not finishable by one time quantum
                currentTime += timeQuantum; // Move time forward by one time quantum
                ReadyQueue->remainingTime -= timeQuantum; // Remove one time quantum from remaining time
                roundRobin = 1; // Then set round robin to 1, so that after new processes arrive at new time, we move current process to back
            }
        }
        else {
            currentTime += 1;
            printf(" __ ");
        }
        printf("%d]", currentTime);
    }
    *JobQueue = TerminationQueue;
    TerminationQueue = NULL;
	printTable(*JobQueue);
	printAveragesHeader();
	printAverages(*JobQueue, currentTime, numberOfProcesses); 
}

void roundRobinSchedulingWithoutIdleTime(struct Process **JobQueue, int numberOfProcesses, int timeQuantum){
	printf("\n\nProceeding to Round Robin Scheduling without Idle Times...\n\n");
	enum SortingMetric sortingMetric = LEASTARRIVALTIME; // Round Robin follows a FCFS sorting within the Ready Queue, but it is preemptive wherein it only processes a specific time quantum
	
	struct Process *ReadyQueue = NULL;
    struct Process *TerminationQueue = NULL;
    
    int currentTime = 0;
    int roundRobin = 0; // We have a variable to note if a process should be placed to the end of the queue
    
    while(*JobQueue != NULL || ReadyQueue != NULL) {
        longTermScheduler(JobQueue, &ReadyQueue, currentTime, sortingMetric); // Note that we let new processes ARRIVE FIRST before doing a round robin
        if(ReadyQueue == NULL){
        	longTermSchedulerNoIdleTime(JobQueue, &ReadyQueue, currentTime, sortingMetric);
        	ReadyQueue->arrivalTime = currentTime;
		}
		if(roundRobin == 1) { // If round robin is 1, we will move the first process to the end
            moveFirstToEnd(&ReadyQueue);
        }
		printf("[%d", currentTime); 
        if(ReadyQueue != NULL) {
			printf(" %c ", ReadyQueue->processID+64);
            if(ReadyQueue->remainingTime <= timeQuantum) { // If current process will be finished with one time quantum
                currentTime += ReadyQueue->remainingTime; // We add current time with remaining time (since sometimes, processes' remaining time is less than time quantum, so full time quantum is not consumed)
                ReadyQueue->remainingTime = 0; // Remaining time is now zero, since process is finished
                ReadyQueue->completionTime = currentTime; // Set CT to currentTime of finishing
                ReadyQueue->turnaroundTime = ReadyQueue->completionTime - ReadyQueue->arrivalTime;
                ReadyQueue->waitingTime = ReadyQueue->turnaroundTime - ReadyQueue->burstTime;
                finishProcess(&ReadyQueue, &TerminationQueue);
                roundRobin = 0; // Set roundRobin to zero, since we instead moved process to termination queue, so no round robin
            }
            else { // Else if process is not finishable by one time quantum
                currentTime += timeQuantum; // Move time forward by one time quantum
                ReadyQueue->remainingTime -= timeQuantum; // Remove one time quantum from remaining time
                roundRobin = 1; // Then set round robin to 1, so that after new processes arrive at new time, we move current process to back
            }
        }
        printf("%d]", currentTime);
    }
    *JobQueue = TerminationQueue;
    TerminationQueue = NULL;
	printTable(*JobQueue);
	printAveragesHeader();
	printAverages(*JobQueue, currentTime, numberOfProcesses); 
}


// longTermScheduler checks for processes that should HAVE ARRIVED given currentTime and inserts them to Ready Queue based on a sorting Metric 
void longTermSchedulerNoIdleTime(struct Process **JobQueue, struct Process **ReadyQueue, int currentTime, enum SortingMetric sortingMetric){
    struct Process *currentPtr = (*JobQueue)->nextPtr; // Assign first node of JobQueue to currentPtr
    struct Process *PtrToTransfer = *JobQueue; // create a PtrToTransfer, which holds process to transfer to ready queue
    struct Process *prevPtr = NULL;
    
    while(currentPtr != NULL){ // Loop through entire Job Queue
        if(currentPtr->arrivalTime <= PtrToTransfer->arrivalTime){ // Check if process should have arrived (lesser than or equal to currentTime)
            PtrToTransfer = currentPtr; // Assign this arrived process to Ptr to transfer    
			              
        } // Assigning new currentPtr based on if whether we traversed or not
        // If we did not, new currentPtr is the head pointer which was the former 2nd node after former transferred node
        // If we did, we assign it to the node after the former transferred node
        prevPtr = currentPtr;
		currentPtr = currentPtr->nextPtr;
    }
    prevPtr = NULL;
    currentPtr = *JobQueue;
    while(currentPtr != NULL){
    	if(currentPtr == PtrToTransfer){
    		break;
		}
		prevPtr = currentPtr;
		currentPtr = currentPtr->nextPtr;
	}
    if(prevPtr == NULL) { // If prevPtr is null, we need to fix the headPointer to next node
        *JobQueue = (*JobQueue)->nextPtr;
    } 
	else{ // If not, previousPointer now points to the node after currentPtr
        prevPtr->nextPtr = currentPtr->nextPtr;
    }
    PtrToTransfer->nextPtr = NULL; // Delink PtrToTransfer from JobQueue
    insertToQueue(ReadyQueue, PtrToTransfer, sortingMetric); // Then insert it to the queue based on a sorting metric
//    currentPtr = (prevPtr == NULL) ? *JobQueue : prevPtr->nextPtr;
}

// longTermSchedulerNoIdleTime checks for processes that should HAVE ARRIVED given currentTime and inserts them to Ready Queue based on a sorting Metric 
void longTermScheduler(struct Process **JobQueue, struct Process **ReadyQueue, int currentTime, enum SortingMetric sortingMetric){
    struct Process *currentPtr = *JobQueue; // Assign first node of JobQueue to currentPtr
    struct Process *PtrToTransfer = NULL; // create a PtrToTransfer, which holds process to transfer to ready queue
    struct Process *prevPtr = NULL;
    while(currentPtr != NULL){ // Loop through entire Job Queue
        if(currentPtr->arrivalTime <= currentTime){ // Check if process should have arrived (lesser than or equal to currentTime)
            PtrToTransfer = currentPtr; // Assign this arrived process to Ptr to transfer
            if(prevPtr == NULL) { // If prevPtr is null, we need to fix the headPointer to next node
                *JobQueue = (*JobQueue)->nextPtr;
            } 
			else{ // If not, previousPointer now points to the node after currentPtr
                prevPtr->nextPtr = currentPtr->nextPtr;
            }
            PtrToTransfer->nextPtr = NULL; // Delink PtrToTransfer from JobQueue
            insertToQueue(ReadyQueue, PtrToTransfer, sortingMetric); // Then insert it to the queue based on a sorting metric
            currentPtr = (prevPtr == NULL) ? *JobQueue : prevPtr->nextPtr;
        } // Assigning new currentPtr based on if whether we traversed or not
        // If we did not, new currentPtr is the head pointer which was the former 2nd node after former transferred node
        // If we did, we assign it to the node after the former transferred node
		else{ // Otherwise, we walk to next node
            prevPtr = currentPtr; 
            currentPtr = currentPtr->nextPtr;
        }
    }
}

// Simple function that finds the correct spot for a process to be inserted then inserts it there basing on the passed sorting metric
void insertToQueue(struct Process **Queue, struct Process *PtrToTransfer, enum SortingMetric sortingMetric){
    struct Process *previousPtr = NULL;
	struct Process *currentPtr = *Queue;
	
	if(sortingMetric == LEASTARRIVALTIME){ // We sort it with ascending arrival time, process that arrived first will be ahead in the queue
		while(currentPtr != NULL && currentPtr->arrivalTime <= PtrToTransfer->arrivalTime){
		    previousPtr = currentPtr;
		    currentPtr = currentPtr->nextPtr;
		}	
	}
	else if(sortingMetric == LEASTBURSTTIME){ // We sort it with ascending burst time, but if same burst time then apply FCFS
		while (currentPtr != NULL && 
    	(currentPtr->burstTime < PtrToTransfer->burstTime || 
    	(currentPtr->burstTime == PtrToTransfer->burstTime && currentPtr->arrivalTime <= PtrToTransfer->arrivalTime))){
        	previousPtr = currentPtr;
        	currentPtr = currentPtr->nextPtr;
    	}
	}
	else if(sortingMetric == HIGHESTPRIORITY){ // We sort it with ascending priority, but if same burst time then apply FCFS
		while (currentPtr != NULL && 
    	(currentPtr->priority < PtrToTransfer->priority || 
    	(currentPtr->priority == PtrToTransfer->priority && currentPtr->arrivalTime <= PtrToTransfer->arrivalTime))){ 
        	previousPtr = currentPtr;
        	currentPtr = currentPtr->nextPtr;
    	}
	}
	else if(sortingMetric == PROCESSID){ // We sort it with process ID (ascending, following original order of input)
		while(currentPtr != NULL && currentPtr->processID <= PtrToTransfer->processID){
        	previousPtr = currentPtr;
        	currentPtr = currentPtr->nextPtr;
    	}
	}
	
	if(previousPtr == NULL){ // If we did not traverse at all
	    PtrToTransfer->nextPtr = *Queue; // Assign transferred Process to point at first process in the queue
	    *Queue = PtrToTransfer; // Then queue's head pointer will now be the transferred Process
	}
	else{ // Otherwise, we insert the transferred process between the prev and current Ptr
	    previousPtr->nextPtr = PtrToTransfer; // previous Pointer now points to transferred process
	    PtrToTransfer->nextPtr = currentPtr; // transferred process now points to current pointer
	}
}

// This function is applied to the first process in a Ready Queue, which has been deemed fully completed
// First it resets remaining time and moves process to Termination Queue
void finishProcess(struct Process **ReadyQueue, struct Process **TerminationQueue){
    enum SortingMetric sortingMetric = PROCESSID; // So that terminationQueue follows original order of input of processes, we sort by ProcessID
	struct Process *currentPtr = *ReadyQueue; // Assign currentPtr to first node of Ready Queue
    if(currentPtr != NULL){
    	currentPtr->remainingTime = currentPtr->burstTime; // Reset remaining time
	    *ReadyQueue = (*ReadyQueue)->nextPtr; // ReadyQueue's first node is now the next node
	    currentPtr->nextPtr = NULL; // Delink currentPtr from readyqueue
	    insertToQueue(TerminationQueue, currentPtr, sortingMetric); // Insert it to termination queue based on processID
	}       
}

void moveFirstToEnd(struct Process **ReadyQueue){ // Used in round robin to move first node of a queue to the back
    if (*ReadyQueue == NULL || (*ReadyQueue)->nextPtr == NULL) { // First check if ReadyQueue is not empty
        return;
    }
    struct Process* first = *ReadyQueue; // Asign firstPointer to first node
    struct Process* second = first->nextPtr; // Assign secondPointer to second node

    struct Process* temp = *ReadyQueue; // Store first node in a temporary Pointer
    while (temp->nextPtr != NULL) { // Traverse to the last node
        temp = temp->nextPtr;
    }

    temp->nextPtr = first; // last node will now point to the first node
    *ReadyQueue = second; // Queue will now be from the 2nd node
    first->nextPtr = NULL; // first Node now points to last, making it the last node now
}

// This function is VERY SIMILAR to longTermScheduler, except it checks for priority instead of arrivalTime and transfer process to the new Queue
// Hence, comments from longTermScheduler also carries over
void splitQueue(struct Process **JobQueue, struct Process **NewJobQueue, enum Priority priority){
    enum SortingMetric sortingMetric = PROCESSID;
    // Processes will be sorted by processID within the new Queue
	struct Process *currentPtr = *JobQueue;
    struct Process *PtrToTransfer = NULL;
    struct Process *prevPtr = NULL;
    while(currentPtr != NULL){
        if(currentPtr->priority == priority){ // Check if process matches priority of the new queue
            PtrToTransfer = currentPtr;
            if(prevPtr == NULL) {
                *JobQueue = (*JobQueue)->nextPtr;
            } 
			else{
                prevPtr->nextPtr = currentPtr->nextPtr;
            }
            PtrToTransfer->nextPtr = NULL;
            insertToQueue(NewJobQueue, PtrToTransfer, sortingMetric);
            currentPtr = (prevPtr == NULL) ? *JobQueue : prevPtr->nextPtr;
        }
		else{
            prevPtr = currentPtr; 
            currentPtr = currentPtr->nextPtr;
        }
    }
}

// END OF SCHEDULING FUNCTIONS
// START OF DATA FUNCTIONS

// This function prints the Table of Processes along with every relevant information of the process
void printTable(struct Process *JobQueue) {
	// Print table header
	printf("\n\nTable containing processes information: \n");
    printf("%-12s | %-12s | %-11s | %-9s | %-15s | %-16s | %-15s |\n",
           "Process No.", "Arrival Time", "Burst Time", "Priority",
           "Completion Time", "Turnaround Time", "Waiting Time");

    printf("------------------------------------------------------------------------------------------------------------\n");
	// Traverse through JobQueue to print each process
    struct Process *currentPtr = JobQueue;
    while (currentPtr != NULL) {
        printf("%-12d | %-12d | %-11d | %-9d | %-15d | %-16d | %-15d |\n",
               currentPtr->processID,
               currentPtr->originalArrivalTime,
               currentPtr->burstTime,
               currentPtr->priority,           
               currentPtr->completionTime,
               currentPtr->turnaroundTime,
               currentPtr->waitingTime);
        currentPtr->arrivalTime = currentPtr->originalArrivalTime;
        currentPtr = currentPtr->nextPtr;
    }
}

// This function prints AveragesHeader
void printAveragesHeader(){
	printf("\nTable containing averages: \n");
    printf("%-21s| %-20s| %-20s| %-10s|\n", 
    "Total Completion Time", "Avg Turnaround Time", "Avg Waiting Time", "Throughput");
    printf("------------------------------------------------------------------------------\n");
}

// This function calculates the avgTAT and avgWT and then prints them as well as the total CT and throughPut
void printAverages(struct Process *JobQueue, int totalCompletionTime, double numberOfProcesses){
	struct Process *currentPtr = JobQueue;
	int totalTurnaroundTime = 0, totalWaitingTime = 0; // Initialize to zero for proper summing
	double averageTurnaroundTime, averageWaitingTime, throughput;
	while(currentPtr != NULL){ // Traverse through queue for summing
		totalTurnaroundTime += currentPtr->turnaroundTime;
		//printf("SUM OF TOTALTAT: %d", totalTurnaroundTime);
		totalWaitingTime += currentPtr->waitingTime;
		currentPtr = currentPtr->nextPtr;
	}
	averageTurnaroundTime = totalTurnaroundTime / numberOfProcesses; // Calculate avg, note numberofProcesses has been casted to double to allow decimal division
	averageWaitingTime = totalWaitingTime / numberOfProcesses;
	throughput = numberOfProcesses / totalCompletionTime;
    printf("%-21d| %-20.3f| %-20.3f| %-10.3f|\n", // Print the information
           totalCompletionTime, averageTurnaroundTime, averageWaitingTime, throughput);
}

// END OF DATA FUNCTIONS
// START OF MISCELLANEOUS FUNCTIONS

void printLinkedList(struct Process *head) {
    struct Process *current = head;
	printf("\n\nProceeding to print processes...\n\n");
//    // Print table header
//    printf("%-10s %-15s %-15s %-10s\n", "ProcessID", "ArrivalTime", "BurstTime", "Priority");
//    printf("------------------------------------------------------------\n");

    // Print each process in a table row format
    while (current != NULL) {
        printf("%-10d \n",
               current->processID);
//               current->arrivalTime,
//               current->burstTime,
//               current->priority);
        
        current = current->nextPtr;
    }
    
    printf("\nFinished with printing processes. Press enter to continue...");
    //clearInputBuffer();
}


// END OF MISCELLANEOUS FUNCTIONS
