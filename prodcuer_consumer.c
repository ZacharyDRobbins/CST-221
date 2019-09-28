//Zachary Robbins
//9/27/2019
//The producer creates a value, puts it into the buffer, wakes up the consumer and consumes the number
//then it wakes up the producer to do it all over again.
//This is my own work
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include  <signal.h>
#include <sys/mman.h>
#include <errno.h>

// Constants
int MAX = 100;
int WAKEUP = SIGUSR1;
int SLEEP = SIGUSR2;

// The Child PID if the Parent else the Parent PID if the Child
pid_t otherPid;

// A Signal Set
sigset_t sigSet;

// Shared Circular Buffer
struct CIRCULAR_BUFFER
{
    int count;          // Number of items in the buffer
    int lower;          // Next slot to read in the buffer
    int upper;          // Next slot to write in the buffer
    int buffer[100];
};
struct CIRCULAR_BUFFER *buffer = NULL;

/****************************************************************************************************/

// This method will put the current Process to sleep forever until it is awoken by the WAKEUP signal
void sleepAndWait()
{
    int nSig;

    printf("Sleeping...........\n");
    // TODO: Sleep until notified to wake up using the sigwait() method
    sigwait(&sigSet, &nSig);
    printf("Awoken\n");
}

// This method will signal the Other Process to WAKEUP
void wakeupOther()
{
	// TODO: Signal Other Process to wakeup using the kill() method
   // printf("Inside wakeupother\n");
    kill(otherPid, WAKEUP);
}

// Gets a value from the shared buffer
int getValue()
{
    // TODO: Get a value from the Circular Buffer and adjust where to read from next
    //Get value from the Circular Buffer
    int value = buffer->buffer[buffer->lower];
    //Increase the value of the "lower" reference
    buffer->lower = (buffer->lower + 1) % 100;
    //Reduce the count of items in the buffer
    buffer->count --;
    //return fetched value
    return value;

}

// Puts a value in the shared buffer
void putValue(struct CIRCULAR_BUFFER* buffer, int value)
{
    // TODO: Write to the next available position in the Circular Buffer and adjust where to write next
    //Place the given value inside of the buffer array
    buffer->buffer[buffer->upper] = value;
    //Increase the upper value to change where the next value should be placed
    buffer->upper = (buffer->upper + 1) % 100;
    //Increase the count of how many values are in the buffer
    buffer->count++;
    //print a statement showing what has been placed in the buffer
    printf("value placed in the buffer by the producer is %d\n", value);
}

/****************************************************************************************************/

/**
 * Logic to run to the Consumer Process
 **/
void consumer()
{
    sigemptyset(&sigSet);
    sigaddset(&sigSet, WAKEUP);
    sigaddset(&sigSet, SLEEP);
    sigprocmask(SIG_BLOCK, &sigSet, NULL);

    // Run the Consumer Logic
    printf("Running Consumer Process.....\n");
    
    // TODO: Implement Consumer Logic (see page 129 in book)
    for(int x = 0; x < 20; x++)
    {
        //printf("Inside for, throwing wakeupother\n");
        wakeupOther();
        //printf("Inside for, throwing sleepandwait\n");
        sleepAndWait();
		int i = getValue();
		printf("consume value %i\n", i);
	}
    // Exit cleanly from the Consumer Process
    _exit(1);
}

/**
 * Logic to run to the Producer Process
 **/
void producer()
{
    // Buffer value to write
    int value = 0;

    // Set Signal Set to watch for WAKEUP signal
    sigemptyset(&sigSet);
    sigaddset(&sigSet, WAKEUP);
    sigaddset(&sigSet, SLEEP);
    sigprocmask(SIG_BLOCK, &sigSet, NULL);

    // Run the Producer Logic
    printf("Running Producer Process.....\n");
    
    // TODO: Implement Producer Logic (see page 129 in book)
    for(int x = 0; x < 20; x++)
    {
		value = rand()  % 5;
		putValue(buffer, value);
        wakeupOther();
        sleepAndWait();
	}
    // Exit cleanly from the Consumer Process
    _exit(1);
}

void signalWakeup(int signum){
    printf("wakeup signal: %d\n", signum);
}

/**
 * Main application entry point to demonstrate forking off a child process that will run concurrently with this process.
 *
 * @return 1 if error or 0 if OK returned to code the caller.
 */
int main(int argc, char* argv[])
{
    signal(WAKEUP, signalWakeup);
   

    printf("Starting main\n");
    pid_t  pid;

    // Create shared memory for the Circular Buffer to be shared between the Parent and Child  Processes
    buffer = (struct CIRCULAR_BUFFER*)mmap(0,sizeof(buffer), PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    buffer->count = 0;
    buffer->lower = 0;
    buffer->upper = 0;

   // printf("MAking fork\n");
    // Use fork()
    pid = fork();
    if (pid == -1)
    {
        // Error: If fork() returns -1 then an error happened (for example, number of processes reached the limit).
        printf("Can't fork, error %d\n", errno);
        exit(EXIT_FAILURE);
    }
    // OK: If fork() returns non zero then the parent process is running else child process is running
    if (pid == 0)
    {
        // Run Producer Process logic as a Child Process
       // printf("Inside producer of main\n");
        otherPid = getppid();
        producer();
    }
    else
    {
        // Run Consumer Process logic as a Parent Process
        //printf("Inside Consumer of main\n");
        otherPid = pid;
        consumer();
    }

    // Return OK
    return 0;
}