#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t mutex;                // Semaphore used to limit the number of active threads
int thread_count = 0;       // Used as index of thread struct

struct ThreadStruct{            // Struct that holds  
    int tid;                    // thread id, path of 
    char path[260];             // the file and file name
    char fname[20];
};

int isPrime (int num) {                     // Prime numbers start from 2, and goes
    if(num <= 1) return 0;                  // like 3, 5, 7, etc. This method finds
    for(int i = 2; i < num; i++)            // if the inputted number is prime or not
        if(num % i == 0)                    // by dividing the number by the factors.
            return 0;                       // If there is no remainder after division,
    return 1;                               // it means it is not prime so it returns 0.
}

                
void *readFileAndCountPrime(void *arg) {                                // Each thread executes this function. File path is taken as
    struct ThreadStruct *data = (struct ThreadStruct *)arg;             // argument while creating a thread in the main function. Then, 
    FILE *file = fopen(data->path, "r");                                // it is opened for reading. If there is no file, it prints message.
    if (file == NULL) {
        printf("Couldn't open the file %s\n", data->path);
        pthread_exit(NULL);
    }

    int prime = 0;
    int number;
    while (fscanf(file, "%d", &number) == 1) {          // Using fscanf method, elements of file is read one by one and copied to
        prime +=  isPrime(number);                      // variable "number". isPrime method checks if the number is prime or not
    }                                                   // and increments the number of prime numbers.

    fclose(file);

    // sleep(1);

    printf("Thread %d has found %d primes in %s\n", data->tid, prime, data->fname);     // Each thread counts the number of prime numbers and it is printed.
    
    sem_post(&mutex);       // Semaphore is released
    pthread_exit(NULL);     // Calling thread is ended
}



int main(int argc, char *argv[]) {
    
    char *dirname = argv[1];                // Holds the directory name coming from second argument
    int activeThread = atoi(argv[2]);       // Holds the number of active threads coming from third argument

    sem_init(&mutex, 0, activeThread);      // Semaphore is initialized

    DIR *dir = opendir(dirname);            // Directory is opened and checked if it's null or not
    if (dir == NULL) {
        printf("Couldn't open directory: %s\n", dirname);
        return 1;
    }

    struct dirent *input;       // Used to return information about directory entries
    pthread_t threads[256];
    struct ThreadStruct threadStruct[activeThread];


    int current_tid = 1;        // Thread ids start from 1

    while ((input = readdir(dir)) != NULL) {        // Directory is to be read
        
        if (strcmp(input->d_name, ".") != 0 && strcmp(input->d_name, "..") != 0) {      // Check if entry is a regular file

            sem_wait(&mutex);       //  Semaphore is locked

            strcpy(threadStruct[thread_count].path, dirname);               // Combining directory name
            strcat(threadStruct[thread_count].path, "/");                   // and file name to form a path
            strcat(threadStruct[thread_count].path, input->d_name);         // to send it as an argument

            strcpy(threadStruct[thread_count].fname, input->d_name);        // Holding the file name

            threadStruct[thread_count].tid = current_tid++;                 // Holding the current thread id

            pthread_create(&threads[thread_count], NULL, readFileAndCountPrime, (void *)&threadStruct[thread_count]);       // Creating thread with given arguments and the read method

            thread_count++;
        }
    }

    for(int i = 0; i < thread_count; i++) {         // Used to block until the thread with specified
        pthread_join(threads[i], NULL);             // id is terminated, before continuing execution
    }

    closedir(dir);
    sem_destroy(&mutex);        // Semaphore is destroyed.
    return 0;
}